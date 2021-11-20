/*
 * ble_app_services.h
 *
 *  Created on: Nov 3, 2021
 *      Author: steelph0enix
 */

#ifndef APP_BLE_APP_SERVICES_H_
#define APP_BLE_APP_SERVICES_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum BLECharacteristic_t {
	BLE_CHAR_TIME = 0,
	BLE_CHAR_DATE,
	BLE_CHAR_TEMPERATURE,
	BLE_CHAR_PRESSURE,
	BLE_CHAR_HUMIDITY,
	BLE_CHAR_CONTROL,
	BLE_CHAR_NUMBER_OF_RECORDS,
	BLE_CHAR_INVALID
} BLECharacteristic;

void add_app_services();

void characteristic_value_changed(BLECharacteristic characteristic, uint8_t data[], uint16_t length);
uint8_t set_characteristic_value(BLECharacteristic characteristic, uint8_t data[], uint16_t length);

#endif /* APP_BLE_APP_SERVICES_H_ */
