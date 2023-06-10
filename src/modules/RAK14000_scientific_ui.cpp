/**
 * @file rak14000_scientific_ui.cpp
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Display functions for scientific UI
 * @version 0.1
 * @date 2023-03-26
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "app.h"

#include <Adafruit_GFX.h>
#include <Adafruit_EPD.h>

#include "RAK14000_epd.h"

void scientific_rak14000(void)
{
	uint8_t old_air_status = g_air_status;

	g_air_status = 0;

	bool has_baro = false;
	if (has_rak1902 || has_rak1906)
	{
		has_baro = true;
	}
	bool has_light = false;
	if (has_rak1903 || has_rak12010)
	{
		has_light = true;
	}
	if (has_rak12047)
	{
		voc_rak14000();
	}
	if (has_rak12037)
	{
		co2_rak14000(has_rak12039);
	}

	if (has_baro)
	{
		baro_rak14000(has_rak12039);
		temp_rak14000(has_rak12039, has_baro);
		humid_rak14000(has_rak12039, has_baro);
	}
	else
	{
		if (has_light)
		{
			light_rak14000(has_rak12039);
			temp_rak14000(has_rak12039, has_light);
			humid_rak14000(has_rak12039, has_light);
		}
		else
		{
			temp_rak14000(has_rak12039, false);
			humid_rak14000(has_rak12039, false);
		}
	}

	if (has_rak12039)
	{
		pm_rak14000();
	}

	if (old_air_status != g_air_status)
	{
		g_status_changed = true;
	}
	if (g_air_status == 0)
	{
		set_rgb_color(RGB_BLUE);
	}
	else if (g_air_status == 128)
	{
		set_rgb_color(RGB_YELLOW);
	}
	else
	{
		set_rgb_color(RGB_RED);
	}

	display.setFont(SMALL_FONT);
	display.setTextSize(1);
	if (has_rak12002)
	{
		read_rak12002();

		if (g_is_using_battery)
		{
			snprintf(disp_text, 59, "RAK10702 Indoor Comfort %s %d %d %02d:%02d Batt: %.2f V",
					 months_txt[g_date_time.month - 1], g_date_time.date, g_date_time.year,
					 g_date_time.hour, g_date_time.minute,
					 read_batt() / 1000.0);
		}
		else
		{
			snprintf(disp_text, 59, "RAK10702 Indoor Comfort %s %d %d %02d:%02d",
					 months_txt[g_date_time.month - 1], g_date_time.date, g_date_time.year,
					 g_date_time.hour, g_date_time.minute);
		}
	}
	else
	{
		if (g_is_using_battery)
		{
			snprintf(disp_text, 59, "RAK10702 Indoor Comfort Batt: %.2f V", read_batt() / 1000.0);
		}
		else
		{
			snprintf(disp_text, 59, "RAK10702 Indoor Comfort");
		}
	}

	display.getTextBounds(disp_text, 0, 0, &txt_x1, &txt_y1, &txt_w, &txt_h);
	text_rak14000((display_width / 2) - (txt_w / 2), 290, disp_text, (uint16_t)txt_color, 1);

	if (has_rak12039)
	{
		pm_rak14000();
		// Vertical divider
		display.drawLine(display_width / 2 + 50, 0, display_width / 2 + 50, display_height - 13, (uint16_t)txt_color);
		// Horizontal dividers
		display.drawLine(0, (display_height / 2 + 3) - 10, display_width / 2 + 50, (display_height / 2 + 3) - 10, (uint16_t)txt_color);
		display.drawLine(display_width / 2 + 50, (display_height / 5) - 10, display_width, (display_height / 5) - 10, (uint16_t)txt_color);
	}
	else
	{
		// Vertical divider
		display.drawLine(display_width / 2 + 50, 0, display_width / 2 + 50, display_height - 13, (uint16_t)txt_color);
		// Horizontal dividers
		display.drawLine(display_width / 2 + 50, (display_height / 3) - 10, display_width, (display_height / 3) - 10, (uint16_t)txt_color);
		display.drawLine(display_width / 2 + 50, (display_height / 3 * 2) - 10, display_width, (display_height / 3 * 2) - 10, (uint16_t)txt_color);
	}
}

/**
 * @brief Update display for VOC values
 *
 */
