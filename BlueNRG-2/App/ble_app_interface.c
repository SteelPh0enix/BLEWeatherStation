/*
 * ble_app_events.c
 *
 *  Created on: Nov 4, 2021
 *      Author: steelph0enix
 */

#include <ble_app_interface.h>
#include <ble_app_services.h>
#include "print_utils.h"
#include "bit_helpers.h"

static RTC_TimeTypeDef bleTime = { 0 };
static RTC_DateTypeDef bleDate = { 0 };
static BLEControlCharValue bleControlValue = BLE_CTRL_DEFAULT;

void ble_time_changed(uint8_t data[]) {
	// 3 bytes - hour, minute, second
	uint8_t const hour = data[0];
	uint8_t const minute = data[1];
	uint8_t const second = data[2];

	bleTime.Hours = hour;
	bleTime.Minutes = minute;
	bleTime.Seconds = second;

//	setRTCTime(hour, minute, second);
}

void ble_date_changed(uint8_t data[]) {
	// 4 bytes - year (last two digits), month, day, weekday
	uint8_t year = data[0];
	uint8_t month = data[1];
	uint8_t day = data[2];
	uint8_t weekday = data[3];

	bleDate.Year = year;
	bleDate.Month = month;
	bleDate.Date = day;
	bleDate.WeekDay = weekday;

//	setRTCDay(year, month, day, weekday);
}

__weak void ble_control_value_changed(BLEControlCharValue value) {
	UNUSED(value);
}

void ble_control_byte_changed(BLEControlCharValue value) {
	debugPrint("BLE control byte changed to 0x%02x", value);
	bleControlValue = value;
	ble_control_value_changed(value);
}

RTC_TimeTypeDef get_ble_time() {
	return bleTime;
}

RTC_DateTypeDef get_ble_date() {
	return bleDate;
}

BLEControlCharValue get_ble_control_value() {
	return bleControlValue;
}

void set_ble_control_value(BLEControlCharValue value) {
	debugPrint("Control byte value set to 0x%02X", value);
	set_characteristic_value(BLE_CHAR_CONTROL, &value, 1);
}

void set_ble_number_of_records(uint16_t number) {
	debugPrint("Number of records set to %d", number);
	uint8_t val[2] = { 0 };
	VALUE_TO_16BIT_BYTEARRAY_BE(number, val);
	set_characteristic_value(BLE_CHAR_NUMBER_OF_RECORDS, val, 2);
}

void set_ble_time(uint8_t hour, uint8_t minute, uint8_t second) {
	debugPrint("Time set to %02d:%02d:%02d", hour, minute, second);
	uint8_t vals[3] = {hour, minute, second};
	set_characteristic_value(BLE_CHAR_TIME, vals, 3);
}

void set_ble_date(uint8_t year, uint8_t month, uint8_t day, uint8_t weekday) {
	debugPrint("Date set to %02d-%02d-20%02d, weekday #%d", day, month, year, weekday);
	uint8_t vals[4] = {year, month, day, weekday};
	set_characteristic_value(BLE_CHAR_DATE, vals, 4);
}

void set_ble_temperature(int32_t temperature) {
	debugPrint("Temperature set to %ld", temperature);
	uint8_t val[4] = {0};
	VALUE_TO_32BIT_BYTEARRAY_BE(temperature, val);
	set_characteristic_value(BLE_CHAR_TEMPERATURE, val, 4);
}

void set_ble_pressure(int32_t pressure) {
	debugPrint("Pressure set to %ld", pressure);
	uint8_t val[4] = {0};
	VALUE_TO_32BIT_BYTEARRAY_BE(pressure, val);
	set_characteristic_value(BLE_CHAR_PRESSURE, val, 4);
}

void set_ble_humidity(int32_t humidity) {
	debugPrint("Humidity set to %ld", humidity);
	uint8_t val[4] = {0};
	VALUE_TO_32BIT_BYTEARRAY_BE(humidity, val);
	set_characteristic_value(BLE_CHAR_HUMIDITY, val, 4);
}

void characteristic_value_changed(BLECharacteristic characteristic,
		uint8_t data[], uint16_t length) {
	switch (characteristic) {
	case BLE_CHAR_TIME:
		ble_time_changed(data);
		break;
	case BLE_CHAR_DATE:
		ble_date_changed(data);
		break;
	case BLE_CHAR_CONTROL:
		ble_control_byte_changed(data[0]);
		break;
	default:
		debugPrint("Unexpected characteristic change, char id: %d, length: %d",
				(uint8_t )characteristic, length);
		break;
	}
}

