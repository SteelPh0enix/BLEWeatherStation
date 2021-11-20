/*
 * ble_app_services.c
 *
 *  Created on: Nov 3, 2021
 *      Author: steelph0enix
 */

#include <ble_app_interface.h>
#include <ble_app_services.h>
#include "bluenrg1_aci.h"
#include "bluenrg1_events.h"
#include "print_utils.h"
#include "bit_helpers.h"
#include <string.h>

#define UUID_LENGTH 16

// 5558ca9b-ab6b-4d0d-95a6-fa45388080c2 - service UUID
static uint8_t const weatherServiceUUIDBytes[UUID_LENGTH] = { 0x55, 0x58, 0xCA, 0x9B, 0xAB, 0x6B, 0x4D, 0x0D, 0x95,
		0xA6, 0xFA, 0x45, 0x38, 0x80, 0x80, 0xC2 };

// 5558caa0-ab6b-4d0d-95a6-fa45388080c2 - time characteristic
// Contains 3 bytes, in order from MSB: hour, minute, second.
// Read/write.
static uint8_t const timeCharUUIDBytes[UUID_LENGTH] = { 0x55, 0x58, 0xCA, 0xA0, 0xAB, 0x6B, 0x4D, 0x0D, 0x95, 0xA6,
		0xFA, 0x45, 0x38, 0x80, 0x80, 0xC2 };

// 5558caa1-ab6b-4d0d-95a6-fa45388080c2 - date characteristic
// Contains 4 bytes, in order from MSB: year (last two digits), month, day and weekday (1-7)
// Read/write.
static uint8_t const dateCharUUIDBytes[UUID_LENGTH] = { 0x55, 0x58, 0xCA, 0xA1, 0xAB, 0x6B, 0x4D, 0x0D, 0x95, 0xA6,
		0xFA, 0x45, 0x38, 0x80, 0x80, 0xC2 };

// 5558caa2-ab6b-4d0d-95a6-fa45388080c2 - temperature characteristic
// Contains 2-byte temperature value, stored as fixed-point integer
// with 2 digits of precision, multiplied by 100.
// After reading, divide it by 100 to get the correct value.
// For example, if you read 2034 - that means the temperature is 20.34*C
// Read-only.
static uint8_t const temperatureCharUUIDBytes[UUID_LENGTH] = { 0x55, 0x58, 0xCA, 0xA2, 0xAB, 0x6B, 0x4D, 0x0D, 0x95,
		0xA6, 0xFA, 0x45, 0x38, 0x80, 0x80, 0xC2 };

// 5558caa3-ab6b-4d0d-95a6-fa45388080c2 - pressure characteristic
// Stored the same way as temperature.
// Read-only.
static uint8_t const pressureCharUUIDBytes[UUID_LENGTH] = { 0x55, 0x58, 0xCA, 0xA3, 0xAB, 0x6B, 0x4D, 0x0D, 0x95, 0xA6,
		0xFA, 0x45, 0x38, 0x80, 0x80, 0xC2 };

// 5558caa4-ab6b-4d0d-95a6-fa45388080c2 - humidity characteristic
// Stored the same way as temperature.
// Read-only.
static uint8_t const humidityCharUUIDBytes[UUID_LENGTH] = { 0x55, 0x58, 0xCA, 0xA4, 0xAB, 0x6B, 0x4D, 0x0D, 0x95, 0xA6,
		0xFA, 0x45, 0x38, 0x80, 0x80, 0xC2 };

// 5558caa5-ab6b-4d0d-95a6-fa45388080c2 - control characteristic
// Single-byte value, read/write/notify. Valid values are:
// 0x00 - DEFAULT
//	      This is the default value of this characteristic.
// 		  If reading from it will return anything else, it means that the device
//		  is currently busy. The device will automatically set this
//		  characteristic' value to this after finishing operations, so make sure you
//		  are notified about it.
// 0x01 - GET_DATA
//		  Begins the process of data fetching. The number of available records is stored
//		  in 'numberOfRecords' characteristic.
// 0x02 - NEXT_RECORD_AVAILABLE
//		  As explained above, this value indicates that there's a record
//		  available to be read. After reading the data, either set this char to
//		  FETCH_NEXT_RECORD, or ABORT_FETCHING.
// 0x03 - FETCH_NEXT_RECORD
//		  As explained above, this will either put values of next record in chars
//		  and set this char to NEXT_RECORD_AVAILABLE, or DEFAULT if no more records
//		  are saved.
// 0x04 - ABORT_FETCHING
//		  Cancels the fetch procedure and returns this char to DEFAULT state
// 0x05 - SET_MEASUREMENT_INTERVAL
//		  This command sets the measurement interval of weather station.
//		  Before invoking that, set the specified interval in time characteristic.
// 0x06 - SET_DATE_AND_TIME
//		  This command makes the device read date and time values from BLE characteristics,
//		  and sets them. Obviously, you have to set the date and time chars before invoking this.
//
// Values set by connected device:
//	* GET_DATA
//	* FETCH_NEXT_RECORD
//	* ABORT_FETCHING
//	* SET_MEASUREMENT_INTERVAL
//	* SET_DATE_AND_TIME
// Value set by weather station:
//	* DEFAULT
//	* NEXT_RECORD_AVAILABLE
static uint8_t const controlCharUUIDBytes[UUID_LENGTH] = { 0x55, 0x58, 0xCA, 0xA5, 0xAB, 0x6B, 0x4D, 0x0D, 0x95, 0xA6,
		0xFA, 0x45, 0x38, 0x80, 0x80, 0xC2 };

