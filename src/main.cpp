/**
 * @file main.cpp
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Low power test
 * @version 0.2
 * @date 2024-02-21
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "main.h"

/** Send Fail counter **/
uint8_t send_fail = 0;

/** Set the device name, max length is 10 characters */
char g_ble_dev_name[10] = "RAK-LP";

/** LoRaWAN packet */
WisCayenne g_solution_data(255);

/** Timer for running the sensors before reading them */
SoftwareTimer g_sensor_timer;

/** Start time of application */
time_t g_app_start_time;

/** No screen update flag for join success */
bool second_screen = false;

/** Flag if the device is battery or permanent powered */
bool g_is_using_battery = false;

/**
 * @brief List of all supported WisBlock modules
 *
 */
bool has_rak1901 = false;
bool has_rak1902 = false;
bool has_rak1903 = false;
bool has_rak1906 = false;
bool has_rak12002 = false;
bool has_rak12010 = false;
bool has_rak12019 = false;
bool has_rak12037 = false;
bool has_rak12039 = false;
bool has_rak12047 = false;
bool has_rgb = false;

/** Counter for connection check with confirmed packet */
uint8_t check_connection = 0;

/**
 * @brief Initial setup of the application (before LoRaWAN and BLE setup)
 *
 */
void setup_app(void)
{
	// Initialize Serial for debug output
	Serial.begin(115200);

	delay(500);

	nrfx_power_usb_state_t usb_status = nrfx_power_usbstatus_get();

	if (usb_status != NRFX_POWER_USB_STATE_DISCONNECTED) // USB power detected
	{
		time_t serial_timeout = millis();
		// On nRF52840 the USB serial is not available immediately
		while (!Serial)
		{
			if ((millis() - serial_timeout) < 5000)
			{
				delay(100);
#if _CUSTOM_BOARD_ == 0
				digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
#endif
			}
			else
			{
#if _CUSTOM_BOARD_ == 0
				digitalWrite(LED_BUILTIN, LOW);
#endif
				break;
			}
		}
	}
#if _CUSTOM_BOARD_ == 0
	digitalWrite(LED_BUILTIN, LOW);
#endif

	g_enable_ble = true;
}

/**
 * @brief Final setup of application  (after LoRaWAN and BLE setup)
 *
 * @return true
 * @return false
 */
