/**
 * @file button.cpp
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Button initializer and handler
 * @version 0.1
 * @date 2023-03-23
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "app.h"
#include "OneButton.h"

/** Timer for VOC measurement */
SoftwareTimer button_check;

// Flag if button timer is already running
bool timer_running = false;

extern SoftwareTimer voc_read_timer;

/**
 * @brief Button instance
 * 		First parameter is interrupt input for button
 * 		Second parameter defines button pushed states:
 * 			true if active low (LOW if pressed)
 * 			false if active high (HIGH if pressed)
 */
OneButton button(BUTTON_INT, true);

/** Time when button was pressed, used to determine length of long press */
unsigned long pressStartTime;

/**
 * @brief Button interrupt callback
 * 		calls button.tick() to process status
 * 		start a timer to frequently repeat button status process until event is handled
 *
 */
void checkTicks(void)
{
	// Button interrupt, call tick()
	button.tick();
	// If not already running, start the timer to frequently check the button status
	if (!timer_running)
	{
		timer_running = true;
		button_check.start();
	}
}

/**
 * @brief Callback if single button push was detected
 * 		Used to switch between different display UI's
 * 		At the moment just enables RGB LED
 */
void singleClick(void)
{
	button_check.stop();
	timer_running = false;
	MYLOG("BTN", "singleClick() detected.");

	// Request an UI change
	switch_ui();
}

/**
 * @brief Callback for double button push was detected
 * 		Used to switch display from white to black mode and back
 */
void doubleClick(void)
{
	button_check.stop();
	timer_running = false;
	MYLOG("BTN", "doubleClick() detected.");
	rak14000_switch_bg();
}

/**
 * @brief Callback for multi push button events (> 3 push)
 * 		Used for different functionalities
 *      - 9 times ==> reset device
 *
 */
void multiClick()
{
	button_check.stop();
	timer_running = false;
	uint8_t tick_num = button.getNumberClicks();
	switch (tick_num)
	{
		// Enable BLE
	case 3:
		// If BLE is enabled, restart Advertising
		if (g_enable_ble)
		{
			MYLOG("BTN", "BLE On.");
			restart_advertising(15);
		}
		break;
	// Show Device Status Screen
	case 4:
		g_ui_selected = 2;
		api_wake_loop(DISP_UPDATE);
		break;
	// Only for testing, shut down everything
	case 5:
		// Stop everything
		MYLOG("BTN", "Stopping everything.");
		api_timer_stop();
		digitalWrite(EPD_POWER, LOW);
		digitalWrite(PIR_POWER, LOW);
		digitalWrite(CO2_PM_POWER, LOW);
		voc_read_timer.stop();
		digitalWrite(VOC_POWER, LOW);
		break;
		// Reset the device
	case 9:
			MYLOG("BTN", "RST request");
			api_wake_loop(RST_REQ);
			break;
	default:
		MYLOG("BTN", "multiClick(%d) detected.", button.getNumberClicks());
		break;
	}
}

// /**
//  * @brief Callback when a button is pushed, records the start time of a long press
//  *
//  */
// void pressStart(void)
// {
// 	MYLOG("BTN", "pressStart()");
// 	pressStartTime = millis() - 1000; // as set in setPressTicks()
// }

// /**
//  * @brief Callback when a long-press event has finished
//  * 		Unused at the moment
//  *
//  */
// void pressStop(void)
// {
// 	button_check.stop();
// 	timer_running = false;
// 	MYLOG("BTN", "pressStop(%ld) detected.", (millis() - pressStartTime));
// }

/**
 * @brief Timer callback after a button push event was detected.
 * 		Needed to continue to check the button status
 *
 * @param unused
 */
void check_button(TimerHandle_t unused)
{
	button.tick();
}

/**
 * @brief Initialize Button functions
 *
 */
void init_button(void)
{
	// Setup interrupt routine
	attachInterrupt(digitalPinToInterrupt(BUTTON_INT), checkTicks, CHANGE);

	// Setup the different callbacks for button events
	button.attachClick(singleClick);
	button.attachDoubleClick(doubleClick);
	button.attachMultiClick(multiClick);

	// // Setup long press button events
	// button.setPressTicks(3000); // that is the time when LongPressStart is called
	// button.attachLongPressStart(pressStart);
	// button.attachLongPressStop(pressStop);

	// Create timer for button handling
	button_check.begin(10, check_button, NULL, true);
}
