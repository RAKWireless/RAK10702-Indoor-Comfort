/**
 * @file timer_cb.cpp
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Timer callbacks
 * @version 0.1
 * @date 2024-02-21
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "main.h"

/**
 * @brief Timer function used to avoid sending packages too often.
 * 			Delays the next package by 10 seconds
 *
 * @param unused Timer handle, not used
 */
void send_delayed(TimerHandle_t unused)
{
	api_wake_loop(SEND_NOW);
	g_sensor_timer.stop();
}

/**
 * @brief Timer callback to wakeup the loop with the VOC_REQ event
 *
 * @param unused
 */
void voc_read_wakeup(TimerHandle_t unused)
{
	// MYLOG("VOC", "VOC triggered");
	api_wake_loop(VOC_REQ);
}

/**
 * @brief Timer callback to switch off the RGB LED
 * 
 * @param unused 
 */
void rgb_timer_cb(TimerHandle_t unused)
{
	api_wake_loop(LED_REQ);
}
