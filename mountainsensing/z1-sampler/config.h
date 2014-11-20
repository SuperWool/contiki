#ifndef SAMPLER_CONFIG_H
#define SAMPLER_CONFIG_H

	#define SAMPLE_CONFIG 1
	#define COMMS_CONFIG 2

	#include <stdlib.h>
	// Config
	#include "settings.pb.h"
	#include "readings.pb.h"

	// Protobuf
	#include "dev/pb_decode.h"
	#include "dev/pb_encode.h"

	#include "cfs/cfs.h"
	#include "contiki.h"

	#include <stdlib.h>
	#include <stdio.h>

	uint8_t set_config(void *pb, uint8_t config);
	uint8_t get_config(void *pb, uint8_t config);
	void print_sensor_config(SensorConfig *conf);

#endif