/**
 * @file led_rgb.cpp
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
#include "modules/RAK14000_epd.h"

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
		digitalWrite(EPD_POWER, HIGH);
	}

	// If room is empty, switch off the LED's
	if (!g_occupied)
	{
		rgb.setColor(0, 0, 0);
		return;
	}
	rgb.setCurrent(1);
	rgb.setColor(red, green, blue);
}

void set_rgb_air_status(void)
{
	uint8_t old_air_status = g_air_status;

	g_air_status = 0;

	if (has_rak12047)
	{
		// Get VOC status
		if (voc_valid)
		{
			if (voc_values[voc_idx - 1] > 400)
			{
				if (g_air_status < 255)
				{
					g_air_status = 255;
				}
			}
			else if (voc_values[voc_idx - 1] > 250)
			{
				if (g_air_status < 128)
				{
					g_air_status = 128;
				}
			}
		}
	}

	if (has_rak12037)
	{
		if (co2_values[co2_idx - 1] > 1500)
		{
			if (g_air_status < 255)
			{
				g_air_status = 255;
			}
		}
		else if (co2_values[co2_idx - 1] > 1000)
		{
			if (g_air_status < 128)
			{
				g_air_status = 128;
			}
		}
	}
	if (has_rak12039)
	{
		// PM 1.0 levels
		if (pm10_values[pm_idx - 1] > 75)
		{
			if (g_air_status < 255)
			{
				g_air_status = 255;
			}
		}
		else if (pm10_values[pm_idx - 1] > 35)
		{
			if (g_air_status < 128)
			{
				g_air_status = 128;
			}
		}
		// PM 2.5 levels
		if (pm25_values[pm_idx - 1] > 75)
		{
			if (g_air_status < 255)
			{
				g_air_status = 255;
			}
		}
		else if (pm25_values[pm_idx - 1] > 35)
		{
			if (g_air_status < 128)
			{
				g_air_status = 128;
			}
		}
		// PM 10 levels
		if (pm100_values[pm_idx - 1] > 199)
		{
			if (g_air_status < 255)
			{
				g_air_status = 255;
			}
		}
		else if (pm100_values[pm_idx - 1] > 150)
		{
			if (g_air_status < 128)
			{
				g_air_status = 128;
			}
		}
	}

	if (old_air_status != g_air_status)
	{
		g_status_changed = true;
	}

	if (g_air_status == 0)
	{
		set_rgb_color(RGB_BLUE);
	}
	else if (g_air_status < 255)
	{
		set_rgb_color(RGB_YELLOW);
	}
	else
	{
		set_rgb_color(RGB_RED);
	}
}