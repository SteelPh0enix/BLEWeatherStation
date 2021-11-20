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

static AppState currentAppState = APP_STATE_IDLE;

void ble_control_value_changed(BLEControlCharValue value) {
	switch (value) {
		case BLE_CTRL_GET_DATA:
			break;
		case BLE_CTRL_FETCH_NEXT_RECORD:
			break;
		case BLE_CTRL_ABORT_FETCHING:
			break;
		case BLE_CTRL_SET_MEASUREMENT_INTERVAL:
			break;
		case BLE_CTRL_SET_DATE_AND_TIME:
			app_set_date_and_time();
			break;
		default:
			debugPrint("Control byte changed to unexpected value 0x%02X, not handling that.", (uint8_t )value);
			break;
	}
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
