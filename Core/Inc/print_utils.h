/*
 * print_utils.h
 *
 *  Created on: Oct 15, 2021
 *      Author: steelph0enix
 */

#ifndef INC_PRINT_UTILS_H_
#define INC_PRINT_UTILS_H_
#include <stdio.h>
#include <stdarg.h>
#include "main.h"

#ifdef DEBUG
#define DEBUG_PRINT 1
#else
#define DEBUG_PRINT 0
#endif

#define debugPrint(format, ...) \
		do { \
			if(DEBUG_PRINT) { \
				printf("[%ld] %s:%d:%s(): " format "\n", \
						HAL_GetTick(), __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
			} \
		} while(0)

#endif /* INC_PRINT_UTILS_H_ */
