/**
 * @file pir.cpp
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief PIR sensor init and handler
 * @version 0.2
 * @date 2024-02-21
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "main.h"

/** Flag if occupancy was detected */
bool g_occupied = true;

/** Timer for VOC measurement */
SoftwareTimer occupation_timer;

/**
 * @brief Interrupt callback for PIR sensor
 *
 */
void pir_int(void)
{
	// Restart the occupation timer
	occupation_timer.stop();
	occupation_timer.start();
	// Was the room empty?
	if (!g_occupied)
	{
		// If room was empty, initiate a refresh
		api_wake_loop(MOTION);
	}
	g_occupied = true;
}

/**
 * @brief Timer callback if room is unoccupied for a long time
 * 		Time is set with the timer start, default is 10 minutes
 *
 * @param unused
 */
void occupation_timeout(TimerHandle_t unused)
{
	g_occupied = false;
	// Wake loop to apply new settings
	api_wake_loop(ROOM_EMPTY);
}

/**
 * @brief Initialize the PIR sensor
 *
 */
void init_pir(void)
{
	// PIR POWER
	pinMode(PIR_POWER, OUTPUT);
	digitalWrite(PIR_POWER, HIGH);

	pinMode(PIR_INT, INPUT);

	// PIR INTERRPUT
	attachInterrupt(PIR_INT, pir_int, RISING);

	// Start timer for occupation detection (10 minutes)
	occupation_timer.begin(10 * 60 * 1000, occupation_timeout, NULL, false);
	occupation_timer.start();
}
