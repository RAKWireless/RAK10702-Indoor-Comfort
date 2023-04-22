/**
 * @file RAK12039_pm.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief PMSA003I particle matter sensor support
 * @version 0.1
 * @date 2022-07-28
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "app.h"
#include <RAK12039_PMSA003I.h>

/** Instance of sensor class */
RAK_PMSA003I PMSA003I;

/** Data structure */
PMSA_Data_t data;

/**
 * @brief Initialize the PMSA003I sensor
 *
 * @return true if sensor was found
 * @return false if sensor was not found
 */
bool init_rak12039(void)
{
	// On/Off control pin
	pinMode(SET_PIN, OUTPUT);

	// Sensor on
	digitalWrite(SET_PIN, HIGH);

	// Wait for sensor wake-up
	time_t wait_sensor = millis();
	MYLOG("PMS", "RAK12039 scan start %ld ms", millis());
	byte error;
	while (1)
	{
		delay(500);
		Wire.beginTransmission(0x12);
		error = Wire.endTransmission();
		if (error == 0)
		{
			MYLOG("PMS", "RAK12039 answered at %ld ms", millis());
			break;
		}
		if ((millis() - wait_sensor) > 10000)
		{
			MYLOG("PMS", "RAK12039 timeout after 10000 ms");
			return false;
		}
	}

	if (!PMSA003I.begin())
	{
		MYLOG("PMS", "PMSA003I begin fail,please check connection!");
		digitalWrite(SET_PIN, LOW);
		return false;
	}

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

		g_solution_data.addVoc_index(LPP_CHANNEL_PM_1_0, data.pm10_env);
		g_solution_data.addVoc_index(LPP_CHANNEL_PM_2_5, data.pm25_env);
		g_solution_data.addVoc_index(LPP_CHANNEL_PM_10_0, data.pm100_env);

		MYLOG("PMS", "Std PM ug/m3: PM 1.0 %d PM 2.5 %d PM 10 %d", data.pm10_standard, data.pm25_standard, data.pm100_standard);
		MYLOG("PMS", "Env PM ug/m3: PM 1.0 %d PM 2.5 %d PM 10 %d", data.pm10_env, data.pm25_env, data.pm100_env);
#if HAS_EPD == 1 || HAS_EPD == 4
		set_pm_rak14000(data.pm10_env, data.pm25_env, data.pm100_env);
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
#if SENSOR_POWER_OFF > 0
	// Sensor on
	digitalWrite(CO2_PM_POWER, HIGH); // power on RAK12039
#else
	digitalWrite(SET_PIN, HIGH);
#endif

	// Wait for wakeup
	time_t wait_sensor = millis();
	MYLOG("PMS", "RAK12039 wake-up scan start %ld ms", millis());
	byte error;
	while (1)
	{
		delay(500);
		Wire.beginTransmission(0x12);
		error = Wire.endTransmission();
		if (error == 0)
		{
			MYLOG("PMS", "RAK12039 answered at %ld ms", millis());
			break;
		}
		if ((millis() - wait_sensor) > 10000)
		{
			MYLOG("PMS", "RAK12039 timeout after 10000 ms");
			break;
		}
	}
#if SENSOR_POWER_OFF > 0
	// Init Sensor
	init_rak12039();
#endif
}

/**
 * @brief Put the RAK12037 into sleep mode
 *
 */
void shut_down_rak12039(void)
{
#if SENSOR_POWER_OFF > 0
	// Disable power
	digitalWrite(CO2_PM_POWER, LOW); // power off RAK12039
#else
	digitalWrite(SET_PIN, LOW); // Sensor off
	MYLOG("PMS", "RAK12039 fan off");
#endif
}