void voc_rak14000(void)
{
	x_text = 2;
	y_text = 1;
	s_text = 2;
	x_graph = 0;
	y_graph = 50;
	h_bar = display_height / 2 - 60;
	w_bar = 2;
	bar_divider = 500.0 / h_bar;

	// Write value
	display.drawBitmap(x_text, y_text, voc_img, 32, 32, txt_color);

	if (!voc_valid)
	{
		snprintf(disp_text, 29, "VOC na");
	}
	else
	{
		if (voc_values[voc_idx - 1] > 400)
		{
			snprintf(disp_text, 29, " !!  VOC %d", voc_values[voc_idx - 1]);
			if (g_air_status < 255)
			{
				g_air_status = 255;
			}
		}
		else if (voc_values[voc_idx - 1] > 250)
		{
			snprintf(disp_text, 29, " !  VOC %d", voc_values[voc_idx - 1]);
			if (g_air_status < 128)
			{
				g_air_status = 128;
			}
		}
		else
		{
			snprintf(disp_text, 29, "VOC %d", voc_values[voc_idx - 1]);
		}
	}
	text_rak14000(x_text + 40, y_text + 20, disp_text, txt_color, s_text);

	text_rak14000(display_width / 2 + 15, y_graph + h_bar - 7, (char *)"0", txt_color, 1);
	text_rak14000(display_width / 2 + 15, y_graph - 7, (char *)"500", txt_color, 1);

	display.drawLine(display_width / 2 + 10, y_graph + h_bar, display_width / 2 + 10, y_graph, (uint16_t)txt_color);
	display.drawLine(display_width / 2 + 5, y_graph + h_bar, display_width / 2 + 10, y_graph + h_bar, (uint16_t)txt_color);
	display.drawLine(display_width / 2 + 5, y_graph, display_width / 2 + 10, y_graph, (uint16_t)txt_color);

	// Draw VOC values
	for (int idx = 0; idx < num_values; idx++)
	{
		display.drawLine((int16_t)(x_graph + (idx * w_bar)),
						 (int16_t)(y_graph + ((h_bar) - (voc_values[idx] / bar_divider))),
						 (int16_t)(x_graph + (idx * w_bar)),
						 (int16_t)(y_graph + h_bar),
						 txt_color);
	}
	display.drawLine(x_graph, y_graph + h_bar, x_graph + display_width / 2, y_graph + h_bar, (uint16_t)txt_color);

	// For partial update only
	// MYLOG("EPD", "Updating x1 %d y1 %d x2 %d y2 %d", x_text, y_text, x_text + w_text, y_text + h_text);
	// display.displayPartial(x_text, y_text, x_text + w_text, y_text + h_text);
}

/**
 * @brief Update display for CO2 values
 *
 * @param has_pm changes display type and position if
 * 			PM sensor is connected
 * @endif
 *
 */
