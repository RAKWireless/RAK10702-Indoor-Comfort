/**
 * @file RAK1903_light.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Initialize and read data from OPT3001 sensor
 * @version 0.3
 * @date 2024-02-21
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "main.h"
#include <ClosedCube_OPT3001.h>

/** Sensor instance */
ClosedCube_OPT3001 opt3001;

/** Sensor I2C address */
#define OPT3001_ADDRESS 0x44

/** Light value */
float g_last_light_lux = 0.0;

/** Configuration register */
OPT3001_Config newConfig;

/**
 * @brief Initialize the Light sensor
 *
 * @return true if sensor found and configuration success
 * @return false if error occured
 */
bool init_rak1903(void)
{
	if (opt3001.begin(OPT3001_ADDRESS) != NO_ERROR)
	{
		MYLOG("LIGHT", "Could not initialize SHTC3");
		return false;
	}

	newConfig.RangeNumber = 0b1100; // B1100;
	newConfig.ConvertionTime = 0b0; // B0;
	newConfig.Latch = 0b1; // B1;
	newConfig.ModeOfConversionOperation = 0b11; // B11;

	OPT3001_ErrorCode errorConfig = opt3001.writeConfig(newConfig);
	if (errorConfig != NO_ERROR)
	{
		MYLOG("LIGHT", "Could not configure OPT3001");
		return false;
	}

	shutdown_rak1903();

	return true;
}

/**
 * @brief Read value from light sensor
 *     Data is added to Cayenne LPP payload as channel
 *     LPP_CHANNEL_LIGHT
 *
 */
void read_rak1903()
{
	MYLOG("LIGHT", "Reading OPT3001");
	OPT3001 result = opt3001.readResult();
	if (result.error == NO_ERROR)
	{
		g_last_light_lux = result.lux;

		MYLOG("LIGHT", "L: %.2f", g_last_light_lux);

		g_solution_data.addLuminosity(LPP_CHANNEL_LIGHT, (uint32_t)(g_last_light_lux));
	}
	else
	{
		MYLOG("LIGHT", "Error reading OPT3001");
		g_solution_data.addLuminosity(LPP_CHANNEL_LIGHT, 0);
	}
}

/**
 * @brief Wake up RAK1903 from sleep
 *
 */
void startup_rak1903(void)
{
	newConfig.RangeNumber = 0b1100;				// B1100;
	newConfig.ConvertionTime = 0b0;				// B0;
	newConfig.Latch = 0b1;						// B1;
	newConfig.ModeOfConversionOperation = 0b11; // B11;

	OPT3001_ErrorCode errorConfig = opt3001.writeConfig(newConfig);
	if (errorConfig != NO_ERROR)
	{
		MYLOG("LIGHT", "Could not configure OPT3001");
	}
}

/**
 * @brief Put the RAK1903 into sleep mode
 *
 */
void shutdown_rak1903(void)
{
	newConfig.RangeNumber = 0b1100;				// B1100;
	newConfig.ConvertionTime = 0b0;				// B0;
	newConfig.Latch = 0b1;						// B1;
	newConfig.ModeOfConversionOperation = 0b00; // B00;

	OPT3001_ErrorCode errorConfig = opt3001.writeConfig(newConfig);
	if (errorConfig != NO_ERROR)
	{
		MYLOG("LIGHT", "Could not configure OPT3001");
	}
}