/**
 * @file led_rgb.cpp
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief RGB LED driver
 * @version 0.2
 * @date 2024-02-21
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "main.h"
#include <NCP5623.h>
#include "RAK14000_epd.h"

/** RGB LED instance */
NCP5623 rgb;

/** Flag if RGB LED driver was found */
bool g_has_rgb = false;

/** Timer for RGB control */
SoftwareTimer g_rgb_timer;

/** Flag if RGB is on or off */
bool g_rgb_on = false;

/**
 * @brief Initialize the RGB driver
 *
 */
bool init_rgb(void)
{
	g_has_rgb = rgb.begin();
	if (!g_has_rgb)
	{
		MYLOG("RGB", "RGB not found");
		return false;
	}
	rgb.setCurrent(1);
	rgb.setColor(255, 255, 0); // Yellow

	return true;
}

void timer_rgb(void)
{
	g_rgb_on = true;
	g_rgb_timer.begin(200, rgb_timer_cb, NULL, false);

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

	// If room is empty, switch off the LED's
	if (!g_occupied)
	{
		rgb.setColor(0, 0, 0);
		rgb.setCurrent(1);
		return;
	}

	rgb.setCurrent(1);

	rgb.setColor(red, green, blue);
}

void shutdown_rgb(void)
{
	rgb.setCurrent(0);
	rgb.shutDown();
	rgb.writeReg(0,0);
}

void set_rgb_air_status(void)
{
	uint8_t old_air_status = g_air_status;

	g_air_status = 0;

	if (has_rak12047)
	{
		// Get VOC status
		if (g_voc_valid)
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