void co2_rak14000(bool has_pm)
{
	if (has_pm)
	{
		x_text = display_width / 2 + 53;
		y_text = 5;
		s_text = 2;
		spacer = 20;

		// Write value
		display.drawBitmap(x_text, y_text, co2_img, 32, 32, txt_color);

		snprintf(disp_text, 29, "ppm");
		display.setFont(SMALL_FONT);
		display.setTextSize(1);
		display.getTextBounds(disp_text, 0, 0, &txt_x1, &txt_y1, &txt_w, &txt_h);

		text_rak14000(display_width - txt_w - 1, y_text + spacer + 4, disp_text, (uint16_t)txt_color, 1);

		if (co2_values[co2_idx - 1] > 1500)
		{
			snprintf(disp_text, 29, "!! %.0f", co2_values[co2_idx - 1]);
			if (g_air_status < 255)
			{
				g_air_status = 255;
			}
		}
		else if (co2_values[co2_idx - 1] > 1000)
		{
			snprintf(disp_text, 29, "! %.0f", co2_values[co2_idx - 1]);
			if (g_air_status < 128)
			{
				g_air_status = 128;
			}
		}
		else
		{
			snprintf(disp_text, 29, "%.0f", co2_values[co2_idx - 1]);
		}

		display.setFont(LARGE_FONT);
		display.setTextSize(1);
		display.getTextBounds(disp_text, 0, 0, &txt_x1, &txt_y1, &txt_w2, &txt_h);

		text_rak14000(display_width - txt_w - txt_w2 - 4, y_text + spacer, disp_text, (uint16_t)txt_color, s_text);
	}
	else
	{
		x_text = 2;
		y_text = (display_height / 2) - 10;
		s_text = 2;
		x_graph = 0;
		y_graph = display_height / 2 + 40;
		h_bar = display_height / 2 - 62;
		w_bar = 2;
		bar_divider = 2500 / h_bar;

		// Get min and max values => maybe adjust graph to the min and max values
		int fmin = 2500;
		int fmax = 0;
		for (int idx = 0; idx < co2_idx; idx++)
		{
			if (co2_values[idx] <= fmin)
			{
				fmin = co2_values[idx];
			}
			if (co2_values[idx] >= fmax)
			{
				fmax = co2_values[idx];
			}
		}
		// give some margin at the top
		fmax += 50;

		// give some margin at the bottom
		if (fmin > 50)
		{
			fmin -= 50;
		}
		// make it an even number
		fmax = ((fmax / 100) + 1) * 100;
		bar_divider = fmax / h_bar;

		MYLOG("EPD", "CO2 min %d max %d", fmin, fmax);

		// Write value
		display.drawBitmap(x_text, y_text, co2_img, 32, 32, txt_color);

		if (co2_values[co2_idx - 1] > 1500)
		{
			snprintf(disp_text, 29, "!!  %.0f", co2_values[co2_idx - 1]);
			if (g_air_status < 255)
			{
				g_air_status = 255;
			}
		}
		else if (co2_values[co2_idx - 1] > 1000)
		{
			snprintf(disp_text, 29, "!  %.0f", co2_values[co2_idx - 1]);
			if (g_air_status < 128)
			{
				g_air_status = 128;
			}
		}
		else
		{
			snprintf(disp_text, 29, "%.0f", co2_values[co2_idx - 1]);
		}
		text_rak14000(x_text + 40, y_text + 20, disp_text, txt_color, s_text);
		display.setFont(LARGE_FONT);
		display.setTextSize(1);
		display.getTextBounds(disp_text, 0, 0, &txt_x1, &txt_y1, &txt_w2, &txt_h);

		text_rak14000(x_text + 40 + txt_w2 + 3, y_text + 24, (char *)"ppm", txt_color, 1);

		sprintf(disp_text, "%d", fmax);
		text_rak14000(display_width / 2 + 15, y_graph + h_bar - 17, (char *)"200", txt_color, 1);
		text_rak14000(display_width / 2 + 15, y_graph + h_bar - 7, (char *)"ppm", txt_color, 1);
		// text_rak14000(display_width / 2 + 15, y_graph + h_bar - 7, (char *)"0ppm", txt_color, 1);
		text_rak14000(display_width / 2 + 15, y_graph - 7, disp_text, txt_color, 1);
		text_rak14000(display_width / 2 + 15, y_graph + 3, (char *)"ppm", txt_color, 1);

		display.drawLine(display_width / 2 + 10, y_graph + h_bar, display_width / 2 + 10, y_graph, (uint16_t)txt_color);
		display.drawLine(display_width / 2 + 5, y_graph + h_bar, display_width / 2 + 10, y_graph + h_bar, (uint16_t)txt_color);
		display.drawLine(display_width / 2 + 5, y_graph, display_width / 2 + 10, y_graph, (uint16_t)txt_color);

		// Draw CO2 values
		for (int idx = 0; idx < num_values; idx++)
		{
			// if (co2_values[idx] != 0.0)
			if (co2_values[idx] >= 200.0)
			{
				display.drawLine((int16_t)(x_graph + (idx * w_bar)),
								 //  (int16_t)(y_graph + ((h_bar) - (co2_values[idx] / bar_divider))),
								 (int16_t)(y_graph + ((h_bar) - ((co2_values[idx] - 200) / bar_divider))),
								 (int16_t)(x_graph + (idx * w_bar)),
								 (int16_t)(y_graph + h_bar),
								 txt_color);
			}
		}
		display.drawLine(x_graph, y_graph + h_bar, x_graph + display_width / 2, y_graph + h_bar, (uint16_t)txt_color);
	}
}

/**
 * @brief Update display with particle matter values
 *
 */
