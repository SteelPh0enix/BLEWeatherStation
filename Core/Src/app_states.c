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

static void set_app_state(AppState new_state);
static void app_set_date_and_time();
static void update_alarm_time(RTC_TimeTypeDef* interval);

static const uint8_t alarmSecondsGracePeriod = 3;

static AppState currentAppState = APP_STATE_IDLE;
static RTC_TimeTypeDef alarmInterval = { 0 };

void ble_control_value_changed(BLEControlCharValue value) {
	switch (value) {
		case BLE_CTRL_GET_DATA:
			break;
		case BLE_CTRL_FETCH_NEXT_RECORD:
			break;
		case BLE_CTRL_ABORT_FETCHING:
			break;
		case BLE_CTRL_SET_MEASUREMENT_INTERVAL: {
			RTC_TimeTypeDef const newInterval = get_ble_time();
			app_set_measurement_interval(newInterval.Hours, newInterval.Minutes, newInterval.Seconds);
			break;
		}
		case BLE_CTRL_SET_DATE_AND_TIME:
			app_set_date_and_time();
			break;
		default:
			debugPrint("Control byte changed to unexpected value 0x%02X, not handling that.", (uint8_t )value);
			break;
	}
}

static void update_alarm_time(RTC_TimeTypeDef* interval) {
	setRTCAlarmSinceNow(interval->Hours, interval->Minutes, interval->Seconds);
}

void app_rtc_alarm_handler() {
	update_alarm_time(&alarmInterval);
}

void app_set_measurement_interval(uint8_t hours, uint8_t minutes, uint8_t seconds) {
	set_app_state(APP_STATE_SETTING_INTERVAL);

	debugPrint("Updating measurement interval...");

	RTC_TimeTypeDef newAlarmInterval = { 0 };

	newAlarmInterval.Hours = hours;
	newAlarmInterval.Minutes = minutes;
	newAlarmInterval.Seconds = seconds;

	if (validateTime(&newAlarmInterval)) {
		alarmInterval = newAlarmInterval;
		setRTCAlarmSinceNow(0, 0, alarmSecondsGracePeriod);
		debugPrint("New alarm interval set, alarm will trigger every %02d:%02d:%02d", hours, minutes, seconds);
	} else {
		debugPrint("New interval validation failed, not setting this up. Tried to set alarm every %02d:%02d:%02d",
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

	set_app_state(APP_STATE_IDLE);
}

static void set_app_state(AppState new_state) {
	debugPrint("App state change from 0x%02X to 0x%02X", currentAppState, new_state);
	currentAppState = new_state;

// Value set by weather station:
//	* DEFAULT
//	* NEXT_RECORD_AVAILABLE

	switch (new_state) {
		case APP_STATE_IDLE:
			set_ble_control_value(BLE_CTRL_DEFAULT);
			break;
		case APP_STATE_RECORD_READY:
			set_ble_control_value(BLE_CTRL_NEXT_RECORD_AVAILABLE);
		default:
			break;
	}

}

AppState get_app_state() {
	return currentAppState;
}
