/**
 * Standard interface between z1-coap and the varios supported platforms.
 * Supports getting / setting the time, and provides functions for getting readings
 * from various sensors.
 */

#ifndef SAMPLING_SENSORS_H
#define SAMPLING_SENSORS_H

#include <stdbool.h>
#include "settings.pb.h"
#include "readings.pb.h"

/**
 * The parent process. Usefull for doing a context switch in order to use timers / wait for events.
 */
void sampler_init(void);

// Required sensors.
float sampler_get_temp(void);
float sampler_get_batt(void);
int16_t sampler_get_acc_x(void);
int16_t sampler_get_acc_y(void);
int16_t sampler_get_acc_z(void);

/**
 * Get the current time.
 * @return  The number of seconds since the Unix epoch, or 0 in case of error.
 * @note	Unix epoch is taken as 1970-01-01 00:00:00 UTC.
 */
uint32_t sampler_get_time(void);

/**
 * Set the current time.
 * @param   seconds The number of seconds since the Unix epoch
 * @return  True on success, false otherwise
 * @note	Unix epoch is taken as 1970-01-01 00:00:00 UTC.
 */
bool sampler_set_time(uint32_t seconds);

/**
 * Get any extra platform specific readings.
 * These should be directly inserted into the sample struct.
 * @return True on success, false on failure.
 */
bool sampler_get_extra(Sample *sample, SensorConfig *config);

#endif // ifndef SAMPLING_SENSORS_H