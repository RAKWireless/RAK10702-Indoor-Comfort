/**
 * @file RAK14000_epd_4_2.cpp
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Initialization and functions for EPD display
 * @version 0.2
 * @date 2024-02-21
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "main.h"

#include "RAK14000_epd.h"

uint16_t display_width = 400;
uint16_t display_height = 300;

// 4.2" EPD with SSD1683
Adafruit_SSD1681 display(display_height, display_width, EPD_MOSI,
						 EPD_SCK, EPD_DC, EPD_RESET,
						 EPD_CS, SRAM_CS, EPD_MISO,
						 EPD_BUSY);

/** Set num_values to 1/4 of the display width */

const uint16_t num_values = 400 / 4;
uint16_t voc_values[num_values] = {0};
float temp_values[num_values] = {0.0};
float humid_values[num_values] = {0.0};
float baro_values[num_values] = {0.0};
float co2_values[num_values] = {0.0};
uint16_t pm10_values[num_values] = {0};
uint16_t pm25_values[num_values] = {0};
uint16_t pm100_values[num_values] = {0};
uint8_t voc_idx = 0;
uint8_t temp_idx = 0;
uint8_t humid_idx = 0;
uint8_t baro_idx = 0;
uint8_t co2_idx = 0;
uint8_t pm_idx = 0;

char disp_text[60];

uint16_t bg_color = EPD_WHITE;
uint16_t txt_color = EPD_BLACK;

uint8_t g_air_status = 0;
bool g_status_changed = true;

// For text length calculations
int16_t txt_x1;
int16_t txt_y1;
uint16_t txt_w;
uint16_t txt_w2;
uint16_t txt_h;

// For text and image placements
uint16_t x_text;
uint16_t y_text;
uint16_t s_text;
uint16_t w_text;
uint16_t h_text;
uint16_t x_graph;
uint16_t y_graph;
uint16_t h_bar;
uint16_t w_bar;
float bar_divider;
uint16_t spacer;

/** UI selector. 0 = scientific, 1 = Icon, 2 = Status */
uint8_t g_ui_selected = 0;
/** Save last selected UI, in case Status screen is forced */
uint8_t g_ui_last = 0;
/** Flag for RAK14000 available */
bool has_rak14000 = false;
/** Timer for shutdown display */
SoftwareTimer g_epd_off_timer;

/**
 * @brief Initialization of RAK14000 EPD
 *
 */
void init_rak14000(void)
{
	has_rak14000 = true;

	display.begin();

	display.setRotation(EPD_ROTATION); // 1 for Gavin 3 for mine
	MYLOG("EPD", "Rotation %d", display.getRotation());

	read_ui_settings();

	rak14000_start_screen(true);

	g_epd_off_timer.begin(2000, shutdown_rak14000, NULL, false);
}

/**
 * @brief Set LoRa connection status
 *
 */
void status_lora_rak14000(void)
{
	if (g_lorawan_settings.lorawan_enable)
	{
		if (!g_lpwan_has_joined)
		{
			if (g_ui_selected == 1)
			{
				display.drawCircle(6, 6, 3, txt_color);
				display.drawCircle(14, 14, 3, txt_color);
				display.drawCircle(6, 6, 4, txt_color);
				display.drawCircle(14, 14, 4, txt_color);
			}
			else if (g_ui_selected == 0)
			{
				display.drawCircle(380, 6, 3, txt_color);
				display.drawCircle(388, 14, 3, txt_color);
				display.drawCircle(380, 6, 4, txt_color);
				display.drawCircle(388, 14, 4, txt_color);
			}
		}
		else
		{
			if (g_ui_selected == 1)
			{
				display.drawCircle(6, 6, 3, txt_color);
				display.drawCircle(10, 10, 3, txt_color);
				display.drawCircle(6, 6, 4, txt_color);
				display.drawCircle(10, 10, 4, txt_color);
			}
			else if (g_ui_selected == 0)
			{
				display.drawCircle(380, 6, 3, txt_color);
				display.drawCircle(384, 10, 3, txt_color);
				display.drawCircle(380, 6, 4, txt_color);
				display.drawCircle(384, 10, 4, txt_color);
			}
		}
	}
	else
	{
		if (g_ui_selected == 1)
		{
			display.drawCircle(6, 6, 5, txt_color);
			display.drawCircle(15, 6, 5, txt_color);
			display.drawCircle(6, 6, 4, txt_color);
			display.drawCircle(15, 6, 4, txt_color);
			display.drawLine(6, 6, 15, 6, txt_color);
		}
		else if (g_ui_selected == 0)
		{
			display.drawCircle(380, 6, 5, txt_color);
			display.drawCircle(389, 6, 5, txt_color);
			display.drawCircle(380, 6, 4, txt_color);
			display.drawCircle(389, 6, 4, txt_color);
			display.drawLine(380, 6, 389, 6, txt_color);
		}
	}
}

