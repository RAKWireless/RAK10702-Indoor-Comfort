/**
 * @file modules.h
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Globals and defines for module handling
 * @version 0.1
 * @date 2022-02-15
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <Arduino.h>

#ifndef MODULE_HANDLER_H
#define MODULE_HANDLER_H

/** Wakeup triggers for application events */
#define MOTION_TRIGGER   0b1000000000000000
#define N_MOTION_TRIGGER 0b0111111111111111
#define SEND_NOW         0b0100000000000000
#define N_SEND_NOW       0b1011111111111111
#define VOC_REQ          0b0010000000000000
#define N_VOC_REQ        0b1101111111111111
#define ROOM_EMPTY       0b0001000000000000
#define N_ROOM_EMPTY     0b1110111111111111
#define LED_REQ          0b0000100000000000
#define N_LED_REQ        0b1111011111111111
#define BSEC_REQ         0b0000010000000000
#define N_BSEC_REQ       0b1111101111111111
#define DISP_UPDATE      0b0000001000000000
#define N_DISP_UPDATE    0b1111110111111111
#define RST_REQ          0b0000000100000000
#define N_RST_REQ        0b1111111011111111

// Flags for modules
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

// LoRaWAN stuff
#include "wisblock_cayenne.h"
// Cayenne LPP Channel numbers per sensor value
#define LPP_CHANNEL_BATT 1			   // Base Board
#define LPP_CHANNEL_HUMID 2			   // RAK1901
#define LPP_CHANNEL_TEMP 3			   // RAK1901
#define LPP_CHANNEL_PRESS 4			   // RAK1902
#define LPP_CHANNEL_LIGHT 5			   // RAK1903
#define LPP_CHANNEL_HUMID_2 6		   // RAK1906
#define LPP_CHANNEL_TEMP_2 7		   // RAK1906
#define LPP_CHANNEL_PRESS_2 8		   // RAK1906
#define LPP_CHANNEL_GAS_2 9			   // RAK1906
#define LPP_CHANNEL_GPS 10			   // RAK1910/RAK12500
#define LPP_CHANNEL_SOIL_TEMP 11	   // RAK12035
#define LPP_CHANNEL_SOIL_HUMID 12	   // RAK12035
#define LPP_CHANNEL_SOIL_HUMID_RAW 13  // RAK12035
#define LPP_CHANNEL_SOIL_VALID 14	   // RAK12035
#define LPP_CHANNEL_LIGHT2 15		   // RAK12010
#define LPP_CHANNEL_VOC 16			   // RAK12047
#define LPP_CHANNEL_GAS 17			   // RAK12004
#define LPP_CHANNEL_GAS_PERC 18		   // RAK12004
#define LPP_CHANNEL_CO2 19			   // RAK12008
#define LPP_CHANNEL_CO2_PERC 20		   // RAK12008
#define LPP_CHANNEL_ALC 21			   // RAK12009
#define LPP_CHANNEL_ALC_PERC 22		   // RAK12009
#define LPP_CHANNEL_TOF 23			   // RAK12014
#define LPP_CHANNEL_TOF_VALID 24	   // RAK12014
#define LPP_CHANNEL_GYRO 25			   // RAK12025
#define LPP_CHANNEL_GESTURE 26		   // RAK14008
#define LPP_CHANNEL_UVI 27			   // RAK12019
#define LPP_CHANNEL_UVS 28			   // RAK12019
#define LPP_CHANNEL_CURRENT_CURRENT 29 // RAK16000
#define LPP_CHANNEL_CURRENT_VOLTAGE 30 // RAK16000
#define LPP_CHANNEL_CURRENT_POWER 31   // RAK16000
#define LPP_CHANNEL_TOUCH_1 32		   // RAK14002
#define LPP_CHANNEL_TOUCH_2 33		   // RAK14002
#define LPP_CHANNEL_TOUCH_3 34		   // RAK14002
#define LPP_CHANNEL_CO2_2 35		   // RAK12037
#define LPP_CHANNEL_CO2_Temp_2 36	   // RAK12037
#define LPP_CHANNEL_CO2_HUMID_2 37	   // RAK12037
#define LPP_CHANNEL_TEMP_3 38		   // RAK12003
#define LPP_CHANNEL_TEMP_4 39		   // RAK12003
#define LPP_CHANNEL_PM_1_0 40		   // RAK12039
#define LPP_CHANNEL_PM_2_5 41		   // RAK12039
#define LPP_CHANNEL_PM_10_0 42		   // RAK12039

extern WisCayenne g_solution_data;

