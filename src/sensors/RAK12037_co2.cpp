/**
 * @file RAK12037_co2.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Functions for RAK12037 CO2 gas sensor
 * @version 0.2
 * @date 2024-02-21
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "main.h"
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
	bool found_scd30 = false;
	for (int retry = 0; retry < 10; retry++)
	{
		if (scd30.begin(Wire))
		{
			found_scd30 = true;
			break;
		}
		delay(100);
	}
	if (!found_scd30)
	{
		MYLOG("CO2", "SCD30 not found");
		return false;
	}

	MYLOG("CO2", "SCD30 found");

	//**************init SCD30 sensor *****************************************************
	// Change number of seconds between measurements: 2 to 1800 (30 minutes), stored in non-volatile memory of SCD30
	scd30.setMeasurementInterval(2);

	// Disabled self calibration for now, because
	// "When activated for the first time a
	// period of minimum 7 days is needed so that the algorithm can find its initial parameter set for ASC. The sensor has to be exposed
	// to fresh air for at least 1 hour every day. Also during that period, the sensor may not be disconnected from the power supply,
	// otherwise the procedure to find calibration parameters is aborted and has to be restarted from the beginning. The successfully
	// calculated parameters are stored in non-volatile memory of the SCD30 having the effect that after a restart the previously found
	// parameters for ASC are still present. "

	// Disable self calibration
	scd30.setAutoSelfCalibration(false);

	// Start the measurements
	scd30.beginMeasuring();

	// shutdown_rak12037();

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
	time_t start_time = millis();
	while (!scd30.dataAvailable())
	{
		MYLOG("CO2", "Waiting for data, sensor needs 20 seconds for first value");
		delay(1000);
		if ((millis() - start_time) > 20000)
		{
			// timeout, no data available
			MYLOG("CO2", "Timeout");
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
			MYLOG("CO2", "Waiting again for data, sensor needs 20 seconds for first value");
			delay(1000);
			if ((millis() - start_time) > 20000)
			{
				// timeout, no data available
				MYLOG("CO2", "Timeout 2");
				return;
			}
		}
		co2_reading = scd30.getCO2();
		temp_reading = scd30.getTemperature();
		humid_reading = scd30.getHumidity();
		MYLOG("CO2", "Second reading");
	}
	MYLOG("CO2", "CO2 level %dppm", co2_reading);
	MYLOG("CO2", "Temperature %.2f", temp_reading);
	MYLOG("CO2", "Humidity %.2f", humid_reading);

	g_last_temp = temp_reading;
	g_last_humid = humid_reading;

	g_solution_data.addConcentration(LPP_CHANNEL_CO2_2, co2_reading);

	scd30.StopMeasurement();

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
	if (g_is_using_battery)
	{ // Initialize the sensor
		init_rak12037();
	}
	// Change number of seconds between measurements: 2 to 1800 (30 minutes), stored in non-volatile memory of SCD30
	scd30.setMeasurementInterval(2);

	bool start_success = false;
	time_t start_timeout = millis();
	while ((millis() - start_timeout) < 3000)
	{
		if (scd30.begin(Wire))
		{
			MYLOG("CO2", "RAK12037 started");
			start_success = true;
			break;
		}
		delay(250);
	}

	if (!start_success)
	{
		MYLOG("CO2", "RAK12037 start failed");
		return;
	}

	// Start the measurements
	start_timeout = millis();
	while ((millis() - start_timeout) < 3000)
	{
		if (scd30.beginMeasuring())
		{
			MYLOG("CO2", "RAK12037 measurements started");
			start_success = true;
			return;
		}
		delay(250);
	}
	MYLOG("CO2", "RAK12037 start measurement failed");
}

/**
 * @brief Put the RAK12037 into sleep mode
 *
 */
void shutdown_rak12037(void)
{
		scd30.readMeasurement();
		time_t start_timeout = millis();
		while ((millis() - start_timeout) < 3000)
		{
			if (scd30.StopMeasurement())
			{
				MYLOG("CO2", "RAK12037 stopped");
				return;
			}
			delay(250);
		}
		MYLOG("CO2", "Stop RAK12037 failed");
}

/**
 * @brief Set the calibration concentration manually
 *
 * @param _concentration CO2 concentration has to be within the range 400 ppm ≤ cref(CO2) ≤ 2000 ppm
 * @return true if successful
 * @return false if failed
 */
bool force_calib_rak12037(uint16_t _concentration)
{
	return scd30.setForcedRecalibrationFactor(_concentration);
}

/**
 * @brief Get the current calibration concentration
 *
 * @return uint16_t calibration value 400 ... 2000 ppm or 0 if request failed
 */
uint16_t get_calib_rak12037(void)
{
	uint16_t set_calib = 0;
	if (!scd30.getForcedRecalibration(&set_calib))
	{
		return 0;
	}
	return set_calib;
}

/**
 * @brief Returns the latest values from the sensor
 *        or starts a new reading
 *
 * @param values array for temperature [0] and humidity [1]
 */
void get_rak12037_values(float *values)
{
	values[0] = g_last_temp;
	values[1] = g_last_humid;
	values[2] = g_last_pressure;
	return;
}
