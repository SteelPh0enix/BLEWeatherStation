/*
 * mems_sensors.h
 *
 *  Created on: Nov 13, 2021
 *      Author: steelph0enix
 */

#ifndef INC_MEMS_SENSORS_H_
#define INC_MEMS_SENSORS_H_

void mems_init();

float mems_get_temperature();
float mems_get_pressure();
float mems_get_humidity();

#endif /* INC_MEMS_SENSORS_H_ */
