/**
 * @file RAK12002_rtc.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Initialization and usage of RAK12002 RTC module
 * @version 0.2
 * @date 2024-02-21
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "main.h"
#include <Melopero_RV3028.h>

/** Instance of the RTC class */
Melopero_RV3028 rtc;

/** Structure for date and time from RTC */
date_time_s g_date_time;

/**
 * @brief Initialize the RTC
 *
 * @return true if success
 * @return false if failed
 */
bool init_rak12002(void)
{
	rtc.initI2C(Wire);

	rtc.useEEPROM();

	// Direct Switching Mode (DSM): when VDD < VBACKUP, switchover occurs from VDD to VBACKUP
	rtc.writeToRegister(0x35, 0x00);
	rtc.writeToRegister(0x37, 0xB4); 

	// Disable timers
	rtc.writeToRegister(0x0A, 0x00);
	rtc.writeToRegister(0x0B, 0x00);

	// Disable clock output
	uint8_t reg_value = rtc.readFromRegister(0x0F);
	reg_value = reg_value & 0b11111011;
	rtc.writeToRegister(0x0F, reg_value);

	// Disable interrupts
	reg_value = rtc.readFromRegister(0x10);
	reg_value = reg_value & 0b00000010;
	rtc.writeToRegister(0x10, reg_value);

	// Disable Clock Interrupts
	reg_value = rtc.readFromRegister(0x12);
	reg_value = reg_value & 0b00000000;
	rtc.writeToRegister(0x12, reg_value);

	// Disable EEPROM
	// rtc.useEEPROM(false);

	rtc.set24HourMode(); // Set the device to use the 24hour format (default) instead of the 12 hour format

	g_date_time.year = rtc.getYear();
	g_date_time.month = rtc.getMonth();
	g_date_time.weekday = rtc.getWeekday();
	g_date_time.date = rtc.getDate();
	g_date_time.hour = rtc.getHour();
	g_date_time.minute = rtc.getMinute();
	g_date_time.second = rtc.getSecond();

	if ((g_date_time.year == 2165) && (g_date_time.month == 165) && (g_date_time.date == 165))
	{
		MYLOG("RTC", "Returned values make no sense, no RTC attached");
		return false;
	}
	MYLOG("RTC", "%d.%02d.%02d %d:%02d:%02d", g_date_time.year, g_date_time.month, g_date_time.date, g_date_time.hour, g_date_time.minute, g_date_time.second);
	return true;
}

/**
 * @brief Set the RAK12002 date and time
 *
 * @param year in 4 digit format, e.g. 2020
 * @param month 1 to 12
 * @param date 1 to 31
 * @param hour 0 to 23
 * @param minute 0 to 59
 */
void set_rak12002(uint16_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t minute)
{
	Wire.begin();
	uint8_t weekday = (date + (uint16_t)((2.6 * month) - 0.2) - (2 * (year / 100)) + year + (uint16_t)(year / 4) + (uint16_t)(year / 400)) % 7;
	MYLOG("RTC", "Calculated weekday is %d", weekday);
	rtc.setTime(year, month, weekday, date, hour, minute, 0);
}

/**
 * @brief Update g_data_time structure with current the date
 *        and time from the RTC
 *
 */
void read_rak12002(void)
{
	g_date_time.year = rtc.getYear();
	g_date_time.month = rtc.getMonth();
	g_date_time.weekday = rtc.getWeekday();
	g_date_time.date = rtc.getDate();
	g_date_time.hour = rtc.getHour();
	g_date_time.minute = rtc.getMinute();
	g_date_time.second = rtc.getSecond();
}