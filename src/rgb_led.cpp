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

/** Flag if RGB LED driver was found */
bool g_has_rgb = false;
/**
 * @brief Initialize the RGB driver 
 * 
 */
void init_rgb(void)
{
	g_has_rgb = rgb.begin();
	if (!g_has_rgb)
	{
		MYLOG("RGB", "RGB not found");
		return;
	}
	rgb.setCurrent(1);
	rgb.setColor(255, 255, 0); // Yellow
	AT_PRINTF("+EVT:RGB OK")
}

/**
 * @brief Set the RGB LED object
 *
 * @param red value for red (0-255)
 * @param green value for green (0-255)
 * @param blue value for blue (0-255)
 */
void set_rgb_color(uint8_t red, uint8_t green, uint8_t blue)
{
	if (!g_has_rgb)
	{
		return;
	}
	
	if (g_is_using_battery)
	{
		MYLOG("RGB", "I2C might be off, switching power on");
	}
	digitalWrite(EPD_POWER, HIGH);
	delay(250);
	// If room is empty, switch off the LED's
	if (!g_occupied)
	{
		rgb.setColor(0, 0, 0);
		return;
	}
	rgb.setCurrent(1);
	rgb.setColor(red, green, blue);
}