// 5558caa5-ab6b-4d0d-95a6-fa45388080c2 - numberOfRecords characteristic
// 2-byte integer, read/notify. Is always set to number of available records
// on the device, that can be fetched with GET_DATA command.
static uint8_t const numberOfRecordsCharUUIDBytes[UUID_LENGTH] = { 0x55, 0x58, 0xCA, 0xA6, 0xAB, 0x6B, 0x4D, 0x0D, 0x95, 0xA6,
		0xFA, 0x45, 0x38, 0x80, 0x80, 0xC2 };

static Service_UUID_t weatherServiceUUID;
static Char_UUID_t timeCharUUID;
static Char_UUID_t dateCharUUID;
static Char_UUID_t temperatureCharUUID;
static Char_UUID_t pressureCharUUID;
static Char_UUID_t humidityCharUUID;
static Char_UUID_t controlCharUUID;
static Char_UUID_t numberOfRecordsCharUUID;

static uint16_t weatherServiceHandle;
static uint16_t timeCharHandle;
static uint16_t dateCharHandle;
static uint16_t temperatureCharHandle;
static uint16_t pressureCharHandle;
static uint16_t humidityCharHandle;
static uint16_t controlCharHandle;
static uint16_t numberOfRecordsCharHandle;

static uint16_t* const charIDBindTable[] = { &timeCharHandle, &dateCharHandle, &temperatureCharHandle,
		&pressureCharHandle, &humidityCharHandle, &controlCharHandle, &numberOfRecordsCharHandle };

#define CHAR_VALUE_OFFSET 1

void copy_reversed_uuid(uint8_t const* const source, uint8_t* destination);