bool init_app(void)
{
	// Start time of application, used for display
	g_app_start_time = millis() / 1000;

	api_set_version(SW_VERSION_1, SW_VERSION_2, SW_VERSION_3);
	// g_device_pid = "RAK10702";
	// g_custom_fw_ver = "RAK10702 V" + String(SW_VERSION_1) + "." + String(SW_VERSION_2) + "." + String(SW_VERSION_3);
	AT_PRINTF("===============================================");
	AT_PRINTF("Indoor Comfort Sensor");
	AT_PRINTF("Built with RAK's WisBlock");
	AT_PRINTF("SW Version %d.%d.%d", g_sw_ver_1, g_sw_ver_2, g_sw_ver_3);
	AT_PRINTF("LoRa(R) is a registered trademark or service\nmark of Semtech Corporation or its affiliates.\nLoRaWAN(R) is a licensed mark.");
	AT_PRINTF("===============================================\n");

	// Enable EPD and I2C power
	pinMode(EPD_POWER, OUTPUT);
	digitalWrite(EPD_POWER, HIGH);

	// Enable CO2 & PM POWER
	pinMode(CO2_PM_POWER, OUTPUT);
	digitalWrite(CO2_PM_POWER, HIGH);
	pinMode(SET_PIN, OUTPUT);
	digitalWrite(SET_PIN, HIGH);

	// Enable VOC POWER
	pinMode(VOC_POWER, OUTPUT);
	digitalWrite(VOC_POWER, HIGH);

	// Enable PIR POWER
	pinMode(PIR_POWER, OUTPUT);
	digitalWrite(PIR_POWER, HIGH);

	// Check if device is running from battery
	float batt_val = read_batt();
	MYLOG("APP", "Battery level is %.3f", batt_val);
	g_is_using_battery = batt_val < 1000.0 ? false : true;

	/// \todo only for testing
	// g_is_using_battery = true;

	Wire.begin();
	delay(100);
	Wire.beginTransmission(0x52);
	byte error = Wire.endTransmission();
	if (error == 0)
	{
		if (!init_rak12002())
		{
			has_rak12002 = false;
		}
		else
		{
			has_rak12002 = true;
		}
	}
	else
	{
		has_rak12002 = false;
	}

#if HAS_EPD > 0
	MYLOG("APP", "Init RAK14000");
	init_rak14000();
#endif

	// Enable the modules
	has_rak1901 = init_rak1901();
	if (has_rak1901)
	{
		startup_rak1901();
		delay(250);
		read_rak1901();
		if (g_is_using_battery)
			shutdown_rak1901();
		AT_PRINTF("+EVT:RAK1901 OK\n");
	}
	has_rak1902 = init_rak1902();
	if (has_rak1902)
	{
		AT_PRINTF("+EVT:RAK1902 OK\n");
	}
	has_rak1903 = init_rak1903();
	if (has_rak1903)
	{
		AT_PRINTF("+EVT:RAK1903 OK\n");
	}
	has_rak1906 = init_rak1906();
	if (has_rak1906)
	{
		AT_PRINTF("+EVT:RAK1906 OK\n");
	}
	has_rak12010 = init_rak12010();
	if (has_rak1906)
	{
		AT_PRINTF("+EVT:RAK12010 OK\n");
	}
	// if (!g_is_using_battery)
	{
		has_rak12039 = init_rak12039();
		if (has_rak12039)
		{
			MYLOG("APP", "PM initialized");
			if (g_is_using_battery)
				startup_rak12039();
			read_rak12039();
			if (g_is_using_battery)
				shutdown_rak12039();
			AT_PRINTF("+EVT:RAK12039 OK\n");
		}
	}
	has_rak12037 = init_rak12037();
	if (has_rak12037)
	{
		MYLOG("APP", "CO2 initialized");
		if (g_is_using_battery)
			startup_rak12037();
		read_rak12037();
		if (g_is_using_battery)
			shutdown_rak12037();
		AT_PRINTF("+EVT:RAK12037 OK\n");
	}
	has_rak12047 = init_rak12047();
	if (has_rak12047)
	{
		MYLOG("APP", "VOC initialized");
		AT_PRINTF("+EVT:RAK12047 OK\n");
	}
	has_rgb = init_rgb();
	if (has_rgb)
	{
		AT_PRINTF("+EVT:RGB OK\n");
	}
	init_pir();
	init_button();

	if (g_is_using_battery)
	{
		// Switch off RGB
		set_rgb_color(0, 0, 0);
		shutdown_rgb();
	}
	if (has_rgb)
	{
		MYLOG("APP", "Start RGB toggle timer");
		// Start RGB toggle timer
		timer_rgb();
		g_rgb_on = false;
		if (g_is_using_battery)
		{
			g_rgb_timer.start();
		}
	}

	if (g_is_using_battery)
	{
		// Power down the whole system
		MYLOG("APP", "Shut down power");
		// Disable CO2 & PM POWER
		digitalWrite(CO2_PM_POWER, LOW);
	}
	if (has_rak12039)
	{
		// Prepare timer to send after the sensors were awake for 30 seconds
		g_sensor_timer.begin(30000, send_delayed, NULL, false);
	}
	else
	{
		// Prepare timer to send after the sensors were awake for 12 seconds
		g_sensor_timer.begin(12000, send_delayed, NULL, false);
	}

	// Initialize User AT commands
	init_user_at();

	return true;
}

/**
 * @brief Handle events
 * 		Events can be
 * 		- timer (setup with AT+SENDINT=xxx)
 * 		- interrupt events
 * 		- wake-up signals from other tasks
 */
