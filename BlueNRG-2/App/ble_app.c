/*
 * ble_app.c
 *
 *  Created on: Nov 3, 2021
 *      Author: steelph0enix
 */

#include <ble_app_services.h>
#include "ble_app.h"
#include "print_utils.h"

#include "FreeRTOS.h"
#include "cmsis_os.h"

#include "hci.h"
#include "hci_tl.h"
#include "hci_const.h"
#include "bluenrg1_aci.h"
#include "bluenrg1_hci_le.h"
#include "bluenrg1_events.h"
#include "bluenrg_utils.h"

#include <string.h>

char const DeviceName[] = { AD_TYPE_COMPLETE_LOCAL_NAME, 'B', 'L', 'E', 'W', 'e', 'a', 't', 'h', 'e', 'r', 'S', 't',
		'a', 't', 'i', 'o', 'n' };
size_t const DeviceNameLength = 18;
uint16_t ble_discovery_time = 0;

void handle_hci_packet(void* pData);
tBleStatus get_bluenrg_version(uint8_t* hwVersion, uint8_t* fwVersion);
void set_discoverable();
void setup_device_address();

void ble_init() {
	// Setup and init BLE
	hci_init(handle_hci_packet, NULL);
	hci_reset();

	// Wait for the module to wake up
	osDelay(2000);

	// Check if module is working - read the version
	uint8_t ble_hw_version = 0;
	uint8_t ble_fw_version = 0;
	tBleStatus get_version_status = get_bluenrg_version(&ble_hw_version, &ble_fw_version);

	if (get_version_status == BLE_STATUS_SUCCESS) {
		debugPrint("BlueNRG version: 0x%02X HW, 0x%02X FW", ble_hw_version, ble_fw_version);
	} else {
		debugPrint("Cannot read firmware and hardware version from BlueNRG, error 0x%02X", get_version_status);
		return;
	}

	setup_device_address();

	// GATT Init
	tBleStatus gatt_init_status = aci_gatt_init();
	if (gatt_init_status == BLE_STATUS_SUCCESS) {
		debugPrint("GATT initialized!");
	} else {
		debugPrint("GATT initialization failed with code 0x%02X", gatt_init_status);
		return;
	}

	// GAP init
	uint16_t gap_service_handle = 0;
	uint16_t gap_device_name_char_handle = 0;
	uint16_t gap_appearance_char_handle = 0;
	tBleStatus gap_init_status = aci_gap_init(GAP_PERIPHERAL_ROLE, 0x00, (uint8_t) DeviceNameLength, &gap_service_handle,
			&gap_device_name_char_handle, &gap_appearance_char_handle);
	if (gap_init_status == BLE_STATUS_SUCCESS) {
		debugPrint("GAP initialized!");
	} else {
		debugPrint("GAP init failed with code 0x%02X", gap_init_status);
	}

	add_app_services();
	set_discoverable();
}

void ble_process() {
	hci_user_evt_proc();
}

tBleStatus get_bluenrg_version(uint8_t* hwVersion, uint8_t* fwVersion) {
	uint8_t hci_version = 0;
	uint8_t lmp_pal_version = 0;
	uint16_t hci_revision = 0;
	uint16_t manufacturer_name = 0;
	uint16_t lmp_pal_subversion = 0;

	tBleStatus status = hci_read_local_version_information(&hci_version, &hci_revision, &lmp_pal_version, &manufacturer_name,
			&lmp_pal_subversion);

	if (status == BLE_STATUS_SUCCESS) {
		*hwVersion = hci_revision >> 8;
		*fwVersion = (hci_revision & 0xFF) << 8;              // Major Version Number
		*fwVersion |= ((lmp_pal_subversion >> 4) & 0xF) << 4; // Minor Version Number
		*fwVersion |= lmp_pal_subversion & 0xF;               // Patch Version Number
	}
	return status;
}

void set_discoverable() {

	hci_le_set_scan_response_data(0, NULL);
	tBleStatus set_discoverable_status = aci_gap_set_discoverable(ADV_DATA_TYPE, ADV_INTERV_MIN, ADV_INTERV_MAX,
	PUBLIC_ADDR, NO_WHITE_LIST_USE, DeviceNameLength, (uint8_t*)DeviceName, 0, NULL, 0, 0);

	if (set_discoverable_status == BLE_STATUS_SUCCESS) {
		debugPrint("Device set as discoverable!");
	} else {
		debugPrint("Discoverable setting failed with code 0x%02X", set_discoverable_status);
	}
}

void setup_device_address() {
	uint8_t ble_addr[6] = { 0x00, 0x00, 0x00, 0xE1, 0x80, 0x02 };
	uint8_t random_number[8];

	tBleStatus rand_read_status = hci_le_rand(random_number);
	if (rand_read_status == BLE_STATUS_SUCCESS) {
		debugPrint("Got random numbers from BlueNRG");
	} else {
		debugPrint("Getting random numbers from BlueNRG failed with code 0x%02X", rand_read_status);
		return;
	}

	ble_discovery_time = 3000;
	for (uint8_t i = 0; i < 8; i++) {
		ble_discovery_time += (2 * random_number[i]);
	}

	ble_addr[0] = random_number[0];
	ble_addr[1] = random_number[3];
	ble_addr[2] = random_number[6];

	tBleStatus write_config_data_status = aci_hal_write_config_data(CONFIG_DATA_PUBADDR_OFFSET, CONFIG_DATA_PUBADDR_LEN,
			ble_addr);

	if (write_config_data_status == BLE_STATUS_SUCCESS) {
		debugPrint("BLE address set to %02X:%02X:%02X:%02X:%02X:%02X", ble_addr[5], ble_addr[4], ble_addr[3],
				ble_addr[2], ble_addr[1], ble_addr[0]);
	} else {
		debugPrint("BLE address setting failed with code 0x%02X", write_config_data_status);
	}
}

void handle_hci_packet(void* pData) {
	uint32_t i;

	hci_spi_pckt* hci_pckt = (hci_spi_pckt*) pData;

	if (hci_pckt->type == HCI_EVENT_PKT) {
		hci_event_pckt* event_pckt = (hci_event_pckt*) hci_pckt->data;

		if (event_pckt->evt == EVT_LE_META_EVENT) {
			evt_le_meta_event* evt = (void*) event_pckt->data;

			for (i = 0; i < (sizeof(hci_le_meta_events_table) / sizeof(hci_le_meta_events_table_type)); i++) {
				if (evt->subevent == hci_le_meta_events_table[i].evt_code) {
					hci_le_meta_events_table[i].process((void*) evt->data);
				}
			}
		} else if (event_pckt->evt == EVT_VENDOR) {
			evt_blue_aci* blue_evt = (void*) event_pckt->data;

			for (i = 0; i < (sizeof(hci_vendor_specific_events_table) / sizeof(hci_vendor_specific_events_table_type));
					i++) {
				if (blue_evt->ecode == hci_vendor_specific_events_table[i].evt_code) {
					hci_vendor_specific_events_table[i].process((void*) blue_evt->data);
				}
			}
		} else {
			for (i = 0; i < (sizeof(hci_events_table) / sizeof(hci_events_table_type)); i++) {
				if (event_pckt->evt == hci_events_table[i].evt_code) {
					hci_events_table[i].process((void*) event_pckt->data);
				}
			}
		}
	}
}
