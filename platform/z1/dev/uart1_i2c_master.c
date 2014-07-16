/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         I2C communication device drivers for Zolertia Z1 sensor node.
 * \author
 *         Enric M. Calvo, Zolertia <ecalvo@zolertia.com>
 *         Marcus Lundén, SICS <mlunden@sics.se>
 */

#define UART1_DEBUG

#include "uart1_i2c_master.h"
#include "isr_compat.h"
#include "contiki.h"
#include <stdlib.h>
#include "dev/watchdog.h"
#include "lib/ringbuf.h"
#include "isr_compat.h"
#include <stdio.h>

signed   char tx_byte_ctr, rx_byte_ctr;
unsigned char rx_buf[2];
unsigned char* tx_buf_ptr;
unsigned char* rx_buf_ptr;
unsigned char receive_data;
unsigned char transmit_data1;
unsigned char transmit_data2;
volatile unsigned int i;	// volatile to prevent optimization



static int (*uart1_input_handler)(unsigned char c);

static volatile uint8_t serial_transmitting;

#ifdef UART1_CONF_TX_WITH_INTERRUPT
#define TX_WITH_INTERRUPT UART1_CONF_TX_WITH_INTERRUPT
#else /* UART1_CONF_TX_WITH_INTERRUPT */
#define TX_WITH_INTERRUPT 1
#endif /* UART1_CONF_TX_WITH_INTERRUPT */

#if TX_WITH_INTERRUPT
#define TXBUFSIZE 64

static struct ringbuf txbuf;
static uint8_t txbuf_data[TXBUFSIZE];
#endif /* TX_WITH_INTERRUPT */
//------------------------------------------------------------------------------
// void i2c_receiveinit(unsigned char slave_address, 
//                              unsigned char prescale)
//
// This function initializes the USCI module for master-receive operation. 
//
// IN:   unsigned char slave_address   =>  Slave Address
//       unsigned char prescale        =>  SCL clock adjustment 
//-----------------------------------------------------------------------------
void
i2c_receiveinit(uint8_t slave_address) {
  UCB1CTL1 = UCSWRST;                    // Enable SW reset
  UCB1CTL0 = UCMST + UCMODE_3 + UCSYNC;  // I2C Master, synchronous mode
  UCB1CTL1 = UCSSEL_2 | UCSWRST;         // Use SMCLK, keep SW reset
  UCB1BR0  = I2C_PRESC_400KHZ_LSB;       // prescaler for 400 kHz data rate
  UCB1BR1  = I2C_PRESC_400KHZ_MSB;
  UCB1I2CSA = slave_address;	         // set slave address

  UCB1CTL1 &= ~UCTR;		         // I2C Receiver 

  UCB1CTL1 &= ~UCSWRST;	                 // Clear SW reset, resume operation
  UCB1I2CIE = UCNACKIE;
#if I2C_RX_WITH_INTERRUPT
  UC1IE = UCB1RXIE;                      // Enable RX interrupt if desired
#endif
}

//------------------------------------------------------------------------------
// void i2c_transmitinit(unsigned char slave_address, 
//                               unsigned char prescale)
//
// Initializes USCI for master-transmit operation. 
//
// IN:   unsigned char slave_address   =>  Slave Address
//       unsigned char prescale        =>  SCL clock adjustment 
//------------------------------------------------------------------------------
void
i2c_transmitinit(uint8_t slave_address) {
  UCB1CTL1 |= UCSWRST;		           // Enable SW reset
  UCB1CTL0 |= (UCMST | UCMODE_3 | UCSYNC); // I2C Master, synchronous mode
  UCB1CTL1  = UCSSEL_2 + UCSWRST;          // Use SMCLK, keep SW reset
  UCB1BR0   = I2C_PRESC_400KHZ_LSB;        // prescaler for 400 kHz data rate
  UCB1BR1   = I2C_PRESC_400KHZ_MSB;
  UCB1I2CSA = slave_address;	           // Set slave address

  UCB1CTL1 &= ~UCSWRST;                    // Clear SW reset, resume operation
  UCB1I2CIE = UCNACKIE;
  UC1IE = UCB1TXIE;		           // Enable TX ready interrupt
}

