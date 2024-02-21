/**
 * @file main.h
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Defines and includes
 * @version 0.1
 * @date 2024-02-13
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef _MAIN_H_
#define _MAIN_H_
#include <Arduino.h>
#include <WisBlock-API-V2.h>
#include <wisblock_cayenne.h>
#include "cayenne_lpp.h"
#include "modules.h"
#include <nrfx_power.h>
#include "debug.h"
#include "RAK14000_epd.h"

// RAK19024 Base Board
#define PIR_INT 25		// Interrupt pin for PIR
#define BUTTON_INT 24	// Input pin for Button
#define VOC_POWER 20	// VOC power enable pin
#define PIR_POWER 2		// PIR power enable pin
#define CO2_PM_POWER 28 // CO2 and PM power enable pin
#define EPD_POWER 34	// EPD power enable pin
#define SET_PIN WB_IO6	// PM sensor enable pin

/** Wakeup triggers for application events */
#define MOTION        0b1000000000000000
#define N_MOTION      0b0111111111111111
#define SEND_NOW      0b0100000000000000
#define N_SEND_NOW    0b1011111111111111
#define VOC_REQ       0b0010000000000000
#define N_VOC_REQ     0b1101111111111111
#define ROOM_EMPTY    0b0001000000000000
#define N_ROOM_EMPTY  0b1110111111111111
#define LED_REQ       0b0000100000000000
#define N_LED_REQ     0b1111011111111111
#define DISP_UPDATE   0b0000010000000000
#define N_DISP_UPDATE 0b1111101111111111
#define DISP_JOIN     0b0000001000000000
#define N_DISP_JOIN   0b1111110111111111
#define RST_REQ       0b0000000100000000
#define N_RST_REQ     0b1111111011111111
#define APP_EVENT     0b1111111110000001

// Structures and Unions
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

/** Define the version of your SW */
#ifndef SW_VERSION_1
#define SW_VERSION_1 1 // major version increase on API change / not backwards compatible
#define SW_VERSION_2 0 // minor version increase on API change / backward compatible
#define SW_VERSION_3 0 // patch version increase on bugfix, no affect on API
#endif

// Forward declarations
void send_delayed(TimerHandle_t unused);
void init_user_at(void);
void read_batt_settings(void);
void save_batt_settings(bool check_batt_enables);
void read_ui_settings(void);
void save_ui_settings(uint8_t ui_selected);

// Global Variables
extern WisCayenne g_solution_data;
extern date_time_s g_date_time;
extern SoftwareTimer g_sensor_timer;
extern SoftwareTimer voc_read_timer;
extern SoftwareTimer g_rgb_timer;
extern SoftwareTimer g_epd_off_timer;
extern bool has_rak1901;
extern bool has_rak1902;
extern bool has_rak1903;
extern bool has_rak1906;
extern bool has_rak12002;
extern bool has_rak12010;
extern bool has_rak12019;
extern bool has_rak12037;
extern bool has_rak12039;
extern bool has_rak12047;
extern bool g_has_rgb;
extern bool g_voc_valid;
extern float g_last_temp;
extern float g_last_humid;
extern float g_last_pressure;
extern float g_last_light_lux;
extern uint8_t g_ui_selected;
extern uint8_t g_ui_last;
extern uint8_t g_air_status;
extern bool g_status_changed;
extern bool g_occupied;
extern bool g_is_using_battery;
extern bool g_rgb_on;
extern time_t g_app_start_time;

#endif