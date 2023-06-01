/**
 * @file app.h
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief For application specific includes and definitions
 *        Will be included from main.h
 * @version 0.2
 * @date 2022-01-30
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef APP_H
#define APP_H

//**********************************************/
//** Set the application firmware version here */
//**********************************************/
// major version increase on firmware change / not backwards compatible
#ifndef SW_VERSION_1
#define SW_VERSION_1 1
#endif
// minor version increase on firmware change / backward compatible
#ifndef SW_VERSION_2
#define SW_VERSION_2 1
#endif
// patch version increase on bugfix, no affect on firmware
#ifndef SW_VERSION_3
#define SW_VERSION_3 8
#endif

#include <Arduino.h>
// // REMOVE AFTER ALL CODE IS ADDED
// #include <Adafruit_I2CDevice.h>
// #include <Adafruit_Sensor.h>
// #include <Adafruit_GFX.h>

#include <Wire.h>
#include <bitset>

/** Include the WisBlock-API */
#include <WisBlock-API-V2.h>

// Debug output set to 0 to disable app debug output
#ifndef MY_DEBUG
#define MY_DEBUG 0
#endif

#if MY_DEBUG > 0
#define MYLOG(tag, ...)                     \
	do                                      \
	{                                       \
		if (tag)                            \
			PRINTF("[%s] ", tag);           \
		PRINTF(__VA_ARGS__);                \
		PRINTF("\n");                       \
		Serial.flush();                     \
		if (g_ble_uart_is_connected)        \
		{                                   \
			g_ble_uart.printf(__VA_ARGS__); \
			g_ble_uart.printf("\n");        \
		}                                   \
	} while (0)
#else
#define MYLOG(...)
#endif

/** Application function definitions */
void setup_app(void);
bool init_app(void);
void app_event_handler(void);
void ble_data_handler(void) __attribute__((weak));
void lora_data_handler(void);
void init_user_at(void);
void change_sendinterval(void);
extern SoftwareTimer delayed_sending;
void send_delayed(TimerHandle_t unused);
void do_rgb_toggle(TimerHandle_t unused);

extern uint8_t g_last_fport;
// extern bool g_no_usb;

/** Module stuff */
#include "modules.h"

/** Battery level uinion */
union batt_s
{
	uint16_t batt16 = 0;
	uint8_t batt8[2];
};

extern bool g_is_using_battery;

/** RTC date/time structure */
struct date_time_s
{
	uint16_t year;
	uint8_t month;
	uint8_t weekday;
	uint8_t date;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
};
extern date_time_s g_date_time;

#endif