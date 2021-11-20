/*
 * rtc_utils.h
 *
 *  Created on: Nov 18, 2021
 *      Author: steelph0enix
 */

#ifndef INC_RTC_UTILS_H_
#define INC_RTC_UTILS_H_

#include <stdint.h>
#include "rtc.h"

void setRTCAlarm(uint8_t hour, uint8_t minute, uint8_t second);
void setRTCTime(uint8_t hour, uint8_t minute, uint8_t second);
void setRTCDate(uint8_t year, uint8_t month, uint8_t day, uint8_t weekday);

void setRTCAlarmS(RTC_AlarmTypeDef alarm);
void setRTCTimeS(RTC_TimeTypeDef time);
void setRTCDateS(RTC_DateTypeDef date);

RTC_TimeTypeDef moveRTCAlarm(uint8_t hours, uint8_t minutes, uint8_t seconds);
RTC_TimeTypeDef moveRTCAlarmBySeconds(unsigned secondsSinceLastOne);
RTC_TimeTypeDef timestampAfter(RTC_TimeTypeDef timestamp, uint8_t hours, uint8_t minutes, uint8_t seconds);
RTC_TimeTypeDef timestampAfterSeconds(RTC_TimeTypeDef timestamp, unsigned seconds);

#endif /* INC_RTC_UTILS_H_ */
