/**
 * @file RAK12047_voc.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Read values from the RAK12047 VOC sensor
 *        The VOC algorithm requires a reading every one second.
 *        This code uses a timer to set the VOC_REQ every one second
 *        and wake up the loop to perform the readings
 * @date 2022-02-05
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "app.h"
#include <SensirionI2CSgp40.h>
#include <VOCGasIndexAlgorithm.h>

/** Sampling interval for the algorithm in seconds */
int32_t sampling_interval = 10;
/** Instance for the VOC sensor */
SensirionI2CSgp40 sgp40;
/** Instance for the VOC algorithm */
VOCGasIndexAlgorithm voc_algorithm(sampling_interval);

/** Timer for VOC measurement */
SoftwareTimer voc_read_timer;

/** Calculated VOC index */
volatile int32_t voc_index = 0;

/** Flag if the VOC index is valid */
bool voc_valid = false;

/** Buffer for debug output */
char errorMessage[256];

/** Counter to discard the first 100 readings */
uint16_t discard_counter = 0;

/** Flag if VOC is active, used from display driver to avoid shut down of power */
volatile bool g_voc_is_active = false;

/**
 * @brief Timer callback to wakeup the loop with the VOC_REQ event
 *
 * @param unused
 */
void voc_read_wakeup(TimerHandle_t unused)
{
	MYLOG("VOC", "VOC triggered");
	api_wake_loop(VOC_REQ);
}

/**
 * @brief Initialize the sensor
 *
 * @return true success
 * @return false failed
 */
bool init_rak12047(void)
{
	sgp40.begin(Wire);

	uint16_t serialNumber[3];
	uint8_t serialNumberSize = 3;

	uint16_t error = sgp40.getSerialNumber(serialNumber, serialNumberSize);

	if (error)
	{
		errorToString(error, errorMessage, 256);
		MYLOG("VOC", "Error trying to execute getSerialNumber() %s", errorMessage);
		return false;
	}
	else
	{
#if MY_DEBUG > 0
		Serial.print("SerialNumber:");
		Serial.print("0x");
		for (size_t i = 0; i < serialNumberSize; i++)
		{
			uint16_t value = serialNumber[i];
			Serial.print(value < 4096 ? "0" : "");
			Serial.print(value < 256 ? "0" : "");
			Serial.print(value < 16 ? "0" : "");
			Serial.print(value, HEX);
		}
		Serial.println();
#endif
	}

	uint16_t testResult;
	error = sgp40.executeSelfTest(testResult);
	if (error)
	{
		errorToString(error, errorMessage, 256);
		MYLOG("VOC", "Error trying to execute executeSelfTest() %s", errorMessage);
		return false;
	}
	else if (testResult != 0xD400)
	{
		MYLOG("VOC", "executeSelfTest failed with error %d", testResult);
		return false;
	}

	int32_t index_offset;
	int32_t learning_time_offset_hours;
	int32_t learning_time_gain_hours;
	int32_t gating_max_duration_minutes;
	int32_t std_initial;
	int32_t gain_factor;
	voc_algorithm.get_tuning_parameters(
		index_offset, learning_time_offset_hours, learning_time_gain_hours,
		gating_max_duration_minutes, std_initial, gain_factor);

	// Reset discard counter
	discard_counter = 0;

	// Set VOC reading interval to 10 seconds
	voc_read_timer.begin(sampling_interval * 1000, voc_read_wakeup, NULL, true);
	voc_read_timer.start();
	return true;
}

/**
 * @brief Read the last VOC index
 *     Data is added to Cayenne LPP payload as channel
 *     LPP_CHANNEL_VOC
 *
 */
void read_rak12047(void)
{
	MYLOG("VOC", "Get VOC");
	if (voc_valid)
	{
		AT_PRINTF("+EVT:GET_VOC\n");
		MYLOG("VOC", "VOC Index: %ld", voc_index);

		g_solution_data.addVoc_index(LPP_CHANNEL_VOC, voc_index);
	}
	else
	{
		AT_PRINTF("+EVT:VOC_NO_DATA\n");
		MYLOG("VOC", "No valid VOC available");
	}
#if HAS_EPD > 0
	set_voc_rak14000((uint16_t)voc_index);
#endif
}

/**
 * @brief Read the current VOC and feed it to the
 *        VOC algorithm
 *        Called every 10 second
 *
 */
void do_read_rak12047(void)
{
	uint16_t error;
	uint16_t srawVoc = 0;
	uint16_t defaultRh = 0x8000;
	uint16_t defaultT = 0x6666;
	float t_h_values[3] = {0.0}; // temperature [0] & humidity [1] value from T&H sensor

	if (has_rak1901)
	{
		get_rak1901_values(t_h_values);
		MYLOG("VOC", "RAK1901 Rh: %.2f T: %.2f", t_h_values[1], t_h_values[0]);

		if ((t_h_values[0] != 0.0) && (t_h_values[1] != 0.0))
		{
			defaultRh = (uint16_t)(t_h_values[1] * 65535 / 100);
			defaultT = (uint16_t)((t_h_values[0] + 45) * 65535 / 175);
		}
	}
	else if (has_rak1906)
	{
		get_rak1906_values(t_h_values);
		MYLOG("VOC", "RAK1906 Rh: %.2f T: %.2f", t_h_values[1], t_h_values[0]);

		if ((t_h_values[0] != 0.0) && (t_h_values[1] != 0.0))
		{
			defaultRh = (uint16_t)(t_h_values[1] * 65535 / 100);
			defaultT = (uint16_t)((t_h_values[0] + 45) * 65535 / 175);
		}
	}

	MYLOG("VOC", "Start reading VOC");
	// 2.a Start heater
	error = sgp40.measureRawSignal(defaultRh, defaultT,
								   srawVoc);
	MYLOG("VOC", "srawVoc: %d", srawVoc);

	// Wait for heater up
	delay(140);

	// 2.b Measure SGP4x signals
	error = sgp40.measureRawSignal(defaultRh, defaultT,
								   srawVoc);
	MYLOG("VOC", "srawVoc: %d", srawVoc);

	// 2.c Stop heater
	sgp40.turnHeaterOff();

	// 3. Process raw signals by Gas Index Algorithm to get the VOC index values
	if (error)
	{
		errorToString(error, errorMessage, 256);
		MYLOG("VOC", "SGP40 - Error trying to execute measureRawSignals(): %s", errorMessage);
	}
	else
	{
		if (discard_counter <= 100)
		{
			// Discard the first 100 readings
			voc_algorithm.process(srawVoc);
			discard_counter++;
			MYLOG("VOC", "Discard reading %d", discard_counter);
		}
		else if (discard_counter == 101)
		{
			// First accepted reading
			voc_index = voc_algorithm.process(srawVoc);
			discard_counter++;
			MYLOG("VOC", "First good reading: %ld", voc_index);
			voc_valid = true;
		}
		else
		{
			uint32_t new_voc_index = voc_algorithm.process(srawVoc);
			voc_index = ((voc_index + new_voc_index) / 2);
			MYLOG("VOC", "VOC: %ld", voc_index);
		}
	}
}
