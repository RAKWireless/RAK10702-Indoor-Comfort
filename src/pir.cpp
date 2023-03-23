/**
 * @file pir.cpp
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief PIR sensor init and handler
 * @version 0.1
 * @date 2023-03-23
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "app.h"

bool g_occupied = true;

/** Timer for VOC measurement */
SoftwareTimer occupation_timer;

void pir_int(void)
{
	MYLOG("PIR", "Triggered");
	Serial.flush();
	// Restart the occupation timer
	occupation_timer.stop();
	occupation_timer.start();
	// Was the room empty?
	if (!g_occupied)
	{
		// If room was empty, initiate a refresh
		api_wake_loop(MOTION_TRIGGER);
	}
	g_occupied = true;
}

void occupation_timeout(TimerHandle_t unused)
{
	g_occupied = false;
	// Wake loop to apply new settings
	api_wake_loop(ROOM_EMPTY);
}

void init_pir(void)
{
	// PIR POWER
	pinMode(PIR_POWER, OUTPUT);
	digitalWrite(PIR_POWER, HIGH);

	// PIR INTERRPUT
	pinMode(PIR_INT, INPUT);
	attachInterrupt(PIR_INT, pir_int, RISING);

	// Start timer for occupation detection (10 minutes)
	occupation_timer.begin(10 * 60 * 1000, occupation_timeout, NULL, false);
	// occupation_timer.begin(30 * 1000, occupation_timeout, NULL, false);
	occupation_timer.start();
}