//------------------------------------------------------------------------------
// void i2c_receive_n(unsigned char byte_ctr, unsigned char * rx_buf)
// This function is used to start an I2C communication in master-receiver mode WITHOUT INTERRUPTS
// for more than 1 byte
// IN:   unsigned char byte_ctr   =>  number of bytes to be read
// OUT:  unsigned char rx_buf     =>  receive data buffer
// OUT:  int n_received           =>  number of bytes read
//------------------------------------------------------------------------------
static volatile uint8_t rx_byte_tot = 0;
uint8_t
i2c_receive_n(uint8_t byte_ctr, uint8_t *rx_buf) {

  rx_byte_tot = byte_ctr;
  rx_byte_ctr = byte_ctr;
  rx_buf_ptr  = rx_buf;

  while ((UCB1CTL1 & UCTXSTT) || (UCB1STAT & UCNACKIFG))	// Slave acks address or not?
    PRINTFDEBUG ("____ UCTXSTT not clear OR NACK received\n");

#if I2C_RX_WITH_INTERRUPT
  PRINTFDEBUG(" RX Interrupts: YES \n");

  // SPECIAL-CASE: Stop condition must be sent while receiving the 1st byte for 1-byte only read operations
  if(rx_byte_tot == 1){                 // See page 537 of slau144e.pdf
    dint();
    UCB1CTL1 |= UCTXSTT;		// I2C start condition
    while(UCB1CTL1 & UCTXSTT)           // Waiting for Start bit to clear
      PRINTFDEBUG ("____ STT clear wait\n");
    UCB1CTL1 |= UCTXSTP;		// I2C stop condition
    eint();
  }
  else{                                 // all other cases
    UCB1CTL1 |= UCTXSTT;		// I2C start condition
  }
  return 0;

#else
  uint8_t n_received = 0;

  PRINTFDEBUG(" RX Interrupts: NO \n");

  UCB1CTL1 |= UCTXSTT;		// I2C start condition

  while (rx_byte_ctr > 0){
    if (UC1IFG & UCB1RXIFG) {   // Waiting for Data
      rx_buf[rx_byte_tot - rx_byte_ctr] = UCB1RXBUF;
      rx_byte_ctr--;
      UC1IFG &= ~UCB1RXIFG;     // Clear USCI_B1 RX int flag      
      n_received++;
    }
  }
  UCB1CTL1 |= UCTXSTP;		// I2C stop condition
  return n_received;
#endif
}


//------------------------------------------------------------------------------
// uint8_t i2c_busy()
//
// This function is used to check if there is communication in progress. 
//
// OUT:  unsigned char  =>  0: I2C bus is idle, 
//                          1: communication is in progress
//------------------------------------------------------------------------------
uint8_t
i2c_busy(void) {
  return (UCB1STAT & UCBBUSY);
}

/*----------------------------------------------------------------------------*/
/* Setup ports and pins for I2C use. */

void
i2c_enable(void) {
  I2C_PxSEL |= (I2C_SDA | I2C_SCL);    // Secondary function (USCI) selected
  I2C_PxSEL2 |= (I2C_SDA | I2C_SCL);   // Secondary function (USCI) selected
  I2C_PxDIR |= I2C_SCL;	               // SCL is output (not needed?)
  I2C_PxDIR &= ~I2C_SDA;	       // SDA is input (not needed?)
  I2C_PxREN |= (I2C_SDA | I2C_SCL);    // Activate internal pull-up/-down resistors
  I2C_PxOUT |= (I2C_SDA | I2C_SCL);    // Select pull-up resistors
}

void
i2c_disable(void) {
  I2C_PxSEL &= ~(I2C_SDA | I2C_SCL);    // GPIO function selected
  I2C_PxSEL2 &= ~(I2C_SDA | I2C_SCL);   // GPIO function selected
  I2C_PxREN &= ~(I2C_SDA | I2C_SCL);    // Deactivate internal pull-up/-down resistors
  I2C_PxOUT &= ~(I2C_SDA | I2C_SCL);    // Select pull-up resistors
}

/*----------------------------------------------------------------------------*/
//------------------------------------------------------------------------------
// void i2c_transmit_n(unsigned char byte_ctr, unsigned char *field)
//
// This function is used to start an I2C communication in master-transmit mode. 
//
// IN:   unsigned char byte_ctr   =>  number of bytes to be transmitted
//       unsigned char *tx_buf    =>  Content to transmit. Read and transmitted from [0] to [byte_ctr]
//------------------------------------------------------------------------------
static volatile uint8_t tx_byte_tot = 0;
void
i2c_transmit_n(uint8_t byte_ctr, uint8_t *tx_buf) {
  tx_byte_tot = byte_ctr;
  tx_byte_ctr = byte_ctr;
  tx_buf_ptr  = tx_buf;
  UCB1CTL1 |= UCTR + UCTXSTT;	   // I2C TX, start condition
}


/*----------------------------------------------------------------------------*/
uint8_t
uart1_active(void)
{
  return (UCA1STAT & UCBUSY) | serial_transmitting;
}
/*---------------------------------------------------------------------------*/
void
uart1_set_input(int (*input)(unsigned char c))
{
  uart1_input_handler = input;
}
/*---------------------------------------------------------------------------*/

