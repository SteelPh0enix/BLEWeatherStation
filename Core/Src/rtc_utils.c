/*
 * rtc_utils.c
 *
 *  Created on: Nov 18, 2021
 *      Author: steelph0enix
 */

#include "rtc_utils.h"
#include "print_utils.h"
#include <stdbool.h>

bool validateTime(RTC_TimeTypeDef* time) {
	bool const hours_valid = time->Hours < 24;
	bool const minutes_valid = time->Minutes < 60;
	bool const seconds_valid = time->Seconds < 60;

	return hours_valid && minutes_valid && seconds_valid;
}

bool validateDate(RTC_DateTypeDef* date) {
	bool const date_valid = date->Date <= 31 && date->Date != 0;
	bool const month_valid = date->Month <= 12 && date->Month != 0;
	bool const weekday_valid = date->WeekDay <= RTC_WEEKDAY_SUNDAY && date->WeekDay != 0;

	return date_valid && month_valid && weekday_valid;
}

bool validateAlarm(RTC_AlarmTypeDef* alarm) {
	bool const weekday_valid = alarm->AlarmDateWeekDay <= RTC_WEEKDAY_SUNDAY && alarm->AlarmDateWeekDay != 0;
	bool const time_valid = validateTime(&(alarm->AlarmTime));

	return weekday_valid && time_valid;
}

void setRTCAlarm(uint8_t week_day, uint8_t hour, uint8_t minute, uint8_t second) {
	if (!NVIC_GetEnableIRQ(RTC_Alarm_IRQn)) {
		HAL_NVIC_SetPriority(RTC_Alarm_IRQn, 5, 0);
		HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
	}

	RTC_AlarmTypeDef alarm = { 0 };
	alarm.AlarmTime.Hours = hour;
	alarm.AlarmTime.Minutes = minute;
	alarm.AlarmTime.Seconds = second;
	alarm.AlarmTime.SubSeconds = 0;
	alarm.AlarmMask = RTC_ALARMMASK_NONE;
	alarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
	alarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_WEEKDAY;
	alarm.AlarmDateWeekDay = week_day;
	alarm.Alarm = RTC_ALARM_A;

	setRTCAlarmS(alarm);
}

void setRTCTime(uint8_t hour, uint8_t minute, uint8_t second) {
	RTC_TimeTypeDef time = { 0 };
	time.Hours = hour;
	time.Minutes = minute;
	time.Seconds = second;
	time.SubSeconds = 0;
	time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	time.StoreOperation = RTC_STOREOPERATION_RESET;

	setRTCTimeS(time);
}

void setRTCDate(uint8_t year, uint8_t month, uint8_t day, uint8_t weekday) {
	RTC_DateTypeDef date = { 0 };
	date.Year = year;
	date.Month = month;
	date.Date = day;
	date.WeekDay = weekday;

	setRTCDateS(date);
}

void setRTCAlarmS(RTC_AlarmTypeDef alarm) {
	if (validateAlarm(&alarm)) {
		HAL_StatusTypeDef status = HAL_RTC_SetAlarm_IT(&hrtc, &alarm, RTC_FORMAT_BIN);
		if (status == HAL_OK) {
			debugPrint("Alarm set on weekday #%d, at %02d:%02d:%02d", alarm.AlarmDateWeekDay, alarm.AlarmTime.Hours,
					alarm.AlarmTime.Minutes, alarm.AlarmTime.Seconds);
		} else {
			debugPrint("Couldn't set alarm! Error code: 0x%02X", status);
		}
	} else {
		debugPrint("Couldn't set alarm, validation failed. Tried to set it to weekday #%d, at %02d:%02d:%02d",
				alarm.AlarmDateWeekDay, alarm.AlarmTime.Hours, alarm.AlarmTime.Minutes, alarm.AlarmTime.Seconds);
	}
}

void setRTCTimeS(RTC_TimeTypeDef time) {
	if (validateTime(&time)) {
		HAL_StatusTypeDef status = HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BIN);
		if (status == HAL_OK) {
			debugPrint("Time set to %02d:%02d:%02d", time.Hours, time.Minutes, time.Seconds);
		} else {
			debugPrint("Couldn't set RTC time!");
		}
	} else {
		debugPrint("Couldn't set time, validation failed. Tried to set it to %02d:%02d:%02d", time.Hours, time.Minutes,
				time.Seconds);
	}
}

