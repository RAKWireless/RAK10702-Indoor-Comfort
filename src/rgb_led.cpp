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

TaskHandle_t rgb_task_handle;
void rgb_task(void *pvParameters);

void init_rgb(void)
{
	rgb.begin();
	rgb.setCurrent(1);
	rgb.setColor(255, 255, 0); // Yellow

	// if (!xTaskCreate(rgb_task, "RGB", 4096, NULL, TASK_PRIO_LOW, &rgb_task_handle))
	// {
	// 	MYLOG("EPD", "Failed to start RGB task");
	// }

	// rgb.setGradualDimmingUpEnd(uint8_t value);
	// rgb.setGradualDimmingDownEnd(1);
	// rgb.setGradualDimming(248);
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

/**
 * @brief Task to update the display
 *
 * @param pvParameters unused
 */
void rgb_task(void *pvParameters)
{
	MYLOG("RGB", "RGB Task started");

	bool up_down = false;
	uint8_t last_red = 128;
	uint8_t last_green = 128;
	uint8_t last_blue = 128;
	uint32_t light_pause = 25;

	while (1)
	{
		if (g_status_changed)
		{
			g_status_changed = false;
			last_red = 128;
			last_green = 128;
			last_blue = 128;
		}
		light_pause = 25;
		if (g_air_status == 0)
		{
			if (up_down)
			{
				last_blue = last_blue + 1;
				if (last_blue == 128)
				{
					up_down = false;
				}
			}
			else
			{
				last_blue = last_blue - 1;
				if (last_blue == 0)
				{
					up_down = true;
					light_pause = 5000;
				}
			}
			last_green = 0;
			last_red = 0;
		}
		if (g_air_status == 128)
		{
			if (up_down)
			{
				last_red = last_red + 1;
				last_green = last_green + 1;
				if (last_red == 128)
				{
					up_down = false;
				}
			}
			else
			{
				last_red = last_red + 1;
				last_green = last_green + 1;
				if (last_red == 0)
				{
					up_down = true;
					light_pause = 5000;
				}
			}
			last_blue = 0;
		}
		if (g_air_status == 255)
		{
			if (up_down)
			{
				last_red = last_red + 1;
				if (last_red == 128)
				{
					up_down = false;
				}
			}
			else
			{
				last_red = last_red + 1;
				if (last_red == 0)
				{
					up_down = true;
					light_pause = 5000;
				}
			}
			last_blue = 0;
			last_green = 0;
		}
		set_rgb_color(last_red, last_green, last_blue);

		vTaskDelay(ms2tick(light_pause));
	}
}