void add_app_services() {
	tBleStatus status = BLE_STATUS_SUCCESS;

	copy_reversed_uuid(weatherServiceUUIDBytes, weatherServiceUUID.Service_UUID_128);
	copy_reversed_uuid(timeCharUUIDBytes, timeCharUUID.Char_UUID_128);
	copy_reversed_uuid(dateCharUUIDBytes, dateCharUUID.Char_UUID_128);
	copy_reversed_uuid(temperatureCharUUIDBytes, temperatureCharUUID.Char_UUID_128);
	copy_reversed_uuid(pressureCharUUIDBytes, pressureCharUUID.Char_UUID_128);
	copy_reversed_uuid(humidityCharUUIDBytes, humidityCharUUID.Char_UUID_128);
	copy_reversed_uuid(controlCharUUIDBytes, controlCharUUID.Char_UUID_128);
	copy_reversed_uuid(numberOfRecordsCharUUIDBytes, numberOfRecordsCharUUID.Char_UUID_128);

	// CALCULATING MAX ATTRIBUTE RECORDS:
	// At least 1 byte is required for service itself.
	// On top of that, add 3 bytes per characteristic. Characteristic value length doesn't matter.

	// @formatter:off
	status = aci_gatt_add_service(
			UUID_TYPE_128, // UUID type
			&weatherServiceUUID, // service UUID
			PRIMARY_SERVICE, // service type
			1+(7*3), // max attribute records, 1 + (numOfChars*3)
			&weatherServiceHandle // service handle
	);
						// @formatter:on
	if (status != BLE_STATUS_SUCCESS) {
		debugPrint("Couldn't add weather station service!");
		return;
	} else {
		debugPrint("Added weather station service, handle: 0x%04X", weatherServiceHandle);
	}

	// @formatter:off
	status = aci_gatt_add_char(
			 weatherServiceHandle, // service handle
			 UUID_TYPE_128, // UUID type
			 &timeCharUUID, // UUID
			 3, // value length (bytes)
			 CHAR_PROP_READ | CHAR_PROP_WRITE, // properties
			 ATTR_PERMISSION_NONE, // permissions
			 GATT_NOTIFY_ATTRIBUTE_WRITE, // event mask
			 16, // enc key size
			 0, // is available
			 &timeCharHandle // handle
	);
						// @formatter:on
	if (status != BLE_STATUS_SUCCESS) {
		debugPrint("Couldn't add time characteristic!");
		return;
	} else {
		debugPrint("Added time characteristic, handle: 0x%04X", timeCharHandle);
	}

	// @formatter:off
	status = aci_gatt_add_char(
			 weatherServiceHandle, // service handle
			 UUID_TYPE_128, // UUID type
			 &dateCharUUID, // UUID
			 4, // value length (bytes)
			 CHAR_PROP_READ | CHAR_PROP_WRITE, // properties
			 ATTR_PERMISSION_NONE, // permissions
			 GATT_NOTIFY_ATTRIBUTE_WRITE, // event mask
			 16, // enc key size
			 0, // is available
			 &dateCharHandle // handle
	);
						// @formatter:on
	if (status != BLE_STATUS_SUCCESS) {
		debugPrint("Couldn't add date characteristic!");
		return;
	} else {
		debugPrint("Added date characteristic, handle: 0x%04X", dateCharHandle);
	}

	// @formatter:off
	status = aci_gatt_add_char(
			 weatherServiceHandle, // service handle
			 UUID_TYPE_128, // UUID type
			 &temperatureCharUUID, // UUID
			 2, // value length (bytes)
			 CHAR_PROP_READ, // properties
			 ATTR_PERMISSION_NONE, // permissions
			 GATT_DONT_NOTIFY_EVENTS, // event mask
			 16, // enc key size
			 0, // is available
			 &temperatureCharHandle // handle
	);
						// @formatter:on
	if (status != BLE_STATUS_SUCCESS) {
		debugPrint("Couldn't add temperature characteristic!");
		return;
	} else {
		debugPrint("Added temperature characteristic, handle: 0x%04X", temperatureCharHandle);
	}

	// @formatter:off
	status = aci_gatt_add_char(
			 weatherServiceHandle, // service handle
			 UUID_TYPE_128, // UUID type
			 &pressureCharUUID, // UUID
			 2, // value length (bytes)
			 CHAR_PROP_READ, // properties
			 ATTR_PERMISSION_NONE, // permissions
			 GATT_DONT_NOTIFY_EVENTS, // event mask
			 16, // enc key size
			 0, // is available
			 &pressureCharHandle // handle
	);
						// @formatter:on
	if (status != BLE_STATUS_SUCCESS) {
		debugPrint("Couldn't add pressure characteristic!");
		return;
	} else {
		debugPrint("Added pressure characteristic, handle: 0x%04X", pressureCharHandle);
	}

	// @formatter:off
	status = aci_gatt_add_char(
			 weatherServiceHandle, // service handle
			 UUID_TYPE_128, // UUID type
			 &humidityCharUUID, // UUID
			 2, // value length (bytes)
			 CHAR_PROP_READ, // properties
			 ATTR_PERMISSION_NONE, // permissions
			 GATT_DONT_NOTIFY_EVENTS, // event mask
			 16, // enc key size
			 0, // is available
			 &humidityCharHandle // handle
	);
						// @formatter:on
	if (status != BLE_STATUS_SUCCESS) {
		debugPrint("Couldn't add humidity characteristic!");
		return;
	} else {
		debugPrint("Added humidity characteristic, handle: 0x%04X", humidityCharHandle);
	}

	// @formatter:off
	status = aci_gatt_add_char(
			 weatherServiceHandle, // service handle
			 UUID_TYPE_128, // UUID type
			 &controlCharUUID, // UUID
			 1, // value length (bytes)
			 CHAR_PROP_READ | CHAR_PROP_WRITE | CHAR_PROP_NOTIFY, // properties
			 ATTR_PERMISSION_NONE, // permissions
			 GATT_NOTIFY_ATTRIBUTE_WRITE, // event mask
			 16, // enc key size
			 0, // is available
			 &controlCharHandle // handle
	);
						// @formatter:on
	if (status != BLE_STATUS_SUCCESS) {
		debugPrint("Couldn't add control characteristic!");
		return;
	} else {
		debugPrint("Added control characteristic, handle: 0x%04X", controlCharHandle);
	}

	// @formatter:off
	status = aci_gatt_add_char(
			 weatherServiceHandle, // service handle
			 UUID_TYPE_128, // UUID type
			 &numberOfRecordsCharUUID, // UUID
			 2, // value length (bytes)
			 CHAR_PROP_READ | CHAR_PROP_NOTIFY, // properties
			 ATTR_PERMISSION_NONE, // permissions
			 GATT_DONT_NOTIFY_EVENTS, // event mask
			 16, // enc key size
			 0, // is available
			 &numberOfRecordsCharHandle // handle
	);
						// @formatter:on
	if (status != BLE_STATUS_SUCCESS) {
		debugPrint("Couldn't add numberOfRecords characteristic!");
		return;
	} else {
		debugPrint("Added numberOfRecords characteristic, handle: 0x%04X", numberOfRecordsCharHandle);
	}

}

