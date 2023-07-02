/**
 * @file modules.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Find and handle WisBlock sensor modules
 * @version 0.1
 * @date 2022-02-02
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "app.h"
#include "modules.h"

/**
 * @brief List of all supported WisBlock modules
 *
 */
bool has_rak1901 = false;
bool has_rak1902 = false;
bool has_rak1903 = false;
bool has_rak1906 = false;
bool has_rak12002 = false;
bool has_rak12010 = false;
bool has_rak12019 = false;
bool has_rak12037 = false;
bool has_rak12039 = false;
bool has_rak12047 = false;

/** Flag if sensors are powered down */
bool g_sensors_off = false;

/**
 * @brief Scan both I2C bus for devices
 *
 */
void find_modules(void)
{
	// RAK12039 has extra GPIO for power control
	// On/Off control pin
	pinMode(WB_IO6, OUTPUT);
	// Sensor on
	digitalWrite(WB_IO6, HIGH);
	delay(500);

	Wire.begin();

	// Initialize RGB LED
	init_rgb();
	set_rgb_color(246, 190, 0); //

#if _CUSTOM_BOARD_ == 1
	// Init PIR (only RAK19024)
	init_pir();
#endif

	// BUTTON
	init_button();

	if (!init_rak1901())
	{
		has_rak1901 = false;
	}
	else
	{
		has_rak1901 = true;
	}

	if (!init_rak1902())
	{
		has_rak1902 = false;
	}
	else
	{
		has_rak1902 = true;
	}

	if (!init_rak1903())
	{
		has_rak1903 = false;
	}
	else
	{
		has_rak1903 = true;
	}

	if (!init_rak1906()) // !!! USING SIMPLE READINGS
						 // #endif
	{
		has_rak1906 = false;
	}
	else
	{
		has_rak1906 = true;
	}

	// Wire.beginTransmission(0x52);
	// byte error = Wire.endTransmission();
	// if (error == 0)
	// {
	// 	if (!init_rak12002())
	// 	{
	// 		has_rak12002 = false;
	// 	}
	// 	else
	// 	{
	// 		has_rak12002 = true;
	// 	}
	// }
	// else
	// {
	// 	has_rak12002 = false;
	// }

	if (!init_rak12010())
	{
		has_rak12010 = false;
	}
	else
	{
		has_rak12010 = true;
	}

	if (!init_rak12019())
	{
		has_rak12019 = false;
	}
	else
	{
		has_rak12019 = true;
	}

	if (!init_rak12037())
	{
		has_rak12037 = false;
	}
	else
	{
		has_rak12037 = true;
	}

	if (!init_rak12039())
	{
		has_rak12039 = false;
	}
	else
	{
		has_rak12039 = true;
	}

	if (!init_rak12047())
	{
		has_rak12047 = false;
	}
	else
	{
		has_rak12047 = true;
	}
}

/**
 * @brief AT command feedback about found modules
 *
 */
void announce_modules(void)
{
	if (has_rak1901)
	{
		AT_PRINTF("+EVT:RAK1901 OK\n");
		read_rak1901();
	}

	if (has_rak1902)
	{
		AT_PRINTF("+EVT:RAK1902 OK\n");
		read_rak1902();
	}

	if (has_rak1903)
	{
		AT_PRINTF("+EVT:RAK1903 OK\n");
		read_rak1903();
	}

	if (has_rak1906)
	{
		AT_PRINTF("+EVT:RAK1906 OK\n");
		read_rak1906();
	}

	if (has_rak12002)
	{
		AT_PRINTF("+EVT:RAK12002 OK\n");
		read_rak12002();
	}

	if (has_rak12010)
	{
		AT_PRINTF("+EVT:RAK12010 OK\n");
		read_rak12010();
	}

	if (has_rak12019)
	{
		AT_PRINTF("+EVT:RAK12019 OK\n");
		read_rak12019();
	}

	if (has_rak12037)
	{
		AT_PRINTF("+EVT:RAK12037 OK\n");
		read_rak12037();
	}

	if (has_rak12039)
	{
		AT_PRINTF("+EVT:RAK12039 OK\n");
		read_rak12039();
	}

	if (has_rak12047)
	{
		AT_PRINTF("+EVT:RAK12047 OK\n");
	}
}

/**
 * @brief Read values from the found modules
 *
 */
void get_sensor_values(void)
{
	if (has_rak1901)
	{
		// Read environment data
		read_rak1901();
	}

	if (has_rak1902)
	{
		// Read barometer data
		read_rak1902();
	}

	if (has_rak1903)
	{
		// Read environment data
		read_rak1903();
	}

	if (has_rak1906)
	{
		// Get environment data
		read_rak1906();
	}

	if (has_rak12037)
	{
		// Get the CO2 sensor values
		read_rak12037();
	}

	if (has_rak12010)
	{
		// Read environment data
		read_rak12010();
	}

	if (has_rak12019)
	{
		// Get the LTR390 sensor values
		read_rak12019();
	}

	if (has_rak12039)
	{
		// Get the particle matter sensor values
		read_rak12039();
	}

	if (has_rak12047)
	{
		// Get the voc sensor values
		read_rak12047();
	}
}

/**
 * @brief Shut down or power up I2C pull-ups and EPD
 *
 * @param switch_on if true power up, else power down I2C pull-ups and EPD
 */
void power_i2c(bool switch_on)
{
	if (switch_on)
	{
		digitalWrite(EPD_POWER, HIGH);
		delay(100);
		// Wire.begin();
	}
	else
	{
		digitalWrite(EPD_POWER, LOW);
		// Wire.end();
		// pinMode(WB_I2C1_SDA, OUTPUT_D0S1);
		// pinMode(WB_I2C1_SCL, OUTPUT_D0S1);
		// digitalWrite(WB_I2C1_SDA, LOW);
		// digitalWrite(WB_I2C1_SCL, LOW);
	}
}

/**
 * @brief Shut down or switch on modules
 *
 * @param switch_on if true, wake up modules, else power down modules
 */
void power_modules(bool switch_on)
{
	if (!switch_on)
	{
		g_sensors_off = true;
	}
	else
	{
		g_epd_off = false;
	}

	if (has_rak1901)
	{
		if (switch_on)
		{
			start_up_rak1901();
		}
		else
		{
			shut_down_rak1901();
		}
	}

	if (has_rak1902)
	{
		if (switch_on)
		{
			startup_rak1902();
		}
		else
		{
			shut_down_rak1902();
		}
	}

	if (has_rak1903)
	{
		if (switch_on)
		{
			startup_rak1903();
		}
		else
		{
			shut_down_rak1903();
		}
	}

	if (has_rak12010)
	{
		if (switch_on)
		{
			startup_rak12010();
		}
		else
		{
			shut_down_rak12010();
		}
	}

	if (has_rak12019)
	{
		if (switch_on)
		{
			startup_rak12019();
		}
		else
		{
			shut_down_rak12019();
		}
	}

	if (has_rak12037 || has_rak12039)
	{
		if (has_rak12037)
		{
			if (switch_on)
			{
				startup_rak12037();
			}
			else
			{
				shut_down_rak12037();
			}
		}

		if (has_rak12039)
		{
			if (switch_on)
			{
				startup_rak12039();
			}
			else
			{
				shut_down_rak12039();
			}
		}
	}

	if (switch_on)
	{
		g_sensors_off = false;
	}
}