void setRTCDateS(RTC_DateTypeDef date) {
	if (validateDate(&date)) {
		HAL_StatusTypeDef status = HAL_RTC_SetDate(&hrtc, &date, RTC_FORMAT_BIN);
		if (status == HAL_OK) {
			debugPrint("Date set to %02d.%02d.20%02d, weekday #%d", date.Date, date.Month, date.Year, date.WeekDay);
		} else {
			debugPrint("Couldn't set RTC date!");
		}
	} else {
		debugPrint("Couldn't set date, validation failed. Tried to set it to %02d.%02d.20%02d, weekday #%d", date.Date,
				date.Month, date.Year, date.WeekDay);
	}
}

RTC_TimeTypeDef moveRTCAlarm(uint8_t hours, uint8_t minutes, uint8_t seconds) {
	RTC_AlarmTypeDef alarm = { 0 };
	HAL_RTC_GetAlarm(&hrtc, &alarm, RTC_ALARM_A, RTC_FORMAT_BIN);

	RTC_TimeTypeDef newAlarmTime = timestampAfter(alarm.AlarmTime, hours, minutes, seconds);
	if (alarm.AlarmTime.Hours > newAlarmTime.Hours) {
		alarm.AlarmDateWeekDay++;
		if (alarm.AlarmDateWeekDay > RTC_WEEKDAY_SUNDAY) {
			alarm.AlarmDateWeekDay = RTC_WEEKDAY_MONDAY;
		}
	}

	alarm.AlarmTime = newAlarmTime;
	HAL_StatusTypeDef status = HAL_RTC_SetAlarm_IT(&hrtc, &alarm, RTC_FORMAT_BIN);
	if (status == HAL_OK) {
		debugPrint("Alarm moved by %02d:%02d:%02d", hours, minutes, seconds);
	} else {
		debugPrint("Couldn't move the alarm, error code: 0x%02X", status);
	}

	return newAlarmTime;
}

RTC_TimeTypeDef moveRTCAlarmBySeconds(unsigned secondsSinceLastOne) {
	RTC_AlarmTypeDef alarm = { 0 };
	HAL_RTC_GetAlarm(&hrtc, &alarm, RTC_ALARM_A, RTC_FORMAT_BIN);

	RTC_TimeTypeDef newAlarmTime = timestampAfterSeconds(alarm.AlarmTime, secondsSinceLastOne);
// Check if we passed midnight - if that's the case, then we have to set the alarm on next day
	if (alarm.AlarmTime.Hours > newAlarmTime.Hours) {
		alarm.AlarmDateWeekDay++;
		if (alarm.AlarmDateWeekDay > RTC_WEEKDAY_SUNDAY) {
			alarm.AlarmDateWeekDay = RTC_WEEKDAY_MONDAY;
		}
	}

	alarm.AlarmTime = newAlarmTime;
	HAL_StatusTypeDef status = HAL_RTC_SetAlarm_IT(&hrtc, &alarm, RTC_FORMAT_BIN);
	if (status == HAL_OK) {
		debugPrint("Alarm moved by %d seconds", secondsSinceLastOne);
	} else {
		debugPrint("Couldn't move the alarm, error code: 0x%02X", status);
	}

	return newAlarmTime;
}

RTC_TimeTypeDef timestampAfter(RTC_TimeTypeDef timestamp, uint8_t hours, uint8_t minutes, uint8_t seconds) {
	unsigned const newSeconds = ((unsigned) timestamp.Seconds) + seconds;
	unsigned const newMinutes = ((unsigned) timestamp.Minutes) + minutes + (newSeconds / 60);
	unsigned const newHours = ((unsigned) timestamp.Hours) + hours + (newMinutes / 60);

	timestamp.Seconds = newSeconds % 60;
	timestamp.Minutes = newMinutes % 60;
	timestamp.Hours = newHours % 24;

	return timestamp;
}

RTC_TimeTypeDef timestampAfterSeconds(RTC_TimeTypeDef timestamp, unsigned seconds) {
	unsigned const newSeconds = ((unsigned) timestamp.Seconds) + seconds;
	unsigned const newMinutes = ((unsigned) timestamp.Minutes) + (newSeconds / 60);
	unsigned const newHours = ((unsigned) timestamp.Hours) + (newMinutes / 60);

	timestamp.Seconds = newSeconds % 60;
	timestamp.Minutes = newMinutes % 60;
	timestamp.Hours = newHours % 24;

	return timestamp;
}