void pm_rak14000(void)
{
	x_text = display_width / 2 + 53;
	y_text = (display_height / 4) - 10;
	s_text = 2;

	// Write value
	uint8_t pm_value_warning = 0;

	// PM 1.0 levels
	if (pm10_values[pm_idx - 1] > 75)
	{
		if (g_air_status < 255)
		{
			g_air_status = 255;
		}
		pm_value_warning = 255;
	}
	else if (pm10_values[pm_idx - 1] > 35)
	{
		if (g_air_status < 128)
		{
			g_air_status = 128;
		}
		pm_value_warning = 128;
	}

	snprintf(disp_text, 29, "1.0:");
	text_rak14000(x_text, y_text + 60, disp_text, txt_color, s_text);

	snprintf(disp_text, 29, "%d", pm10_values[pm_idx - 1]);
	display.setFont(LARGE_FONT);
	display.setTextSize(1);
	display.getTextBounds(disp_text, 0, 0, &txt_x1, &txt_y1, &txt_w, &txt_h);
	text_rak14000(display_width - txt_w - 45, y_text + 60, disp_text, txt_color, s_text);
	snprintf(disp_text, 29, "%cg/m%c", 0x7F, 0x80);
	text_rak14000(display_width - 38, y_text + 65, disp_text, txt_color, 1);

	// PM 2.5 levels
	if (pm25_values[pm_idx - 1] > 75)
	{
		if (g_air_status < 255)
		{
			g_air_status = 255;
		}
		pm_value_warning = 255;
	}
	else if (pm25_values[pm_idx - 1] > 35)
	{
		if (g_air_status < 128)
		{
			g_air_status = 128;
		}
		pm_value_warning = 255;
	}

	snprintf(disp_text, 29, "2.5:");
	text_rak14000(x_text, y_text + 120, disp_text, txt_color, s_text);

	snprintf(disp_text, 29, "%d", pm25_values[pm_idx - 1]);
	display.setFont(LARGE_FONT);
	display.setTextSize(1);
	display.getTextBounds(disp_text, 0, 0, &txt_x1, &txt_y1, &txt_w, &txt_h);
	text_rak14000(display_width - txt_w - 45, y_text + 120, disp_text, txt_color, s_text);
	snprintf(disp_text, 29, "%cg/m%c", 0x7F, 0x80);
	text_rak14000(display_width - 38, y_text + 125, disp_text, txt_color, 1);

	// PM 10 levels
	if (pm100_values[pm_idx - 1] > 199)
	{
		if (g_air_status < 255)
		{
			g_air_status = 255;
		}
		pm_value_warning = 255;
	}
	else if (pm100_values[pm_idx - 1] > 150)
	{
		if (g_air_status < 128)
		{
			g_air_status = 128;
		}
		pm_value_warning = 128;
	}

	snprintf(disp_text, 29, "10:");
	text_rak14000(x_text, y_text + 180, disp_text, txt_color, s_text);

	snprintf(disp_text, 29, "%d", pm100_values[pm_idx - 1]);
	display.setFont(LARGE_FONT);
	display.setTextSize(1);
	display.getTextBounds(disp_text, 0, 0, &txt_x1, &txt_y1, &txt_w, &txt_h);
	text_rak14000(display_width - txt_w - 45, y_text + 180, disp_text, txt_color, s_text);
	snprintf(disp_text, 29, "%cg/m%c", 0x7F, 0x80);
	text_rak14000(display_width - 38, y_text + 185, disp_text, txt_color, 1);

	display.drawBitmap(x_text, y_text, pm_img, 32, 32, txt_color);

	if (pm_value_warning == 255)
	{
		snprintf(disp_text, 29, "PM !!");
	}
	else if (pm_value_warning == 128)
	{
		snprintf(disp_text, 29, "PM !");
	}
	else
	{
		snprintf(disp_text, 29, "PM");
	}
	text_rak14000(x_text + 40, y_text + 20, disp_text, txt_color, s_text);
}

/**
 * @brief Update display for temperature values
 *
 * @param has_pm changes display type and position if
 * 			PM sensor is connected
 * @param has_baro changes display position if
 * 			barometric pressure sensor is connected
 */
