/*
 * mems_data_buffer.h
 *
 *  Created on: Nov 21, 2021
 *      Author: steelph0enix
 */

#ifndef INC_MEMS_DATA_BUFFER_H_
#define INC_MEMS_DATA_BUFFER_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct WeatherStationMeasurement_t {
	// measured data, multiplied by 100 (int truncation)
	int32_t temperature;
	int32_t pressure;
	int32_t humidity;

	// time of measurement
	uint8_t hour;
	uint8_t minute;
	uint8_t second;

	// date of measurement
	uint8_t year;
	uint8_t month;
	uint8_t day;
} WeatherStationMeasurement;

size_t measurements_stored_count();
size_t measurements_slots_left();
void clear_stored_measurements();
bool append_measurement(WeatherStationMeasurement* measurement);
bool fetch_measurement(WeatherStationMeasurement* output_measurement);

#endif /* INC_MEMS_DATA_BUFFER_H_ */
