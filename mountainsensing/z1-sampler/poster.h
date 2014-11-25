#ifndef POSTER_H
#define POSTER_H
	#include "filenames-old.h"
	#include "config.h"
	#include "contiki.h"
	#include "contiki-conf.h"
	#include <stdio.h>
	#include <stdint.h>
	#include "z1-sampler-config-defaults.h"
	#include "settings.pb.h"
	#include "cfs/cfs.h"
	// Networking
	#include "contiki-net.h"
	#include "sampling-sensors.h"
	#include "uip-ds6.h"
	#include "uip-debug.h"
	#include "dev/cc1120.h"
	#include "dev/cc1120-arch.h"
	#include "platform-conf.h"

	#define CONNECTION_RETRIES 3
	#define LIVE_CONNECTION_TIMEOUT 300
	#define DATA_BUFFER_LENGTH 256
	#define PSOCK_BUFFER_LENGTH 120

	void refreshPosterConfig(void);
	
	PROCESS_NAME(post_process);

	int handle_connection(char *data_buffer, uint8_t data_length, uint8_t *http_status, struct psock *p);
	uint8_t load_file(char *data_buffer, char *filename);

#endif
