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
 * specific settings for University of Southampton feshie project 2016
 */

#ifndef __PROJECT_FESHIE_Z1_COAP_H__
#define __PROJECT_FESHIE_Z1_COAP_H__

/* Disabling TCP on CoAP nodes. */
#undef UIP_CONF_TCP
#define UIP_CONF_TCP                   0

/* Increase rpl-border-router IP-buffer when using more than 64. */
#undef REST_MAX_CHUNK_SIZE
#define REST_MAX_CHUNK_SIZE            48

/* Multiplies with chunk size, be aware of memory constraints. */
#undef COAP_MAX_OPEN_TRANSACTIONS
#define COAP_MAX_OPEN_TRANSACTIONS     1

/* Must be <= open transactions, default is COAP_MAX_OPEN_TRANSACTIONS-1. */
#undef COAP_MAX_OBSERVERS
#define COAP_MAX_OBSERVERS             0

#undef COAP_CONF_MAX_OBSERVEES
#define COAP_CONF_MAX_OBSERVEES        0

/* Filtering .well-known/core per query can be disabled to save space. */
#undef COAP_LINK_FORMAT_FILTERING
#define COAP_LINK_FORMAT_FILTERING     0
#undef COAP_PROXY_OPTION_PROCESSING
#define COAP_PROXY_OPTION_PROCESSING   0

/* Enable client-side support for COAP observe */
#define COAP_OBSERVE_CLIENT            0

/* Disable .well-known/core ressource. 
#define COAP_RES_WITHOUT_WELL_KNOWN
*/

#endif