/**
   @brief Write a text on the display

   @param x x position to start
   @param y y position to start
   @param text text to write
   @param text_color color of text
   @param text_size size of text
*/
void text_rak14000(int16_t x, int16_t y, char *text, uint16_t text_color, uint32_t text_size)
{
	if (!has_rak14000)
	{
		return;
	}
	if (text_size == 1)
	{
		display.setFont(SMALL_FONT); // Font_5x7_practical8pt7b
		y = y + 7;
	}
	else
	{
		display.setFont(LARGE_FONT);
		y = y + 12;
	}
	display.setCursor(x, y);
	display.setTextColor(text_color);
	display.setTextSize(1);
	display.setTextWrap(false);
	display.print(text);
}

/**
 * @brief Clear display content
 *
 */
void clear_rak14000(void)
{
	if (!has_rak14000)
	{
		return;
	}
	display.clearBuffer();
	display.fillRect(0, 0, display_width, display_height, bg_color);
}

/**
 * @brief Switch the UI to the next version.
 *			Triggered by button
 *			g_ui_selected options
 *			0 = scientific
 *			1 = iconized
 *			2 = status screen (reset automatically on next update)
 */
void switch_ui(void)
{
	if (!has_rak14000)
	{
		return;
	}
	g_ui_selected += 1;
	if (g_ui_selected >= 2)
	{
		g_ui_selected = 0;
	}
	g_ui_last = g_ui_selected;
	api_wake_loop(DISP_UPDATE);
}

/** Flag for first screen update */
bool first_time = true;

char *months_txt[] = {(char *)"Jan", (char *)"Feb", (char *)"Mar", (char *)"Apr", (char *)"May", (char *)"Jun", (char *)"Jul", (char *)"Aug", (char *)"Sep", (char *)"Oct", (char *)"Nov", (char *)"Dec"};

/**
 * @brief Update screen content
 *
 */
void refresh_rak14000(void)
{
	if (!has_rak14000)
	{
		MYLOG("EPD", "No EPD to refresh");
		// No display detected, set RGB color
		set_rgb_air_status();
		return;
	}

	// Clear display buffer
	clear_rak14000();

	status_lora_rak14000();

	switch (g_ui_selected)
	{
	case 0:
		scientific_rak14000();
		break;
	case 1:
		icon_rak14000();
		break;
	case 2:
		status_ui_rak14000();
		break;
	}

	if (has_rak14000)
	{
		delay(100);

		display.display();
		delay(100);
	}
}

/**
 * @brief Add VOC value to buffer
 *
 * @param voc_value new VOC value
 */
void set_voc_rak14000(uint16_t voc_value)
{
	MYLOG("EPD", "VOC set to %d at index %d", voc_value, voc_idx);
	// Shift values if necessary
	if (voc_idx == num_values)
	{
		for (int idx = 0; idx < (num_values - 1); idx++)
		{
			voc_values[idx] = voc_values[idx + 1];
		}
		voc_idx = (num_values - 1);
	}

	// Fill VOC array
	voc_values[voc_idx] = voc_value;

	// Increase index
	voc_idx++;
}

/**
 * @brief Add temperature value to buffer
 *
 * @param temp_value new temperature value
 */
