/*
 * ble_app_events.h
 *
 *  Created on: Nov 4, 2021
 *      Author: steelph0enix
 */

#ifndef APP_BLE_APP_INTERFACE_H_
#define APP_BLE_APP_INTERFACE_H_

#include <stdint.h>
#include "rtc_utils.h"

typedef enum BLEControlCharValue_t {
	BLE_CTRL_DEFAULT = 0x00,
	BLE_CTRL_GET_DATA = 0x01,
	BLE_CTRL_NEXT_RECORD_AVAILABLE = 0x02,
	BLE_CTRL_FETCH_NEXT_RECORD = 0x03,
	BLE_CTRL_ABORT_FETCHING = 0x04,
	BLE_CTRL_SET_MEASUREMENT_INTERVAL = 0x05,
	BLE_CTRL_SET_DATE_AND_TIME = 0x06
} BLEControlCharValue;

RTC_TimeTypeDef get_ble_time();
RTC_DateTypeDef get_ble_date();
BLEControlCharValue get_ble_control_value();
uint16_t get_ble_number_of_records();

void set_ble_control_value(BLEControlCharValue value);
void set_ble_number_of_records(uint16_t number);
void set_ble_time(uint8_t hour, uint8_t minute, uint8_t second);
void set_ble_date(uint8_t year, uint8_t month, uint8_t day, uint8_t weekday);
void set_ble_temperature(int32_t temperature);
void set_ble_pressure(int32_t pressure);
void set_ble_humidity(int32_t humidity);

void ble_control_value_changed(BLEControlCharValue value);

#endif /* APP_BLE_APP_INTERFACE_H_ */
