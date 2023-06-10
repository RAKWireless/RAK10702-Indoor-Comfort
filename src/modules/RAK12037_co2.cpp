/**
 * @file RAK12037_co2.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Functions for RAK12037 CO2 gas sensor
 * @version 0.1
 * @date 2022-04-01
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "app.h"
#include <SparkFun_SCD30_Arduino_Library.h> //Click here to get the library: http://librarymanager/All#SparkFun_SCD30

/** Sensor instance */
SCD30 scd30;

/**
 * @brief Initialize MQ2 gas sensor
 *
 * @return true success
 * @return false failed
 */
bool init_rak12037(void)
{
	if (!scd30.begin(Wire))
	{
		MYLOG("SCD30", "SCD30 not found");
		return false;
	}

	//**************init SCD30 sensor *****************************************************
	// Change number of seconds between measurements: 2 to 1800 (30 minutes), stored in non-volatile memory of SCD30
	scd30.setMeasurementInterval(2);
	// Enable self calibration
	scd30.setAutoSelfCalibration(true);

	// Start the measurements
	scd30.beginMeasuring();

	return true;
}

/**
 * @brief Read CO2 sensor data
 *     Data is added to Cayenne LPP payload as channels
 *     LPP_CHANNEL_CO2_2, LPP_CHANNEL_CO2_Temp_2 and LPP_CHANNEL_CO2_HUMID_2
 *
 */
void read_rak12037(void)
{
	// Change number of seconds between measurements: 2 to 1800 (30 minutes), stored in non-volatile memory of SCD30
	scd30.setMeasurementInterval(2);
	// Enable self calibration
	scd30.setAutoSelfCalibration(true);

	// Start the measurements
	scd30.beginMeasuring();

	time_t start_time = millis();
	while (!scd30.dataAvailable())
	{
		MYLOG("SCD30", "Waiting for data");
		delay(500);
		if ((millis() - start_time) > 10000)
		{
			// timeout, no data available
			MYLOG("SCD30", "Timeout");
			return;
		}
	}

	uint16_t co2_reading = scd30.getCO2();
	float temp_reading = scd30.getTemperature();
	float humid_reading = scd30.getHumidity();

	if (co2_reading == 0)
	{
		start_time = millis();
		while (!scd30.dataAvailable())
		{
			MYLOG("SCD30", "Waiting again for data");
			delay(500);
			if ((millis() - start_time) > 10000)
			{
				// timeout, no data available
				MYLOG("SCD30", "Timeout 2");
				return;
			}
		}
		co2_reading = scd30.getCO2();
		temp_reading = scd30.getTemperature();
		humid_reading = scd30.getHumidity();
		MYLOG("SCD30", "Second reading");
	}
	MYLOG("SCD30", "CO2 level %dppm", co2_reading);
	MYLOG("SCD30", "Temperature %.2f", temp_reading);
	MYLOG("SCD30", "Humidity %.2f", humid_reading);

	g_solution_data.addConcentration(LPP_CHANNEL_CO2_2, co2_reading);

#if HAS_EPD > 0
	set_co2_rak14000(co2_reading);
#endif
}

/**
 * @brief Wake up RAK12037 from sleep
 *
 */
void startup_rak12037(void)
{
#if SENSOR_POWER_OFF > 0
	// Power up the sensor
	digitalWrite(CO2_PM_POWER, HIGH); // power off RAK12037
	init_rak12037();
#else
	// Change number of seconds between measurements: 2 to 1800 (30 minutes), stored in non-volatile memory of SCD30
	scd30.setMeasurementInterval(2);

	// Enable self calibration
	scd30.setAutoSelfCalibration(true);

	// Start the measurements
	scd30.beginMeasuring();
#endif
}

/**
 * @brief Put the RAK12037 into sleep mode
 *
 */
void shut_down_rak12037(void)
{
#if SENSOR_POWER_OFF > 0
	// Disable power
	digitalWrite(CO2_PM_POWER, LOW); // power off RAK12037
#else
	scd30.StopMeasurement();
#endif
}