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

bool timer_running = false;

// Setup a new OneButton on pin PIN_INPUT
// The 2. parameter activeLOW is true, because external wiring sets the button to LOW when pressed.
OneButton button(BUTTON_INT, true);

// save the millis when a press has started.
unsigned long pressStartTime;

// In case the momentary button puts the input to HIGH when pressed:
// The 2. parameter activeLOW is false when the external wiring sets the button to HIGH when pressed.
// The 3. parameter can be used to disable the PullUp .
// OneButton button(PIN_INPUT, false, false);

// This function is called from the interrupt when the signal on the PIN_INPUT has changed.

void checkTicks()
{
	// include all buttons here to be checked
	button.tick(); // just call tick() to check the state.
	if (!timer_running)
	{
		timer_running = true;
		button_check.start();
	}
}

// this function will be called when the button was pressed 1 time only.
void singleClick()
{
	button_check.stop();
	timer_running = false;
	Serial.println("singleClick() detected.");
} // singleClick

// this function will be called when the button was pressed 2 times in a short timeframe.
void doubleClick()
{
	button_check.stop();
	timer_running = false;
	Serial.println("doubleClick() detected.");
	rak14000_switch_bg();
} // doubleClick

// this function will be called when the button was pressed multiple times in a short timeframe.
void multiClick()
{
	button_check.stop();
	timer_running = false;
	uint8_t tick_num = button.getNumberClicks();
	switch (tick_num)
	{
		case 2: // Switch EPD colors
		rak14000_switch_bg();
		break;
		case 9:
		rak14000_start_screen();
		delay(3000);
		api_reset();
		break;
		default:
		Serial.print("multiClick(");
		Serial.print(button.getNumberClicks());
		Serial.println(") detected.");
		break;
	}
} // multiClick

// this function will be called when the button was held down for 1 second or more.
void pressStart()
{
	Serial.println("pressStart()");
	pressStartTime = millis() - 1000; // as set in setPressTicks()
} // pressStart()

// this function will be called when the button was released after a long hold.
void pressStop()
{
	button_check.stop();
	timer_running = false;
	Serial.print("pressStop(");
	Serial.print(millis() - pressStartTime);
	Serial.println(") detected.");
} // pressStop()

void check_button(TimerHandle_t unused)
{
	button.tick();
}

void init_button(void)
{
	// setup interrupt routine
	// when not registering to the interrupt the sketch also works when the tick is called frequently.
	attachInterrupt(digitalPinToInterrupt(BUTTON_INT), checkTicks, CHANGE);

	// button.setDebounceTicks(25);
	// button.setClickTicks(100);

	// link the xxxclick functions to be called on xxxclick event.
	button.attachClick(singleClick);
	button.attachDoubleClick(doubleClick);
	button.attachMultiClick(multiClick);

	button.setPressTicks(3000); // that is the time when LongPressStart is called
	button.attachLongPressStart(pressStart);
	button.attachLongPressStop(pressStop);

	// Create timer for button handling
	button_check.begin(10, check_button, NULL, true);
	// button_check.start();
}