void set_temp_rak14000(float temp_value)
{
	MYLOG("EPD", "Temp set to %.2f at index %d", temp_value, temp_idx);
	// Shift values if necessary
	if (temp_idx == num_values)
	{
		for (int idx = 0; idx < (num_values - 1); idx++)
		{
			temp_values[idx] = temp_values[idx + 1];
		}
		temp_idx = (num_values - 1);
	}

	// Fill Temperature array
	temp_values[temp_idx] = temp_value;

	// Increase index
	temp_idx++;
}

/**
 * @brief Add humidity value to buffer
 *
 * @param humid_value new humidity value
 */
void set_humid_rak14000(float humid_value)
{
	MYLOG("EPD", "Humid set to %.2f at index %d", humid_value, humid_idx);
	// Shift values if necessary
	if (humid_idx == num_values)
	{
		for (int idx = 0; idx < (num_values - 1); idx++)
		{
			humid_values[idx] = humid_values[idx + 1];
		}
		humid_idx = (num_values - 1);
	}

	// Fill VOC array
	humid_values[humid_idx] = humid_value;

	// Increase index
	humid_idx++;
}

/**
 * @brief Add CO2 value to buffer
 *
 * @param co2_value new CO2 value
 */
void set_co2_rak14000(float co2_value)
{
	MYLOG("EPD", "CO2 set to %.2f at index %d", co2_value, co2_idx);
	// Shift values if necessary
	if (co2_idx == num_values)
	{
		for (int idx = 0; idx < (num_values - 1); idx++)
		{
			co2_values[idx] = co2_values[idx + 1];
		}
		co2_idx = (num_values - 1);
	}

	// Fill VOC array
	co2_values[co2_idx] = co2_value;

	// Increase index
	co2_idx++;
}

/**
 * @brief Add barometric pressure to buffer
 *
 * @param baro_value new barometric pressure
 */
void set_baro_rak14000(float baro_value)
{
	MYLOG("EPD", "Baro set to %.2f at index %d", baro_value, baro_idx);
	// Shift values if necessary
	if (baro_idx == num_values)
	{
		for (int idx = 0; idx < (num_values - 1); idx++)
		{
			baro_values[idx] = baro_values[idx + 1];
		}
		baro_idx = (num_values - 1);
	}

	// Fill Barometer array
	baro_values[baro_idx] = baro_value;

	// Increase index
	baro_idx++;
}

/**
 * @brief Add PM values to array
 *
 * @param pm10_env new PM 1.0 value
 * @param pm25_env new PM 2.5 value
 * @param pm100_env new PM 10 value
 */
void set_pm_rak14000(uint16_t pm10_env, uint16_t pm25_env, uint16_t pm100_env)
{
	MYLOG("EPD", "PM set to %d %d %d  at index %d", pm10_env, pm25_env, pm100_env, pm_idx);
	// Shift values if necessary
	if (pm_idx == num_values)
	{
		for (int idx = 0; idx < (num_values - 1); idx++)
		{
			pm10_values[idx] = pm10_values[idx + 1];
			pm25_values[idx] = pm25_values[idx + 1];
			pm100_values[idx] = pm100_values[idx + 1];
		}
		pm_idx = (num_values - 1);
	}

	// Fill PM array
	pm10_values[pm_idx] = pm10_env;
	pm25_values[pm_idx] = pm25_env;
	pm100_values[pm_idx] = pm100_env;

	// Increase index
	pm_idx++;
}

