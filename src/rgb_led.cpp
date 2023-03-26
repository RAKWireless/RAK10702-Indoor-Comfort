/**
 * @file rgb_led.cpp
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief RGB LED driver
 * @version 0.1
 * @date 2023-03-22
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "app.h"
#include <NCP5623.h>

/** RGB LED instance */
NCP5623 rgb;

void init_rgb(void)
{
	rgb.begin();
	rgb.setCurrent(1);
	rgb.setColor(255, 255, 0); // Yellow
}

void set_rgb_color(uint8_t red, uint8_t green, uint8_t blue)
{
	// If room is empty, switch off the LED's
	if (g_is_unoccupied)
	{
		rgb.setColor(0, 0, 0);
		return;
	}
	rgb.setCurrent(1);
	rgb.setColor(red, green, blue);
}