void temp_rak14000(bool has_pm, bool has_baro)
{
	x_text = 25;
	if (has_baro)
	{
		y_text = display_height / 2;
	}
	else
	{
		y_text = display_height / 4 + 95;
	}
	s_text = 2;
	spacer = 60;

	// If PM sensor is not available, position is different
	if (!has_pm)
	{
		x_text = display_width / 2 + 53;
		y_text = 12;
		s_text = 2;
		spacer = 50;

		// Write value
		display.drawBitmap(display_width - (display_width / 4 - 16), y_text, celsius_img, 32, 32, txt_color);

		snprintf(disp_text, 29, "~C");
		display.setFont(SMALL_FONT);
		display.setTextSize(1);
		display.getTextBounds(disp_text, 0, 0, &txt_x1, &txt_y1, &txt_w, &txt_h);

		text_rak14000(display_width - txt_w - 3, y_text + spacer + 4, disp_text, (uint16_t)txt_color, 1);

		snprintf(disp_text, 29, "%.2f ", temp_values[temp_idx - 1]);
		display.setFont(LARGE_FONT);
		display.setTextSize(1);
		display.getTextBounds(disp_text, 0, 0, &txt_x1, &txt_y1, &txt_w2, &txt_h);

		text_rak14000(display_width - txt_w - txt_w2 - 6, y_text + spacer, disp_text, (uint16_t)txt_color, s_text);
	}
	else
	{
		// Write value
		display.drawBitmap(x_text, y_text, celsius_img, 32, 32, txt_color);

		snprintf(disp_text, 29, "%.2f", temp_values[temp_idx - 1]);

		display.setFont(LARGE_FONT);
		display.setTextSize(1);
		display.getTextBounds(disp_text, 0, 0, &txt_x1, &txt_y1, &txt_w, &txt_h);

		text_rak14000(x_text + spacer, y_text + 16, disp_text, (uint16_t)txt_color, s_text);

		snprintf(disp_text, 29, "~C");
		text_rak14000(x_text + spacer + txt_w + 4, y_text + 16 + 4, disp_text, (uint16_t)txt_color, 1);
	}
}

/**
 * @brief Update display for humidity values
 *
 * @param has_pm changes display type and position if
 * 			PM sensor is connected
 * @param has_baro changes display position if
 * 			barometric pressure sensor is connected
 */
void humid_rak14000(bool has_pm, bool has_baro)
{
	x_text = 25;
	if (has_baro)
	{
		y_text = display_height / 2 + (display_height / 2 / 3);
	}
	else
	{
		y_text = (display_height / 4) + 155;
	}
	s_text = 2;
	spacer = 60;

	// If PM sensor is not available, position is different
	if (!has_pm)
	{
		x_text = display_width / 2 + 53;
		y_text = display_height / 3 + 15;
		s_text = 2;
		spacer = 50;

		// Write value
		display.drawBitmap(display_width - (display_width / 4 - 16), y_text, humidity_img, 32, 32, txt_color);

		snprintf(disp_text, 29, "%%RH");
		display.setFont(SMALL_FONT);
		display.setTextSize(1);
		display.getTextBounds(disp_text, 0, 0, &txt_x1, &txt_y1, &txt_w, &txt_h);

		text_rak14000(display_width - txt_w - 3, y_text + spacer + 4, disp_text, (uint16_t)txt_color, 1);

		snprintf(disp_text, 29, "%.2f ", humid_values[humid_idx - 1]);
		display.setFont(LARGE_FONT);
		display.setTextSize(1);
		display.getTextBounds(disp_text, 0, 0, &txt_x1, &txt_y1, &txt_w2, &txt_h);

		text_rak14000(display_width - txt_w - txt_w2 - 6, y_text + spacer, disp_text, (uint16_t)txt_color, s_text);
	}
	else
	{
		// Write value
		display.drawBitmap(x_text, y_text, humidity_img, 32, 32, txt_color);

		snprintf(disp_text, 29, "%.2f", humid_values[humid_idx - 1]);

		display.setFont(LARGE_FONT);
		display.setTextSize(1);
		display.getTextBounds(disp_text, 0, 0, &txt_x1, &txt_y1, &txt_w, &txt_h);

		text_rak14000(x_text + spacer, y_text + 16, disp_text, (uint16_t)txt_color, s_text);

		snprintf(disp_text, 29, "%%RH");
		text_rak14000(x_text + spacer + txt_w + 4, y_text + 16 + 4, disp_text, (uint16_t)txt_color, 1);
	}
}

