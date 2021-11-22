/*
 * mems_data_buffer.c
 *
 *  Created on: Nov 21, 2021
 *      Author: steelph0enix
 */

#include "mems_data_buffer.h"

#define MAX_MEASUREMENTS_STORED 2048

static WeatherStationMeasurement measurementsData[MAX_MEASUREMENTS_STORED] = { 0 };
static WeatherStationMeasurement const* const endOfMeasurementDataBuffer = &measurementsData[MAX_MEASUREMENTS_STORED];
static WeatherStationMeasurement* lastMeasurement = &measurementsData[0];
static WeatherStationMeasurement* firstMeasurement = &measurementsData[0];
static size_t currentlyStoredMeasurements = 0;

WeatherStationMeasurement* get_next_element(WeatherStationMeasurement* current_element) {
	WeatherStationMeasurement* nextElement = current_element + (measurements_stored_count() > 0);
	if (nextElement == endOfMeasurementDataBuffer) {
		nextElement = &measurementsData[0];
	}

	return nextElement;
}

WeatherStationMeasurement* get_next_measurement_slot() {
	return get_next_element(lastMeasurement);
}

WeatherStationMeasurement* get_next_measurement() {
	return get_next_element(firstMeasurement);
}

size_t measurements_stored_count() {
	return currentlyStoredMeasurements;
}

size_t measurements_slots_left() {
	return MAX_MEASUREMENTS_STORED - measurements_stored_count();
}

void clear_stored_measurements() {
	currentlyStoredMeasurements = 0;
	lastMeasurement = &measurementsData[0];
	firstMeasurement = &measurementsData[0];
}

bool append_measurement(WeatherStationMeasurement* measurement) {
	if (measurements_slots_left() == 0) {
		return false;
	}

	WeatherStationMeasurement* next_slot = get_next_measurement_slot();
	*next_slot = *measurement;
	lastMeasurement = next_slot;

	currentlyStoredMeasurements++;
	return true;
}

bool fetch_measurement(WeatherStationMeasurement* output_measurement) {
	if (measurements_stored_count() == 0) {
		return false;
	}

	*output_measurement = *firstMeasurement;
	firstMeasurement = get_next_measurement();

	currentlyStoredMeasurements--;
	return true;
}