void
uart1_writeb(unsigned char c)
{
  printf("UART1 writeb **\n");
  /* watchdog_periodic(); */
//#if TX_WITH_INTERRUPT
//  printf("Uart1 write with interrupt\n");
  /* Put the outgoing byte on the transmission buffer. If the buffer
     is full, we just keep on trying to put the byte into the buffer
     until it is possible to put it there. */
 // while(ringbuf_put(&txbuf, c) == 0);

  /* If there is no transmission going, we need to start it by putting
     the first byte into the UART. */
//  if(serial_transmitting == 0) {
//    serial_transmitting = 1;
//    UCA1TXBUF = ringbuf_get(&txbuf);
//  }

//#else /* TX_WITH_INTERRUPT */
  printf("UART1 tx without interrupt\n");
  /* Loop until the transmission buffer is available. */
  while(!(IFG2 & UCA1TXIFG));

  /* Transmit the data. */
  UCA1TXBUF = 'c';
//  UCA1TXBUF = 0x23;
  printf("char written to UCA1TXBUF\n");
//#endif /* TX_WITH_INTERRUPT */
}



/*----------------------------------------------------------------------------*/
/**
 * Initalize the RS232 port.
 *
 */
void
uart1_init(unsigned long ubr)
{
  UCA1CTL1 |= UCSWRST;            /* Hold peripheral in reset state */
  P3SEL |= 0xC0;                            /* P3.6,7 = USCI_A1 TXD/RXD */
/*  P3SEL2 &= ~0xC0; This register doesn't seem to be defined anywhere */
  P3DIR &= ~0x80;                 /*3.7 as input*/
  P3DIR |= 0x40;                    /*3.6 as output*/

  UCA1CTL0 = 0x00;
  UCA1CTL1 |= UCSSEL_3;                     /* CLK = SMCLK */
  UCA1BR0 = BAUD2UBR(38400); /*Hard coded as passing an arg in didn't work */
  UCA1BR1 = 0x00;
  UCA1MCTL = UCBRS_2;                        /* Modulation UCBRSx = 4 */

  UCA1CTL1 &= ~UCSWRST;                     /* Initialize USCI state machine */

  serial_transmitting = 0;

  /* XXX Clear pending interrupts before enable */
  IFG2 &= ~UCA1RXIFG;
  IFG2 &= ~UCA1TXIFG;
  UCA1CTL1 &= ~UCSWRST;                   /* Initialize USCI state machine  **before** enabling interrupts */
  UC1IE |= UCA1RXIE;
}


/*----------------------------------------------------------------------------*/
ISR(USCIAB1TX, uart1_i2c_tx_interrupt)
{
//  printf("ISR TX\n");
  // TX Part
  if (UC1IFG & UCB1TXIFG) {        // TX int. condition
    if (tx_byte_ctr == 0) {
      UCB1CTL1 |= UCTXSTP;	   // I2C stop condition
      UC1IFG &= ~UCB1TXIFG;	   // Clear USCI_B1 TX int flag
    }
    else {
      UCB1TXBUF = tx_buf_ptr[tx_byte_tot - tx_byte_ctr];
      tx_byte_ctr--;
    }
  }
  // RX Part
#if I2C_RX_WITH_INTERRUPT
  else if (UC1IFG & UCB1RXIFG){    // RX int. condition
    rx_buf_ptr[rx_byte_tot - rx_byte_ctr] = UCB1RXBUF;
    rx_byte_ctr--;
    if (rx_byte_ctr == 1){ //stop condition should be set before receiving last byte
      // Only for 1-byte transmissions, STOP is handled in receive_n_int
      if (rx_byte_tot != 1) 
        UCB1CTL1 |= UCTXSTP;       // I2C stop condition
        UC1IFG &= ~UCB1RXIFG;        // Clear USCI_B1 RX int flag. XXX Just in case, check if necessary
    }
  }
#endif
#if TX_WITH_INTERRUPT
  else if(IFG2 & UCA1TXIFG) {
    if(ringbuf_elements(&txbuf) == 0) {
      serial_transmitting = 0;
    } else {
      UCA0TXBUF = ringbuf_get(&txbuf);
    }
  }
#endif /* TX_WITH_INTERRUPT */
}

ISR(USCIAB1RX, uart1_i2c_rx_interrupt)
{
  uint8_t c;
//  printf("ISR\n"); 
#if I2C_RX_WITH_INTERRUPT
  if(UCB1STAT & UCNACKIFG) {
//    PRINTFDEBUG("!!! NACK received in RX\n");
//    printf("i2c int");
    UCB1CTL1 |= UCTXSTP;
    UCB1STAT &= ~UCNACKIFG;
  }
#endif
  if( UC1IFG & UCA1RXIFG){
//    printf("Char recieved\n");
    if(UCA1STAT & UCRXERR) {
//      printf("Serial1 RX error");
    /* Check status register for receive errors. */
      c = UCA1RXBUF;   /* Clear error flags by forcing a dummy read. */
    } else {
      c = UCA1RXBUF;
#ifdef UART1_DEBUG
      printf("%i\n", c);
#endif
      if(uart1_input_handler != NULL) {
        if(uart1_input_handler(c)) {
          LPM4_EXIT;
        }
      }
    }
  }
}
