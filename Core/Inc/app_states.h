/*
 * app_states.h
 *
 *  Created on: Nov 20, 2021
 *      Author: steelph0enix
 */

#ifndef INC_APP_STATES_H_
#define INC_APP_STATES_H_

#include <stdint.h>

typedef enum AppState_t {
	APP_STATE_IDLE,
	APP_STATE_MEASURING,
	APP_STATE_SETTING_INTERVAL,
	APP_STATE_SETTING_DATE_AND_TIME,
	APP_STATE_FETCHING_RECORDS,
	APP_STATE_RECORD_READY
} AppState;

AppState get_app_state();

void app_set_measurement_interval(uint8_t hours, uint8_t minutes, uint8_t seconds);

#endif /* INC_APP_STATES_H_ */
