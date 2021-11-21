/*
 * mems_sensors.c
 *
 *  Created on: Nov 13, 2021
 *      Author: steelph0enix
 */

#include "mems_sensors.h"
#include "stm32g4xx_nucleo_bus.h"
#include "print_utils.h"
#include "hts221.h"
#include "lps22hb.h"
#include "stts751.h"

HTS221_Object_t hts;
LPS22HB_Object_t lps;
STTS751_Object_t stts;

float mems_get_temperature() {
	float ret = 0.f;
	uint8_t ready = 0;
	STTS751_Set_One_Shot(&stts);

	while (ready == 0) {
		STTS751_TEMP_Get_DRDY_Status(&stts, &ready);
	}

	if (STTS751_TEMP_GetTemperature(&stts, &ret) != STTS751_OK) {
		debugPrint("Couldn't read temperature correctly!");
	}
	return ret;
}

float mems_get_pressure() {
	float ret = 0.f;
	uint8_t ready = 0;
	LPS22HB_Set_One_Shot(&lps);

	while (ready == 0) {
		LPS22HB_PRESS_Get_DRDY_Status(&lps, &ready);
	}

	if (LPS22HB_PRESS_GetPressure(&lps, &ret) != LPS22HB_OK) {
		debugPrint("Couldn't read pressure correctly!");
	}
	return ret;
}

float mems_get_humidity() {
	float ret = 0.f;
	uint8_t ready = 0;
	HTS221_Set_One_Shot(&hts);

	while (ready == 0) {
		HTS221_HUM_Get_DRDY_Status(&hts, &ready);
	}

	if (HTS221_HUM_GetHumidity(&hts, &ret) != HTS221_OK) {
		debugPrint("Couldn't read humidity correctly!");
	}
	return ret;
}

int32_t get_tick_wrap() {
	return HAL_GetTick();
}

int32_t mems_config() {
	int32_t status = 0;

	status = HTS221_Set_One_Shot(&hts);
	if (status != HTS221_OK) {
		debugPrint("Error while setting HTS221 one shot mode: %ld", status);
		return status;
	}

	status = HTS221_HUM_Enable(&hts);
	if (status != HTS221_OK) {
		debugPrint("Error while enabling HTS221 humidity sensing: %ld", status);
		return status;
	}

	status = LPS22HB_Set_One_Shot(&lps);
	if (status != LPS22HB_OK) {
		debugPrint("Error while setting LPS22HB one shot mode: %ld", status);
		return status;
	}

	status = LPS22HB_PRESS_Enable(&lps);
	if (status != LPS22HB_OK) {
		debugPrint("Error while enabling LPS22HB pressure sensing: %ld", status);
		return status;
	}

	status = STTS751_Set_One_Shot(&stts);
	if (status != STTS751_OK) {
		debugPrint("Error while setting STTS751 one shot mode: %ld", status);
		return status;
	}

	status = STTS751_TEMP_Enable(&stts);
	if (status != LPS22HB_OK) {
		debugPrint("Error while enabling STTS751 temperature sensing: %ld", status);
		return status;
	}

	return 0;
}

void mems_init() {
	HTS221_IO_t hts_io = { .Init = BSP_I2C1_Init, .DeInit = BSP_I2C1_DeInit, .BusType = 0, .Address = 0xBE, .WriteReg =
			BSP_I2C1_WriteReg, .ReadReg = BSP_I2C1_ReadReg, .GetTick = get_tick_wrap };

	LPS22HB_IO_t lps_io = { .Init = BSP_I2C1_Init, .DeInit = BSP_I2C1_DeInit, .BusType = 0, .Address = 0xBA, .WriteReg =
			BSP_I2C1_WriteReg, .ReadReg = BSP_I2C1_ReadReg, .GetTick = get_tick_wrap };

	STTS751_IO_t stts_io = { .Init = BSP_I2C1_Init, .DeInit = BSP_I2C1_DeInit, .BusType = 0, .Address = 0x94,
			.WriteReg = BSP_I2C1_WriteReg, .ReadReg = BSP_I2C1_ReadReg, .GetTick = get_tick_wrap };

	int32_t retval = HTS221_RegisterBusIO(&hts, &hts_io);
	if (retval != HTS221_OK) {
		debugPrint("HTS221 IO registration or init failed with code %ld!", retval);
	} else {
		debugPrint("HTS221 IO registration and init successful!");
	}

	retval = LPS22HB_RegisterBusIO(&lps, &lps_io);
	if (retval != LPS22HB_OK) {
		debugPrint("LPS22HB IO registration or init failed with code %ld!", retval);
	} else {
		debugPrint("LPS22HB IO registration and init successful!");
	}

	retval = STTS751_RegisterBusIO(&stts, &stts_io);
	if (retval != STTS751_OK) {
		debugPrint("STTS751 IO registration or init failed with code %ld!", retval);
	} else {
		debugPrint("STTS751 IO registration and init successful!");
	}

	retval = HTS221_Init(&hts);
	if (retval != HTS221_OK) {
		debugPrint("HTS221 init failed with code %ld", retval);
	} else {
		debugPrint("HTS221 init successful!");
	}

	retval = LPS22HB_Init(&lps);
	if (retval != LPS22HB_OK) {
		debugPrint("LPS22HB init failed with code %ld", retval);
	} else {
		debugPrint("LPS22HB init successful!");
	}

	retval = STTS751_Init(&stts);
	if (retval != STTS751_OK) {
		debugPrint("STTS751 init failed with code %ld", retval);
	} else {
		debugPrint("STTS751 init successful!");
	}

	retval = mems_config();
	if (retval != 0) {
		debugPrint("There was an issue with MEMS configuration!");
	} else {
		debugPrint("MEMS sensors configured and ready to work!");
	}
}