void app_event_handler(void)
{
	// Check if there is event for app_event_handler
	if ((g_task_event_type & APP_EVENT) == 0)
	{
		return;
	}

	/*********************************************************/
	/* Timer wakeup event handling                           */
	/*********************************************************/
	if ((g_task_event_type & STATUS) == STATUS)
	{
		g_task_event_type &= N_STATUS;
		MYLOG("APP", "Timer wakeup");

		// Set a no screen update flag for join success
		second_screen = true;

		if (g_is_using_battery)
		{
			digitalWrite(CO2_PM_POWER, HIGH);
			digitalWrite(SET_PIN, HIGH);
		}
		// Start sensor measurements
		if (has_rak1901)
		{
			startup_rak1901();
		}
		if (has_rak1902)
		{
			startup_rak1902();
		}
		if (has_rak1903)
		{
			startup_rak1903();
		}
		if (has_rak1906)
		{
			startup_rak1906();
		}
		if (has_rak12010)
		{
			startup_rak12010();
		}
		if (has_rak12037)
		{
			startup_rak12037();
		}
		if (has_rak12039)
		{
			startup_rak12039();
		}
		if (has_rak12047)
		{
			// Always running in the background
		}

		g_sensor_timer.start();
	}

	/*********************************************************/
	/* Sensor reading end event handling                     */
	/*********************************************************/
	if ((g_task_event_type & SEND_NOW) == SEND_NOW)
	{
		g_task_event_type &= N_SEND_NOW;

		// Reset the packet
		g_solution_data.reset();

		// Read last measurement from available sensors
		if (has_rak1901)
		{
			read_rak1901();
			shutdown_rak1901();
		}
		if (has_rak1902)
		{
			shutdown_rak1902();
		}
		if (has_rak1903)
		{
			read_rak1903();
			shutdown_rak1903();
		}
		if (has_rak1906)
		{
			read_rak1906();
			shutdown_rak1906();
		}
		if (has_rak12010)
		{
			read_rak12010();
			shutdown_rak12010();
		}
		if (has_rak12037)
		{
			read_rak12037();
			shutdown_rak12037();
		}
		if (has_rak12039)
		{
			startup_rak12039();
			delay(500);
			read_rak12039();
			shutdown_rak12039();
		}
		if (has_rak12047)
		{
			read_rak12047();
		}
		// Get battery level
		float batt_level_f = read_batt();
		g_solution_data.addVoltage(LPP_CHANNEL_BATT, batt_level_f / 1000.0);

		// Add occupation information
		g_solution_data.addPresence(LPP_CHANNEL_SWITCH, g_occupied);

		if (g_lorawan_settings.lorawan_enable)
		{
			if (g_lpwan_has_joined)
			{
				// Send a confirmed package every 30 packets to check connection
				if (check_connection > 30)
				{
					g_lorawan_settings.confirmed_msg_enabled = LMH_CONFIRMED_MSG;
					check_connection = 0;
				}
				else
				{
					g_lorawan_settings.confirmed_msg_enabled = LMH_UNCONFIRMED_MSG;
				}
				check_connection++;

				lmh_error_status result = send_lora_packet(g_solution_data.getBuffer(), g_solution_data.getSize(), 2);
				switch (result)
				{
				case LMH_SUCCESS:
					MYLOG("APP", "Packet enqueued");
					break;
				case LMH_BUSY:
					MYLOG("APP", "LoRa transceiver is busy");
					api_wake_loop(DISP_UPDATE);
					break;
				case LMH_ERROR:
					MYLOG("APP", "Packet error, too big to send with current DR");
					api_wake_loop(DISP_UPDATE);
					break;
				}
			}
			else
			{
				MYLOG("APP", "Network not joined, skip sending");
			}
		}
		else
		{
			g_solution_data.addDevID(LPP_CHANNEL_DEVID, &g_lorawan_settings.node_device_eui[4]);
			send_p2p_packet(g_solution_data.getBuffer(), g_solution_data.getSize());
		}

		if (g_is_using_battery)
		{
			digitalWrite(CO2_PM_POWER, LOW);
			digitalWrite(SET_PIN, LOW);
		}
	}

	/*********************************************************/
	/* Display event handling                                */
	/*********************************************************/
	// Display update event
	if ((g_task_event_type & DISP_UPDATE) == DISP_UPDATE)
	{
		g_task_event_type &= N_DISP_UPDATE;
#if HAS_EPD > 0
		// Refresh display
		MYLOG("APP", "Refresh RAK14000");

		startup_rak14000();

		refresh_rak14000();

		g_epd_off_timer.start();
#endif
	}

	// Display show join event
	if ((g_task_event_type & DISP_JOIN) == DISP_JOIN)
	{
		g_task_event_type &= N_DISP_JOIN;
#if HAS_EPD > 0
		// Refresh display
		MYLOG("APP", "Join RAK14000");

		startup_rak14000();

		// Show join on display
		rak14000_start_screen(true);

		g_epd_off_timer.start();
#endif
	}

	/*********************************************************/
	/* VOC sensor interval read event handling               */
	/*********************************************************/
	if ((g_task_event_type & VOC_REQ) == VOC_REQ)
	{
		g_task_event_type &= N_VOC_REQ;

		MYLOG("APP", "Handle VOC");

		if (has_rgb)
		{
			g_rgb_on = true;
			// Show air quality on RGB
			set_rgb_air_status();
			// MYLOG("APP", "Start timer for RGB off");
			g_rgb_timer.setPeriod(200);
			if (g_is_using_battery)
			{
				g_rgb_timer.start();
			}
		}
		run_rak12047_algo();
	}

	/*********************************************************/
	/* RGB LED event handling                                */
	/*********************************************************/
	if ((g_task_event_type & LED_REQ) == LED_REQ)
	{
		g_task_event_type &= N_LED_REQ;

		if (has_rgb)
		{
			// MYLOG("APP", "RGB LED");
			if (g_rgb_on)
			{
				// MYLOG("APP", "RGB is on");
				g_rgb_on = false;
				// Switch off RGB
				set_rgb_color(0, 0, 0);
				shutdown_rgb();

				if (!has_rak12047)
				{
					g_rgb_timer.setPeriod(29800);
					if (g_is_using_battery)
					{
						g_rgb_timer.start();
					} // MYLOG("APP", "RGB off started");
				}
				// MYLOG("APP", "RGB off not started");
			}
			else
			{
				// MYLOG("APP", "RGB is off");
				g_rgb_on = true;
				// Show air quality on RGB
				set_rgb_air_status();
				g_rgb_timer.setPeriod(200);
				if (g_is_using_battery)
				{
					g_rgb_timer.start();
				} // MYLOG("APP", "RGB on started");
			}
		}
	}

	/*********************************************************/
	/* PIR event handling                                    */
	/*********************************************************/
	// Unoccupied event
	if ((g_task_event_type & ROOM_EMPTY) == ROOM_EMPTY)
	{
		g_task_event_type &= N_ROOM_EMPTY;
		MYLOG("APP", "Room is not occupied, switch off the RGB");
		set_rgb_color(0, 0, 0);
		shutdown_rgb();
		g_rgb_timer.stop();
		g_rgb_on = false;
	}

	// Occupied event
	if ((g_task_event_type & MOTION) == MOTION)
	{
		g_task_event_type &= N_MOTION;
		MYLOG("APP", "Room is occupied, stop power savings");

		g_rgb_on = true;
		// Show air quality on RGB
		set_rgb_air_status();
		g_rgb_timer.setPeriod(200);
		if (g_is_using_battery)
		{
			g_rgb_timer.start();
		}
	}

	/*********************************************************/
	/* Device reset request event                            */
	/*********************************************************/
	// Handle Reset request
	if ((g_task_event_type & RST_REQ) == RST_REQ)
	{
		g_task_event_type &= N_RST_REQ;

		// Power up display
		startup_rak14000();
		rak14000_start_screen(false);
		delay(3000);
		api_reset();
	}
}

