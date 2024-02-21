/**
 * @file RAK12039_pm.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief PMSA003I particle matter sensor support
 * @version 0.2
 * @date 2024-02-21
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "main.h"
#include <RAK12039_PMSA003I.h>

/** Instance of sensor class */
RAK_PMSA003I PMSA003I;

/** Data structure */
PMSA_Data_t data;

// Forward declarations
uint16_t normalize_std_val(uint16_t org_val);
uint16_t normalize_env_val(uint16_t org_val);

/**
 * @brief Initialize the PMSA003I sensor
 *
 * @return true if sensor was found
 * @return false if sensor was not found
 */
bool init_rak12039(void)
{
	// Wait for sensor wake-up
	time_t wait_sensor = millis();
	MYLOG("PMS", "RAK12039 scan start %ld ms", millis());
	byte error;
	bool waiting = true;
	while (waiting)
	{
		delay(500);
		Wire.beginTransmission(0x12);
		error = Wire.endTransmission();
		if (error == 0)
		{
			MYLOG("PMS", "RAK12039 answered at %ld ms", millis());
			waiting = false;
		}
		if ((millis() - wait_sensor) > 10000)
		{
			MYLOG("PMS", "RAK12039 timeout after 10000 ms");
			return false;
		}
	}

	MYLOG("PMS", "Initialize PMSA003I");

	if (!PMSA003I.begin())
	{
		MYLOG("PMS", "PMSA003I begin fail,please check connection!");
		digitalWrite(SET_PIN, LOW);
		return false;
	}
	else
	{
		MYLOG("PMS", "PMSA003I begin finished");
	}

	shutdown_rak12039();

	return true;
}

/**
 * @brief Read PM data from RAK12039
 *     Data is added to Cayenne LPP payload as channels
 *     LPP_CHANNEL_PM_1_0, LPP_CHANNEL_PM_2_5, LPP_CHANNEL_PM_10_0
 *
 */
void read_rak12039(void)
{
	if (PMSA003I.readDate(&data))
	{

		MYLOG("PMS", "PMSA003I read date success.");

		g_solution_data.addVoc_index(LPP_CHANNEL_PM_1_0, normalize_env_val(data.pm10_env));
		g_solution_data.addVoc_index(LPP_CHANNEL_PM_2_5, normalize_env_val(data.pm25_env));
		g_solution_data.addVoc_index(LPP_CHANNEL_PM_10_0, normalize_env_val(data.pm100_env));

		MYLOG("PMS", "Std PM ug/m3: PM 1.0 %d PM 2.5 %d PM 10 %d", normalize_std_val(data.pm10_standard), normalize_std_val(data.pm25_standard), normalize_std_val(data.pm100_standard));
		MYLOG("PMS", "Env PM ug/m3: PM 1.0 %d PM 2.5 %d PM 10 %d", normalize_env_val(data.pm10_env), normalize_env_val(data.pm25_env), normalize_env_val(data.pm100_env));
#if HAS_EPD == 1 || HAS_EPD == 4
		set_pm_rak14000(normalize_env_val(data.pm10_env), normalize_env_val(data.pm25_env), normalize_env_val(data.pm100_env));
#endif
	}
	else
	{
		Serial.println("PMSA003I read failed!");
	}

	return;
}

/**
 * @brief Wake up RAK12039 from sleep
 *
 */
void startup_rak12039(void)
{
	digitalWrite(SET_PIN, HIGH);
	delay(1000);

	if (g_is_using_battery)
	{
		// Init Sensor
		if (!init_rak12039())
		{
			MYLOG("PM", "Failed to restart PM sensor");
			has_rak12039 = false;
			return;
		}
	}
}

/**
 * @brief Put the RAK12037 into sleep mode
 *
 */
void shutdown_rak12039(void)
{
	digitalWrite(SET_PIN, LOW);
}

/**
 * @brief Normalize standard sensor value with formula given by the manufacturer
 *
 * @param org_val value from sensor
 * @return uint16_t Normalized value
 */
uint16_t normalize_std_val(uint16_t org_val)
{
	float _org_val = org_val * 1.0;

	// org_val less than 1
	if (org_val <= 1)
	{
		return 1;
	}

	// org_val between 1 and 896
	if ((org_val > 1) && org_val <= 896)
	{
		return (uint16_t)((1 / (1.585325 / _org_val - 0.0009357)) * 1.08);
	}

	// org_val > 896
	return 304 + org_val;
}

/**
 * @brief Normalize environment sensor value with formula given by the manufacturer
 *
 * @param org_val value from sensor
 * @return uint16_t Normalized value
 */
uint16_t normalize_env_val(uint16_t org_val)
{
	float _org_val = org_val * 1.0;

	// org_val less than 1
	if (org_val <= 1)
	{
		return 1;
	}

	// org_val between 1 and 583
	if ((org_val > 1) && org_val <= 583)
	{
		return (uint16_t)((1 / (1.061263 / _org_val - 0.0009423)) * 1.08);
	}

	// org_val > 583
	return 618 + org_val;
}