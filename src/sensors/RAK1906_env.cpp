/**
 * @file RAK1906_env.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief BME680 sensor functions
 * @version 0.2
 * @date 2024-02-21
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "main.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>

/** BME680 instance for Wire */
Adafruit_BME680 bme(&Wire);

// Might need adjustments
#define SEALEVELPRESSURE_HPA (1010.0)

/** Last pressure read */
float g_last_pressure = 0;

/**
 * @brief Initialize the BME680 sensor
 *
 * @return true if sensor was found
 * @return false if sensor was not found
 */
bool init_rak1906(void)
{
	if (!bme.begin(0x76))
	{
		MYLOG("BME", "Could not find a valid BME680 sensor, check wiring!");
		return false;
	}

	// Set up oversampling and filter initialization
	bme.setTemperatureOversampling(BME680_OS_8X);
	bme.setHumidityOversampling(BME680_OS_2X);
	bme.setPressureOversampling(BME680_OS_4X);
	bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
	// As we do not use the BSEC library here, the gas value is useless and just consumes battery. Better to switch it off
	// bme.setGasHeater(320, 150); // 320*C for 150 ms
	bme.setGasHeater(0, 0); // switch off
	MYLOG("BME", "Setup BME680 sensor finished");

	shutdown_rak1906();

	return true;
}

/**
 * @brief Read environment data from BME680
 *     Data is added to Cayenne LPP payload as channels
 *     LPP_CHANNEL_HUMID_2, LPP_CHANNEL_TEMP_2,
 *     LPP_CHANNEL_PRESS_2 and LPP_CHANNEL_GAS_2
 *
 *
 * @return true if reading was successful
 * @return false if reading failed
 */
bool read_rak1906()
{
	time_t wait_start = millis();
	bool read_success = false;
	while ((millis() - wait_start) < 5000)
	{
		if (bme.endReading())
		{
			read_success = true;
			break;
		}
		delay(100);
	}

	if (!read_success)
	{
		MYLOG("BME", "BME680 read failed");
		return false;
	}

	g_solution_data.addRelativeHumidity(LPP_CHANNEL_HUMID_2, (float)bme.humidity);
	g_solution_data.addTemperature(LPP_CHANNEL_TEMP_2, (float)bme.temperature);
	g_solution_data.addBarometricPressure(LPP_CHANNEL_PRESS_2, (float)(bme.pressure) / 100.0);
	// g_solution_data.addAnalogInput(LPP_CHANNEL_GAS_2, (float)(bme.gas_resistance) / 1000.0);

	g_last_temp = bme.temperature;
	g_last_humid = bme.humidity;
	g_last_pressure = (float)(bme.pressure) / 100.0;

#if MY_DEBUG > 0
	MYLOG("BME", "RH= %.2f T= %.2f", bme.humidity, bme.temperature);
	MYLOG("BME", "P= %.3f R= %.2f", (float)(bme.pressure) / 100.0, (float)(bme.gas_resistance) / 1000.0);
#endif

#if HAS_EPD > 0
	set_humid_rak14000(bme.humidity);
	set_temp_rak14000(bme.temperature);
	set_baro_rak14000(bme.pressure / 100.0);
#endif

	return true;
}

/**
 * @brief Returns the latest values from the sensor
 *        or starts a new reading
 *
 * @param values array for temperature [0], humidity [1] and pressure [2]
 */
void get_rak1906_values(float *values)
{
	values[0] = g_last_temp;
	values[1] = g_last_humid;
	values[2] = g_last_pressure;
}

/**
 * @brief Wake up RAK1906 from sleep
 *
 */
void startup_rak1906(void)
{
	MYLOG("BME", "Start BME reading");
	bme.beginReading();
}

/**
 * @brief Put the RAK1906 into sleep mode
 *
 */
void shutdown_rak1906(void)
{
	// No low power functionality found
}