/**
 * @brief Handle BLE events
 *
 */
void ble_data_handler(void)
{
	if (g_enable_ble)
	{
		/**************************************************************/
		/**************************************************************/
		/// \todo BLE UART data arrived
		/// \todo or forward them to the AT command interpreter
		/// \todo parse them here
		/**************************************************************/
		/**************************************************************/
		if ((g_task_event_type & BLE_DATA) == BLE_DATA)
		{
			MYLOG("AT", "RECEIVED BLE");
			// BLE UART data arrived
			// in this example we forward it to the AT command interpreter
			g_task_event_type &= N_BLE_DATA;

			while (g_ble_uart.available() > 0)
			{
				at_serial_input(uint8_t(g_ble_uart.read()));
				delay(5);
			}
			at_serial_input(uint8_t('\n'));
		}
	}
}

/**
 * @brief Handle LoRa events
 *
 */
void lora_data_handler(void)
{
	// LoRa Join finished handling
	if ((g_task_event_type & LORA_JOIN_FIN) == LORA_JOIN_FIN)
	{
		g_task_event_type &= N_LORA_JOIN_FIN;
		if (g_join_result)
		{
			MYLOG("APP", "Successfully joined network");
#ifdef HAS_EPD
			if (!second_screen)
			{
				MYLOG("APP", "Update EPD");
				api_wake_loop(DISP_JOIN);
				api_wake_loop(STATUS);
			}
#endif
		}
		else
		{
			MYLOG("APP", "Join network failed");
			/// \todo here join could be restarted.
			lmh_join();
		}
	}

	// LoRa data handling
	if ((g_task_event_type & LORA_DATA) == LORA_DATA)
	{
		/**************************************************************/
		/**************************************************************/
		/// \todo LoRa data arrived
		/// \todo parse them here
		/**************************************************************/
		/**************************************************************/
		g_task_event_type &= N_LORA_DATA;
		MYLOG("APP", "Received package over LoRa");
		MYLOG("APP", "Last RSSI %d", g_last_rssi);

		char log_buff[g_rx_data_len * 3] = {0};
		uint8_t log_idx = 0;
		for (int idx = 0; idx < g_rx_data_len; idx++)
		{
			sprintf(&log_buff[log_idx], "%02X ", g_rx_lora_data[idx]);
			log_idx += 3;
		}
		MYLOG("APP", "%s", log_buff);
	}

	// LoRa TX finished handling
	if ((g_task_event_type & LORA_TX_FIN) == LORA_TX_FIN)
	{
		g_task_event_type &= N_LORA_TX_FIN;

		if (g_lorawan_settings.lorawan_enable)
		{
			if (g_lorawan_settings.confirmed_msg_enabled == LMH_UNCONFIRMED_MSG)
			{
				MYLOG("APP", "LPWAN TX cycle finished");
			}
			else
			{
				MYLOG("APP", "LPWAN TX cycle %s", g_rx_fin_result ? "finished ACK" : "failed NAK");
			}
			if (!g_rx_fin_result)
			{
				// Increase fail send counter
				send_fail++;

				if (send_fail == 10)
				{
					// Too many failed sendings, reset node and try to rejoin
					delay(100);
					api_reset();
				}
			}
		}
		else
		{
			MYLOG("APP", "P2P TX finished");
		}
		api_wake_loop(DISP_UPDATE);
	}
}
