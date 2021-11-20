/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : app_freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */

/* List of I2C devices on the X-NUCLEO-IKS01A3 board
 * 0x32 - LIS2DW12 (accelerometer)
 * 0x3C - LIS2MDL (magnetometer)
 * 0x94 - STTS751 (temperature)
 * 0xBA - LPS22HB (pressure)
 * 0xBE - HTS221 (humidity and temperature)
 * 0xD6 - LSM6DSO (accelerometer and gyroscope)
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include "rtc_utils.h"
#include "ble_app.h"
#include "print_utils.h"
#include "mems_sensors.h"
#include "app_states.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
bool isTimeForUpdate = false;
/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = { .name = "defaultTask", .priority = (osPriority_t) osPriorityNormal,
		.stack_size = 512 * 4 };

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void scanI2CDevices(I2C_HandleTypeDef* i2c);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void* argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void) {
	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* creation of defaultTask */
	defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	/* USER CODE END RTOS_THREADS */

	/* USER CODE BEGIN RTOS_EVENTS */
	/* add events, ... */
	/* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void* argument) {
	/* USER CODE BEGIN StartDefaultTask */
	debugPrint("Hello, world!");
	mems_init();
	ble_init();
	app_set_measurement_interval(1, 0, 0);

	RTC_TimeTypeDef currentTime = { 0 };
	RTC_DateTypeDef currentDate = { 0 };
	HAL_RTC_GetTime(&hrtc, &currentTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &currentDate, RTC_FORMAT_BIN);

	debugPrint("Current time: %02d:%02d:%02d", currentTime.Hours, currentTime.Minutes, currentTime.Seconds);
	/* Infinite loop */
	for (;;) {
		osDelay(1);
		ble_process();

		if (isTimeForUpdate) {
			HAL_RTC_GetTime(&hrtc, &currentTime, RTC_FORMAT_BIN);
			HAL_RTC_GetDate(&hrtc, &currentDate, RTC_FORMAT_BIN);
			debugPrint("RTC alarm just happened @ %02d:%02d:%02d!", currentTime.Hours, currentTime.Minutes,
					currentTime.Seconds);
			isTimeForUpdate = false;
			app_rtc_alarm_handler();
		}
	}
	/* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef* hrtc) {
	isTimeForUpdate = true;
}

void scanI2CDevices(I2C_HandleTypeDef* i2c) {
	unsigned found_devices = 0;

	for (uint8_t address = 0x00; address <= 0x7F; address++) {
		HAL_StatusTypeDef scan_result = HAL_I2C_IsDeviceReady(i2c, address << 1, 5, 100);
		if (scan_result == HAL_OK) {
			printf("0x%02X ", address << 1);
			found_devices++;
		} else {
			printf(".... ");
		}

		if ((address + 1) % 0x10 == 0) {
			printf("\n");
		}
	}

	printf("\nFound %d devices!\n", found_devices);
}

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
