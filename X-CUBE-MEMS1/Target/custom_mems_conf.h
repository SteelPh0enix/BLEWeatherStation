/**
 ******************************************************************************
 * @file    custom_mems_conf.h
 * @author  MEMS Application Team
 * @brief   This file contains definitions of the MEMS components bus interfaces for custom boards
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Software License Agreement SLA0077,
 * the “License”. You may not use this component except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        www.st.com/sla0077
 *
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CUSTOM_MEMS_CONF_H__
#define __CUSTOM_MEMS_CONF_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"
#include "stm32g4xx_nucleo_bus.h"
#include "stm32g4xx_nucleo_errno.h"

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

#define USE_CUSTOM_ENV_SENSOR_HTS221_0            1U

#define USE_CUSTOM_ENV_SENSOR_LPS22HB_0           1U

#define USE_CUSTOM_ENV_SENSOR_STTS751_0           1U

#define CUSTOM_HTS221_0_I2C_Init BSP_I2C1_Init
#define CUSTOM_HTS221_0_I2C_DeInit BSP_I2C1_DeInit
#define CUSTOM_HTS221_0_I2C_ReadReg BSP_I2C1_ReadReg
#define CUSTOM_HTS221_0_I2C_WriteReg BSP_I2C1_WriteReg

#define CUSTOM_LPS22HB_0_I2C_Init BSP_I2C1_Init
#define CUSTOM_LPS22HB_0_I2C_DeInit BSP_I2C1_DeInit
#define CUSTOM_LPS22HB_0_I2C_ReadReg BSP_I2C1_ReadReg
#define CUSTOM_LPS22HB_0_I2C_WriteReg BSP_I2C1_WriteReg

#define CUSTOM_STTS751_0_I2C_Init BSP_I2C1_Init
#define CUSTOM_STTS751_0_I2C_DeInit BSP_I2C1_DeInit
#define CUSTOM_STTS751_0_I2C_ReadReg BSP_I2C1_ReadReg
#define CUSTOM_STTS751_0_I2C_WriteReg BSP_I2C1_WriteReg

#ifdef __cplusplus
}
#endif

#endif /* __CUSTOM_MEMS_CONF_H__*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