void rak14000_start_screen(bool startup)
{
	// Clear display
	display.clearBuffer();

	// Draw Welcome Logo
	display.fillRect(0, 0, display_width, display_height, bg_color);
	display.drawBitmap(display_width / 2 - 92, 40, rak_img, 184, 56, txt_color); // 184x56

	display.setFont(SMALL_FONT);
	display.setTextSize(1);

	// If RTC is available, write the date
	if (has_rak12002)
	{
		read_rak12002();
		if (g_is_using_battery)
		{
			snprintf(disp_text, 59, "%s %d %d %02d:%02d Batt: %.2f V",
					 months_txt[g_date_time.month - 1], g_date_time.date, g_date_time.year,
					 g_date_time.hour, g_date_time.minute,
					 read_batt() / 1000.0);
		}
		else
		{
			snprintf(disp_text, 59, "%s %d %d %02d:%02d",
					 months_txt[g_date_time.month - 1], g_date_time.date, g_date_time.year,
					 g_date_time.hour, g_date_time.minute);
		}
		display.getTextBounds(disp_text, 0, 0, &txt_x1, &txt_y1, &txt_w, &txt_h);
		text_rak14000((display_width / 2) - (txt_w / 2), 290, disp_text, (uint16_t)txt_color, 1);
	}

	display.setFont(LARGE_FONT);
	display.setTextSize(1);
	display.getTextBounds((char *)"IoT Made Easy", 0, 0, &txt_x1, &txt_y1, &txt_w, &txt_h);
	text_rak14000(display_width / 2 - (txt_w / 2), 110, (char *)"IoT Made Easy", (uint16_t)txt_color, 2);

	display.setFont(LARGE_FONT);
	display.setTextSize(1);
	display.getTextBounds((char *)"RAK10702 Indoor Comfort", 0, 0, &txt_x1, &txt_y1, &txt_w, &txt_h);
	text_rak14000(display_width / 2 - (txt_w / 2), 150, (char *)"RAK10702 Indoor Comfort", (uint16_t)txt_color, 2);

	display.drawBitmap(display_width / 2 - 63, 190, built_img, 126, 66, txt_color);

	display.setFont(SMALL_FONT);
	display.setTextSize(1);
	if (startup)
	{
		if (g_lorawan_settings.lorawan_enable)
		{
			if (!g_lpwan_has_joined)
			{
				display.getTextBounds((char *)"Wait for connection to LoRaWAN server", 0, 0, &txt_x1, &txt_y1, &txt_w, &txt_h);
				text_rak14000(display_width / 2 - (txt_w / 2), 260, (char *)"Wait for connection to LoRaWAN server", (uint16_t)txt_color, 1);
			}
			else
			{
				// snprintf(disp_text, 59, "Wait %lds for first sensor data readings", (uint32_t)(g_lorawan_settings.send_repeat_time - millis() + g_app_start_time) / 1000);
				snprintf(disp_text, 59, "Wait 30s for first sensor data readings");
				display.getTextBounds(disp_text, 0, 0, &txt_x1, &txt_y1, &txt_w, &txt_h);
				text_rak14000(display_width / 2 - (txt_w / 2), 260, disp_text, (uint16_t)txt_color, 1);
			}
		}
		else
		{
			// snprintf(disp_text, 59, "Wait %lds for first sensor data readings", (uint32_t)(g_lorawan_settings.send_repeat_time - millis() + g_app_start_time) / 1000);
			snprintf(disp_text, 59, "Wait 30s for first sensor data readings");
			display.getTextBounds(disp_text, 0, 0, &txt_x1, &txt_y1, &txt_w, &txt_h);
			text_rak14000(display_width / 2 - (txt_w / 2), 260, disp_text, (uint16_t)txt_color, 1);
		}
	}
	else
	{
		display.getTextBounds((char *)"Thank you!", 0, 0, &txt_x1, &txt_y1, &txt_w, &txt_h);
		text_rak14000(display_width / 2 - (txt_w / 2), 260, (char *)"Thank you!", (uint16_t)txt_color, 1);
	}

	status_lora_rak14000();

	display.display(false);
}

void rak14000_switch_bg(void)
{
	uint16_t old_txt = txt_color;
	txt_color = bg_color;
	bg_color = old_txt;
	api_wake_loop(DISP_UPDATE);
}

/**
 * @brief Wake up RAK14000 from sleep
 *
 */
void startup_rak14000(void)
{
	// Keeping RAK14000 always on
}

/**
 * @brief Put the RAK14000 into sleep mode
 *
 */
void shutdown_rak14000(TimerHandle_t unused)
{
	// Keeping RAK14000 always on
}