/**
 * @brief Update display for barometric pressure
 *
 * @param has_pm changes display type and position if
 * 			PM sensor is connected
 */
void baro_rak14000(bool has_pm)
{
	x_text = 25;
	y_text = display_height / 2 + (display_height / 2 / 3 * 2);
	s_text = 2;
	spacer = 60;

	// If PM sensor is not available, position is different
	if (!has_pm)
	{
		x_text = display_width / 2 + 53;
		y_text = display_height / 3 * 2 + 15;
		s_text = 2;
		spacer = 50;

		// Write value
		display.drawBitmap(display_width - (display_width / 4 - 16), y_text, barometer_img, 32, 32, txt_color);

		snprintf(disp_text, 29, "mBar");
		display.setFont(SMALL_FONT);
		display.setTextSize(1);
		display.getTextBounds(disp_text, 0, 0, &txt_x1, &txt_y1, &txt_w, &txt_h);

		text_rak14000(display_width - txt_w - 3, y_text + spacer + 4, disp_text, (uint16_t)txt_color, 1);

		snprintf(disp_text, 29, "%.1f ", baro_values[baro_idx - 1]);
		display.setFont(LARGE_FONT);
		display.setTextSize(1);
		display.getTextBounds(disp_text, 0, 0, &txt_x1, &txt_y1, &txt_w2, &txt_h);

		text_rak14000(display_width - txt_w - txt_w2 - 6, y_text + spacer, disp_text, (uint16_t)txt_color, s_text);
	}
	else
	{
		// Write value
		display.drawBitmap(x_text, y_text, barometer_img, 32, 32, txt_color);

		snprintf(disp_text, 29, "%.2f", baro_values[baro_idx - 1]);

		display.setFont(LARGE_FONT);
		display.setTextSize(1);
		display.getTextBounds(disp_text, 0, 0, &txt_x1, &txt_y1, &txt_w, &txt_h);

		text_rak14000(x_text + spacer, y_text + 16, disp_text, (uint16_t)txt_color, s_text);

		snprintf(disp_text, 29, "mBar");
		text_rak14000(x_text + spacer + txt_w + 4, y_text + 16 + 4, disp_text, (uint16_t)txt_color, 1);
	}
}

/**
 * @brief Update display for light
 *
 * @param has_pm changes display type and position if
 * 			PM sensor is connected
 */
void light_rak14000(bool has_pm)
{
	x_text = 25;
	y_text = display_height / 2 + (display_height / 2 / 3 * 2);
	s_text = 2;
	spacer = 60;

	// If PM sensor is not available, position is different
	if (!has_pm)
	{
		x_text = display_width / 2 + 53;
		y_text = display_height / 3 * 2 + 15;
		s_text = 2;
		spacer = 50;

		// Write value
		display.drawBitmap(display_width - (display_width / 4 - 16), y_text, brightness_img, 32, 32, txt_color);

		snprintf(disp_text, 29, "Lux");
		display.setFont(SMALL_FONT);
		display.setTextSize(1);
		display.getTextBounds(disp_text, 0, 0, &txt_x1, &txt_y1, &txt_w, &txt_h);

		text_rak14000(display_width - txt_w - 3, y_text + spacer + 4, disp_text, (uint16_t)txt_color, 1);

		snprintf(disp_text, 29, "%.1f ", last_light_lux);
		display.setFont(LARGE_FONT);
		display.setTextSize(1);
		display.getTextBounds(disp_text, 0, 0, &txt_x1, &txt_y1, &txt_w2, &txt_h);

		text_rak14000(display_width - txt_w - txt_w2 - 6, y_text + spacer, disp_text, (uint16_t)txt_color, s_text);
	}
	else
	{
		// Write value
		display.drawBitmap(x_text, y_text, brightness_img, 32, 32, txt_color);

		snprintf(disp_text, 29, "%.2f", last_light_lux);

		display.setFont(LARGE_FONT);
		display.setTextSize(1);
		display.getTextBounds(disp_text, 0, 0, &txt_x1, &txt_y1, &txt_w, &txt_h);

		text_rak14000(x_text + spacer, y_text + 16, disp_text, (uint16_t)txt_color, s_text);

		snprintf(disp_text, 29, "Lux");
		text_rak14000(x_text + spacer + txt_w + 4, y_text + 16 + 4, disp_text, (uint16_t)txt_color, 1);
	}
}