void invert_byte_order(uint8_t data[], size_t length) {
	uint8_t* startPtr = data;
	uint8_t* endPtr = data + length - 1;
	while (startPtr < endPtr) {
		uint8_t tempValue = *startPtr;
		*startPtr = *endPtr;
		*endPtr = tempValue;
		startPtr++;
		endPtr--;
	}
}

void hci_le_connection_complete_event(uint8_t Status, uint16_t Connection_Handle, uint8_t Role,
		uint8_t Peer_Address_Type, uint8_t Peer_Address[6], uint16_t Conn_Interval, uint16_t Conn_Latency,
		uint16_t Supervision_Timeout, uint8_t Master_Clock_Accuracy) {
	debugPrint(
			"Device connected! Status: 0x%02X, connection handle: 0x%04X, role: %s, address type: %s, address: %02X:%02X:%02X:%02X:%02X:%02X",
			Status, Connection_Handle, (Role == 0x00 ? "Master" : "Slave"),
			(Peer_Address_Type == 0x00 ? "Public" : "Random"), Peer_Address[0], Peer_Address[1], Peer_Address[2],
			Peer_Address[3], Peer_Address[4], Peer_Address[5]);
}

void hci_disconnection_complete_event(uint8_t Status, uint16_t Connection_Handle, uint8_t Reason) {
	debugPrint("Device 0x%04X disconnected, reason code: 0x%02X, status: 0x%02X", Connection_Handle, Reason, Status);
}

void aci_gatt_attribute_modified_event(uint16_t Connection_Handle, uint16_t Attr_Handle, uint16_t Offset,
		uint16_t Attr_Data_Length, uint8_t Attr_Data[]) {
	debugPrint("Device 0x%04X modified GATT attribute 0x%04X (offset 0x%04X), data length: %d bytes", Connection_Handle,
			Attr_Handle, Offset, Attr_Data_Length);

	uint16_t const char_handle = Attr_Handle - CHAR_VALUE_OFFSET;

//	if (char_handle == testCharHandle) {
//		uint32_t const char_value = BYTEARRAY_TO_32BIT_VALUE_BE(Attr_Data);
//		setTestValue(char_value);
//		debugPrint("Set the char value to %lud (0x%08lX)", testValue(), testValue());
//	}

	BLECharacteristic charID = BLE_CHAR_INVALID;
	if (char_handle == timeCharHandle) {
		charID = BLE_CHAR_TIME;
	} else if (char_handle == dateCharHandle) {
		charID = BLE_CHAR_DATE;
	} else if (char_handle == temperatureCharHandle) {
		charID = BLE_CHAR_TEMPERATURE;
	} else if (char_handle == pressureCharHandle) {
		charID = BLE_CHAR_PRESSURE;
	} else if (char_handle == humidityCharHandle) {
		charID = BLE_CHAR_HUMIDITY;
	} else if (char_handle == controlCharHandle) {
		charID = BLE_CHAR_CONTROL;
	} else if (char_handle == numberOfRecordsCharHandle) {
		charID = BLE_CHAR_NUMBER_OF_RECORDS;
	}

//	invert_byte_order(Attr_Data, Attr_Data_Length);
	characteristic_value_changed(charID, Attr_Data, Attr_Data_Length);
}

void copy_reversed_uuid(uint8_t const* const source, uint8_t* destination) {
	uint8_t const* source_ptr = source + UUID_LENGTH - 1;
	for (uint8_t i = 0; i < UUID_LENGTH; i++) {
		*destination = *source_ptr;
		destination++;
		source_ptr--;
	}
}

__weak void characteristic_value_changed(BLECharacteristic characteristic, uint8_t data[], uint16_t length) {
	UNUSED(characteristic);
	UNUSED(data);
	UNUSED(length);
}

uint8_t set_characteristic_value(BLECharacteristic characteristic, uint8_t data[], uint16_t length) {
	if (characteristic >= BLE_CHAR_INVALID) {
		return 0xFF;
	}

	uint16_t const charHandle = *(charIDBindTable[(size_t) characteristic]);
	// BLE byte order is reversed
	invert_byte_order(data, length);
	return aci_gatt_update_char_value(weatherServiceHandle, charHandle, 0, length, data);
}
