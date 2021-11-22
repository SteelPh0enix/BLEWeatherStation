/*
 * app_states.c
 *
 *  Created on: Nov 20, 2021
 *      Author: steelph0enix
 */

#include "app_states.h"
#include "ble_app_interface.h"
#include "print_utils.h"
#include "rtc_utils.h"
#include "mems_data_buffer.h"
#include "mems_sensors.h"

#include <stdbool.h>

static void set_app_state(AppState new_state);
static void app_set_date_and_time();
static void app_start_fetching();
static void app_fetch_next();
static void app_finish_fetching();
static void update_alarm_time(RTC_TimeTypeDef *interval);

static uint8_t const alarmSecondsGracePeriod = 3;

static AppState currentAppState = APP_STATE_IDLE;
static bool isFetchingData = false;
static RTC_TimeTypeDef alarmInterval = { 0 };

void ble_control_value_changed(BLEControlCharValue value) {
	switch (value) {
	case BLE_CTRL_GET_DATA:
		app_start_fetching();
		break;
	case BLE_CTRL_FETCH_NEXT_RECORD:
		app_fetch_next();
		break;
	case BLE_CTRL_ABORT_FETCHING:
		app_finish_fetching();
		break;
	case BLE_CTRL_SET_MEASUREMENT_INTERVAL: {
		RTC_TimeTypeDef const newInterval = get_ble_time();
		app_set_measurement_interval(newInterval.Hours, newInterval.Minutes,
				newInterval.Seconds);
		break;
	}
	case BLE_CTRL_SET_DATE_AND_TIME:
		app_set_date_and_time();
		break;
	default:
		debugPrint(
				"Control byte changed to unexpected value 0x%02X, not handling that.",
				(uint8_t )value);
		break;
	}
}

static void update_alarm_time(RTC_TimeTypeDef *interval) {
	setRTCAlarmSinceNow(interval->Hours, interval->Minutes, interval->Seconds);
}

static void print_measurement(WeatherStationMeasurement const *measurement) {
	// @formatter:off
	printf("\tMeasurement @ %02d:%02d:%02d, %02d-%02d-20%02d -> temperature: %.2f*C, pressure: %.2fhPa, humidity: %.2f%% \n",
			measurement->hour, measurement->minute, measurement->second,
			measurement->day, measurement->month, measurement->day,
			((float)(measurement->temperature)) / 100.f,
			((float)(measurement->pressure)) / 100.f,
			((float)(measurement->humidity)) / 100.f);
						// @formatter:on
}

static void make_new_measurement() {
	set_app_state(APP_STATE_MEASURING);

	WeatherStationMeasurement measurement = { 0 };
	RTC_TimeTypeDef currentTime = { 0 };
	RTC_DateTypeDef currentDate = { 0 };

	HAL_RTC_GetTime(&hrtc, &currentTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &currentDate, RTC_FORMAT_BIN);

	measurement.temperature = (int32_t) (mems_get_temperature() * 100.f);
	measurement.pressure = (int32_t) (mems_get_pressure() * 100.f);
	measurement.humidity = (int32_t) (mems_get_humidity() * 100.f);

	measurement.hour = currentTime.Hours;
	measurement.minute = currentTime.Minutes;
	measurement.second = currentTime.Seconds;

	measurement.year = currentDate.Year;
	measurement.month = currentDate.Month;
	measurement.day = currentDate.Date;

	print_measurement(&measurement);

	if (append_measurement(&measurement)) {
		debugPrint(
				"Measurement added to buffer, %u measurements in buffer, %u slots left",
				measurements_stored_count(), measurements_slots_left());
	} else {
		debugPrint(
				"Measurement NOT ADDED to buffer, %u measurements in buffer, %u slots left",
				measurements_stored_count(), measurements_slots_left());
	}

	set_ble_number_of_records(measurements_stored_count());

	set_app_state(APP_STATE_IDLE);
}

void app_rtc_alarm_handler() {
	update_alarm_time(&alarmInterval);
	make_new_measurement();
}

void app_set_measurement_interval(uint8_t hours, uint8_t minutes,
		uint8_t seconds) {
	set_app_state(APP_STATE_SETTING_INTERVAL);

	debugPrint("Updating measurement interval...");

	RTC_TimeTypeDef newAlarmInterval = { 0 };

	newAlarmInterval.Hours = hours;
	newAlarmInterval.Minutes = minutes;
	newAlarmInterval.Seconds = seconds;

	if (validateTime(&newAlarmInterval)) {
		alarmInterval = newAlarmInterval;
		setRTCAlarmSinceNow(0, 0, alarmSecondsGracePeriod);
		debugPrint(
				"New alarm interval set, alarm will trigger every %02d:%02d:%02d",
				hours, minutes, seconds);
	} else {
		debugPrint(
				"New interval validation failed, not setting this up. Tried to set alarm every %02d:%02d:%02d",
				hours, minutes, seconds);
	}

	set_app_state(APP_STATE_IDLE);
}

static void app_set_date_and_time() {
	set_app_state(APP_STATE_SETTING_DATE_AND_TIME);

	debugPrint("Updating date and time...");

	RTC_TimeTypeDef const time = get_ble_time();
	RTC_DateTypeDef const date = get_ble_date();

	setRTCDateS(date);
	setRTCTimeS(time);

	setRTCAlarmSinceNow(0, 0, alarmSecondsGracePeriod);

	set_app_state(APP_STATE_IDLE);
}

static void app_start_fetching() {
	isFetchingData = true;
	app_fetch_next();
}

static void app_fetch_next() {
	set_app_state(APP_STATE_FETCHING_RECORDS);

	WeatherStationMeasurement measurement = { 0 };
	if (!fetch_measurement(&measurement)) {
		app_finish_fetching();
		return;
	}

	uint16_t const number_of_records = measurements_stored_count();

	set_ble_time(measurement.hour, measurement.minute, measurement.second);
	set_ble_date(measurement.year, measurement.month, measurement.day, 0);
	set_ble_temperature(measurement.temperature);
	set_ble_pressure(measurement.pressure);
	set_ble_humidity(measurement.humidity);
	set_ble_number_of_records(number_of_records);

	set_app_state(APP_STATE_RECORD_READY);
}

static void app_finish_fetching() {
	isFetchingData = false;
	set_app_state(APP_STATE_IDLE);
}

static void set_app_state(AppState new_state) {
	debugPrint("App state change from 0x%02X to 0x%02X", currentAppState,
			new_state);

// Value set by weather station:
//	* DEFAULT
//	* NEXT_RECORD_AVAILABLE

	switch (new_state) {
	case APP_STATE_IDLE:
		if (isFetchingData) {
			// do not change the value of control char when it's set to NEXT_RECORD_AVAILABLE
			// to prevent sync issues on the device side
			return;
//			set_app_state(APP_STATE_RECORD_READY);
		} else {
			set_ble_control_value(BLE_CTRL_DEFAULT);
		}
		break;
	case APP_STATE_RECORD_READY:
		set_ble_control_value(BLE_CTRL_NEXT_RECORD_AVAILABLE);
	default:
		break;
	}

	currentAppState = new_state;
}

AppState get_app_state() {
	return currentAppState;
}