// Index for known I2C devices
#define ACC_ID 0	   // RAK1904 accelerometer
#define LIGHT_ID 1	   // RAK1903 light sensor
#define GNSS_ID 2	   // RAK12500 GNSS sensor
#define PRESS_ID 3	   // RAK1902 barometric pressure sensor
#define TEMP_ID 4	   // RAK1901 temperature & humidity sensor
#define ENV_ID 5	   // RAK1906 environment sensor
#define SOIL_ID 6	   // RAK12035 soil moisture sensor
#define LIGHT2_ID 7	   // RAK12010 light sensor
#define EEPROM_ID 8	   // RAK15000 EEPROM module
#define MQ2_ID 9	   // RAK12004 MQ2 CO2 gas sensor
#define SCT31_ID 10	   // RAK12008 SCT31 CO2 gas sensor
#define MQ3_ID 11	   // RAK12009 MQ3 Alcohol gas sensor
#define TOF_ID 12	   // RAK12014 Laser ToF sensor
#define RTC_ID 13	   // RAK12002 RTC module
#define BAR_ID 14	   // RAK14003 LED bargraph module
#define VOC_ID 15	   // RAK12047 VOC sensor
#define GYRO_ID 16	   // RAK12025 Gyroscope
#define GESTURE_ID 17  // RAK14008 Gesture sensor
#define OLED_ID 18	   // RAK1921 OLED display
#define UVL_ID 19	   // RAK12019 UV light sensor
#define TOUCH_ID 20	   // RAK14002 Touch Pad
#define CURRENT_ID 21  // RAK16000 current sensor
#define MPU_ID 22	   // RAK1905 9DOF MPU9250 sensor
#define CO2_ID 23	   // RAK12037 CO2 sensor
#define FIR_ID 24	   // RAK12003 FIR temperature sensor
#define TEMP_ARR_ID 25 // RAK12040 Temp Array sensor
#define DOF_ID 26	   // RAK12034 9DOF BMX160 sensor
#define ACC2_ID 27	   // RAK12032 ADXL313 accelerometer
#define PM_ID 28	   // RAK12039 particle matter sensor

/** Sensor functions */
bool init_rak1901(void);
void read_rak1901(void);
void get_rak1901_values(float *values);
bool init_rak1902(void);
void read_rak1902(void);
float get_rak1902(void);
bool init_rak1903(void);
void read_rak1903();
bool init_rak1906(void);
void start_rak1906(void);
bool read_rak1906(void);
void get_rak1906_values(float *values);
bool init_rak12002(void);
void set_rak12002(uint16_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t minute);
void read_rak12002(void);
bool init_rak12010(void);
void read_rak12010();
bool init_rak12019(void);
void read_rak12019(void);
bool init_rak12037(void);
void read_rak12037(void);
bool init_rak12039(void);
void read_rak12039(void);
bool init_rak12047(void);
void read_rak12047(void);
void do_read_rak12047(void);
void power_rak12047(bool switch_on);
extern volatile bool g_voc_is_active;
extern float last_light_lux;

void find_modules(void);
void announce_modules(void);
void get_sensor_values(void);

// RAK14000 EPD stuff
void init_rak14000(void);
void wake_rak14000(void);
void clear_rak14000(void);
void refresh_rak14000(void);
void set_voc_rak14000(uint16_t voc_value);
extern bool voc_valid;
void set_temp_rak14000(float temp_value);
void set_humid_rak14000(float humid_value);
void set_baro_rak14000(float baro_value);
void set_co2_rak14000(float co2_value);
void set_pm_rak14000(uint16_t pm10_env, uint16_t pm25_env, uint16_t pm100_env);
void voc_rak14000(void);
void temp_rak14000(bool has_pm, bool has_baro);
void humid_rak14000(bool has_pm, bool has_baro);
void baro_rak14000(bool has_pm);
void co2_rak14000(bool has_pm);
void pm_rak14000(void);
void light_rak14000(bool has_pm);
void status_ui_rak14000(void);
void rak14000_start_screen(bool startup = true);
void rak14000_switch_bg(void);
void switch_ui(void);
extern volatile bool g_epd_off;
extern uint8_t g_ui_selected;
extern uint8_t g_ui_last;

// Sensor power functions
void power_modules(bool switch_on);
void power_i2c(bool switch_on);
void start_up_rak1901(void);
void shut_down_rak1901(void);
void startup_rak1902(void);
void shut_down_rak1902(void);
void startup_rak1903(void);
void shut_down_rak1903(void);
void startup_rak12010(void);
void shut_down_rak12010(void);
void startup_rak12019(void);
void shut_down_rak12019(void);
void startup_rak12037(void);
void shut_down_rak12037(void);
bool force_calib_rak12037(uint16_t _concentration);
uint16_t get_calib_rak12037(void);
void startup_rak12039(void);
void shut_down_rak12039(void);

// RGB stuff
void init_rgb(void);
void set_rgb_color(uint8_t red, uint8_t green, uint8_t blue);
void set_rgb_air_status(void);
extern uint8_t g_air_status;
extern bool g_status_changed;

// PIR stuff
void init_pir(void);
void shut_down_pir(void);
void startup_pir(void);
extern bool g_occupied;
#define PIR_INT 25 // WB_I2C2_SCL

// Button stuff
void init_button(void);
void check_button(void);
#if _CUSTOM_BOARD_ > 0
#define BUTTON_INT 24 // WB_I2C2_SDA
#else
#define BUTTON_INT WB_SW1
#endif
#ifndef TASK_PRIO_LOW
#define TASK_PRIO_LOW 1
#endif

// AT command read and save functions
void read_batt_settings(void);
void save_batt_settings(bool check_batt_enables);
void read_ui_settings(void);
void save_ui_settings(uint8_t ui_selected);

#if _CUSTOM_BOARD_ > 0
#define VOC_POWER 20	// PIN_SERIAL2_TX
#define PIR_POWER 2		// PIN_AREF
#define CO2_PM_POWER 28 // PIN_A2
#define EPD_POWER 34	// WB_IO2
#define SET_PIN WB_IO6
#else
#define VOC_POWER WB_IO2
#define PIR_POWER WB_IO2
#define CO2_PM_POWER WB_IO2 
#define EPD_POWER WB_IO2
#define SET_PIN WB_IO6 
#endif

extern bool g_sensors_off;
#endif