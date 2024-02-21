/**
 * @file modules.h
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Functions for sensor modules, PIR, RGB, button
 * @version 0.1
 * @date 2024-02-08
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef _MODULES_H_
#define _MODULES_H_

/** Sensor functions */
bool init_rak1901(void);
void read_rak1901(void);
void get_rak1901_values(float *values);
void startup_rak1901(void);
void shutdown_rak1901(void);

bool init_rak1902(void);
void read_rak1902(void);
float get_rak1902(void);
void startup_rak1902(void);
void shutdown_rak1902(void);

bool init_rak1903(void);
void read_rak1903();
void startup_rak1903(void);
void shutdown_rak1903(void);

bool init_rak1906(void);
bool read_rak1906(void);
void get_rak1906_values(float *values);
void startup_rak1906(void);
void shutdown_rak1906(void);

bool init_rak12002(void);
void set_rak12002(uint16_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t minute);
void read_rak12002(void);

bool init_rak12010(void);
void read_rak12010();
void startup_rak12010(void);
void shutdown_rak12010(void);

bool init_rak12019(void);
void read_rak12019(void);

bool init_rak12037(void);
void read_rak12037(void);
void startup_rak12037(void);
void shutdown_rak12037(void);
uint16_t get_calib_rak12037(void);
bool force_calib_rak12037(uint16_t _concentration);
void get_rak12037_values(float *values);

bool init_rak12039(void);
void read_rak12039(void);
void startup_rak12039(void);
void shutdown_rak12039(void);

bool init_rak12047(void);
void read_rak12047(void);
void run_rak12047_algo(void);
void startup_rak12047(void);
void shutdown_rak12047(void);
void voc_read_wakeup(TimerHandle_t unused);

// RAK14000 EPD stuff
void init_rak14000(void);
void clear_rak14000(void);
void refresh_rak14000(void);
void set_voc_rak14000(uint16_t voc_value);
void switch_ui(void);
void rak14000_switch_bg(void);
void startup_rak14000(void);
void shutdown_rak14000(TimerHandle_t unused);

// RGB stuff
bool init_rgb(void);
void set_rgb_color(uint8_t red, uint8_t green, uint8_t blue);
void set_rgb_air_status(void);
void timer_rgb(void);
void rgb_timer_cb(TimerHandle_t unused);
void shutdown_rgb(void);

// PIR stuff
void init_pir(void);
void startup_pir(void);
void shutdown_pir(void);

// Button stuff
void init_button(void);
void check_button(void);

#endif // _MODULES_H_