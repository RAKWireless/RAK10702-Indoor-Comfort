/**
 * @file app.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Application specific functions. Mandatory to have init_app(),
 *        app_event_handler(), ble_data_handler(), lora_data_handler()
 *        and lora_tx_finished()
 * @version 0.2
 * @date 2022-01-30
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "app.h"
/** Timer for delayed sending to keep duty cycle */
SoftwareTimer delayed_sending;

/** Timer to switch off LED 15 seconds after sending */
SoftwareTimer rgb_toggle;

/** Flag if delayed sending is already activated */
bool delayed_active = false;

/** Set the device name, max length is 10 characters */
char g_ble_dev_name[10] = "RAK";

/** Send Fail counter **/
uint8_t join_send_fail = 0;

/** LoRaWAN packet */
WisCayenne g_solution_data(255);

/** Flag if a USB device is connected */
// bool g_no_usb = false;

/** Flag if the device is battery or permanent powered */
bool g_is_using_battery = false;

volatile bool new_dtr = false;
volatile bool new_rts = false;

/**
 * @brief Application specific setup functions
 *
 */
void setup_app(void)
{
	// Initialize Serial for debug output
	Serial.begin(115200);
	// delay(5000);
	time_t serial_timeout = millis();
	// On nRF52840 the USB serial is not available immediately
	while (!Serial)
	{
		if ((millis() - serial_timeout) < 5000)
		{
			delay(100);
			// digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
		}
		else
		{
			break;
		}
	}

	// Enable BLE
	g_enable_ble = true;
}

/**
 * @brief Application specific initializations
 *
 * @return true Initialization success
 * @return false Initialization failure
 */
bool init_app(void)
{
	MYLOG("APP", "init_app");

	api_set_version(SW_VERSION_1, SW_VERSION_2, SW_VERSION_3);

	AT_PRINTF("===============================================");
	AT_PRINTF("Air Quality Sensor");
	AT_PRINTF("Built with RAK's WisBlock");
	AT_PRINTF("SW Version %d.%d.%d", g_sw_ver_1, g_sw_ver_2, g_sw_ver_3);
	AT_PRINTF("LoRa(R) is a registered trademark or service\nmark of Semtech Corporation or its affiliates.\nLoRaWAN(R) is a licensed mark.");
	AT_PRINTF("===============================================\n");

	// Enable EPD and I2C power
	pinMode(EPD_POWER, OUTPUT);
	digitalWrite(EPD_POWER, HIGH);

#if HAS_EPD > 0
	MYLOG("APP", "Init RAK14000");
	init_rak14000();
#endif

	// CO2 & PM POWER
	pinMode(CO2_PM_POWER, OUTPUT);
	digitalWrite(CO2_PM_POWER, HIGH);

	// VOC POWER
	pinMode(VOC_POWER, OUTPUT);
	digitalWrite(VOC_POWER, HIGH);

	// PIR POWER
	pinMode(PIR_POWER, OUTPUT);
	digitalWrite(PIR_POWER, HIGH);

	delay(500);
	// Scan the I2C interfaces for devices
	find_modules();

	// Initialize RGB LED
	init_rgb();
	set_rgb_color(246, 190, 0); //

	// Initialize the User AT command list
	init_user_at();

	// Announce found modules with +EVT: over Serial
	announce_modules();

	AT_PRINTF("============================\n");

	Serial.flush();
	// Reset the packet
	g_solution_data.reset();

	// Init PIR
	init_pir();

	// BUTTON
	init_button();

	// Check if device is running from battery
	float batt_val = read_batt();
	MYLOG("APP", "Battery level is %.3f", batt_val);
	g_is_using_battery = batt_val < 1000.0 ? false : true;

#if FORCE_PWR_SRC == 1
	g_is_using_battery = false;
#elif FORCE_PWR_SRC == 2
	g_is_using_battery = true;
#endif

	rgb_toggle.begin(15000, do_rgb_toggle, NULL, false);
	// If on battery usage, start timer to switch off the RGB LED
	if (g_is_using_battery)
	{
		MYLOG("APP", "Device is battery powered!");
		rgb_toggle.start();
	}
	else
	{
		MYLOG("APP", "Device is external powered!");
	}
	// Prepare timer to send after the sensors were awake for 30 seconds
	delayed_sending.begin(30000, send_delayed, NULL, false);

	// If using LoRa P2P, there is no need to wait for the device to join
	if (!g_lorawan_settings.lorawan_enable)
	{
		api_wake_loop(STATUS);
		MYLOG("APP", "LoRa P2P => wake up loop");
	}
	else
	{
		// Check if auto join is disabled for LoRaWAN
		if (!g_lorawan_settings.auto_join)
		{
			api_wake_loop(STATUS);
			MYLOG("APP", "LoRaWAN autojoin disabled");
		}

		// Check if the send interval is enabled
		if (g_lorawan_settings.send_repeat_time == 0)
		{
			// Enable a 5 minutes send interval to get some data on the screen
			// even if there is no send interval defined
			g_lorawan_settings.send_repeat_time = 300000;
			api_timer_restart(g_lorawan_settings.send_repeat_time);
			api_wake_loop(STATUS);
			MYLOG("APP", "LoRaWAN send interval is 0");
		}
	}

	digitalWrite(EPD_POWER, LOW);
	Wire.end();
	pinMode(WB_I2C1_SDA, INPUT);
	pinMode(WB_I2C1_SCL, INPUT);
	digitalWrite(CO2_PM_POWER, LOW);
	return true;
}

bool switch_power_off = true;

/**
 * @brief Application specific event handler
 *        Requires as minimum the handling of STATUS event
 *        Here you handle as well your application specific events
 */
void app_event_handler(void)
{
	digitalWrite(EPD_POWER, HIGH);
	Wire.begin();

	// Handle Reset request
	if ((g_task_event_type & RST_REQ) == RST_REQ)
	{
		g_task_event_type &= N_RST_REQ;
		// if (g_epd_off)
		// {
		// 	MYLOG("BTN", "EPD was off");
		// startup_rak14000();
		// }
		rak14000_start_screen(false);
		delay(3000);
		api_reset();
	}

	// Handle Display updates
	if ((g_task_event_type & DISP_UPDATE) == DISP_UPDATE)
	{
		g_task_event_type &= N_DISP_UPDATE;
		MYLOG("APP", "Switch display UI");

		refresh_rak14000();

		// Screen update takes time, put a delay here
		delay(2000);
		if (g_ui_selected == 2)
		{
			g_ui_selected = g_ui_last;
		}
	}

	// Toggle RGB visibility
	if ((g_task_event_type & LED_REQ) == LED_REQ)
	{
		g_task_event_type &= N_LED_REQ;
		MYLOG("APP", "RGB toggle every 15 seconds");

		set_rgb_color(0, 0, 0);
	}

	// Unoccupied event
	if ((g_task_event_type & ROOM_EMPTY) == ROOM_EMPTY)
	{
		g_task_event_type &= N_ROOM_EMPTY;
		MYLOG("APP", "Room is not occupied, start power savings");
		set_rgb_color(0, 0, 0);
	}

	// Occupied event
	if ((g_task_event_type & MOTION_TRIGGER) == MOTION_TRIGGER)
	{
		g_task_event_type &= N_MOTION_TRIGGER;
		MYLOG("APP", "Room is occupied, stop power savings");
		if (g_air_status == 0)
		{
			set_rgb_color(0, 0, 128);
		}
		else if (g_air_status == 128)
		{
			set_rgb_color(128, 128, 0);
		}
		else
		{
			set_rgb_color(128, 0, 0);
		}
		if (g_is_using_battery)
		{
			MYLOG("APP", "On battery, switch off the RGB after 15 seconds");
			rgb_toggle.start();
		}
	}

	if ((g_task_event_type & STATUS) == STATUS)
	{
		g_task_event_type &= N_STATUS;

		switch_power_off = false;
		if (!g_is_using_battery)
		{
			g_task_event_type |= SEND_NOW;
			power_modules(true);
		}
		else
		{
			MYLOG("APP", "Wake-up, power up sensors");
			power_modules(true);
			delayed_sending.start();
		}
	}

	// Timer triggered event
	if ((g_task_event_type & SEND_NOW) == SEND_NOW)
	{
		g_task_event_type &= N_SEND_NOW;

		switch_power_off = true;
		MYLOG("APP", "Start reading and sending");

		// Reset the packet
		g_solution_data.reset();

		// Get values from the connected modules
		get_sensor_values();

		if (g_is_using_battery)
		{
			// Get battery level
			float batt_level_f = read_batt();
			g_solution_data.addVoltage(LPP_CHANNEL_BATT, batt_level_f / 1000.0);
		}
		// Add occupation information
		g_solution_data.addPresence(LPP_CHANNEL_SWITCH, g_occupied);

#if HAS_EPD > 0
		// Refresh display
		MYLOG("APP", "Refresh RAK14000");
		refresh_rak14000();
#endif

		MYLOG("APP", "Packetsize %d", g_solution_data.getSize());

		if (g_lorawan_settings.lorawan_enable)
		{
			if (g_lpwan_has_joined)
			{
				lmh_error_status result = send_lora_packet(g_solution_data.getBuffer(), g_solution_data.getSize());
				switch (result)
				{
				case LMH_SUCCESS:
					MYLOG("APP", "Packet enqueued");
					break;
				case LMH_BUSY:
					MYLOG("APP", "LoRa transceiver is busy");
					AT_PRINTF("+EVT:BUSY\n");
					break;
				case LMH_ERROR:
					AT_PRINTF("+EVT:SIZE_ERROR\n");
					MYLOG("APP", "Packet error, too big to send with current DR");
					break;
				}
			}
		}
		else
		{
			// Add the device DevEUI as a device ID to the packet
			uint8_t packet_buffer[g_solution_data.getSize() + 8];
			memcpy(packet_buffer, g_lorawan_settings.node_device_eui, 8);
			memcpy(&packet_buffer[8], g_solution_data.getBuffer(), g_solution_data.getSize());

			// Send packet over LoRa
			if (send_p2p_packet(packet_buffer, g_solution_data.getSize() + 8))
			{
				MYLOG("APP", "Packet enqueued");
			}
			else
			{
				AT_PRINTF("+EVT:SIZE_ERROR\n");
				MYLOG("APP", "Packet too big");
			}
		}
		// Reset the packet
		g_solution_data.reset();

		// Check if room is occupied
		if (g_occupied)
		{
			// Occupied, use default update time
			api_timer_restart(g_lorawan_settings.send_repeat_time);
		}
		else
		{
			// Not occupied, double the update time
			api_timer_restart(g_lorawan_settings.send_repeat_time * 2);
		}

		// if (g_is_using_battery)
		{
			MYLOG("APP", ">>>> POWER OFF");
			// Power down the modules
			power_modules(false);
		}
	}

	// VOC read request event
	if ((g_task_event_type & VOC_REQ) == VOC_REQ)
	{
		g_task_event_type &= N_VOC_REQ;
		MYLOG("APP", "Handle VOC");
		do_read_rak12047();
	}

	/*********************************************/
	/** Select between Bosch BSEC algorithm for  */
	/** IAQ index or simple T/H/P readings       */
	/*********************************************/
	// BSEC read request event
	if ((g_task_event_type & BSEC_REQ) == BSEC_REQ)
	{
		g_task_event_type &= N_BSEC_REQ;

#if USE_BSEC == 1
		do_read_rak1906_bsec();
#endif
	}

	if (switch_power_off)
	{
		digitalWrite(EPD_POWER, LOW);
		Wire.end();
		pinMode(WB_I2C1_SDA, INPUT);
		pinMode(WB_I2C1_SCL, INPUT);
	}
}

/**
 * @brief Handle BLE UART data
 *
 */
void ble_data_handler(void)
{
	if (g_enable_ble)
	{
		// BLE UART data handling
		if ((g_task_event_type & BLE_DATA) == BLE_DATA)
		{
			// MYLOG("AT", "RECEIVED BLE");
			/** BLE UART data arrived */
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
 * @brief Handle received LoRa Data
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
			AT_PRINTF("+EVT:JOINED\n");

			// Reset join failed counter
			join_send_fail = 0;

			// Start a sensor reading
			api_wake_loop(STATUS);
		}
		else
		{
			MYLOG("APP", "Join network failed");
			AT_PRINTF("+EVT:JOIN FAILED\n");
			/// \todo here join could be restarted.
			lmh_join();

			join_send_fail++;
			if (join_send_fail == 10)
			{
				// Too many failed join requests, reset node and try to rejoin
				delay(100);
				api_reset();
			}
		}
		if ((join_send_fail < 2) && !g_join_result)
		{
			// Force a sensor reading
			api_wake_loop(STATUS);
		}
	}

	// LoRa TX finished handling
	if ((g_task_event_type & LORA_TX_FIN) == LORA_TX_FIN)
	{
		g_task_event_type &= N_LORA_TX_FIN;

		// #if HAS_EPD > 0
		// 		// Refresh display
		// 		MYLOG("APP", "Refresh RAK14000");
		// 		api_wake_loop(DISP_UPDATE);
		// #endif
		MYLOG("APP", "LoRa TX cycle %s", g_rx_fin_result ? "finished ACK" : "failed NAK");

		if ((g_lorawan_settings.confirmed_msg_enabled) && (g_lorawan_settings.lorawan_enable))
		{
			AT_PRINTF("+EVT:SEND CONFIRMED %s\n", g_rx_fin_result ? "SUCCESS" : "FAIL");
		}
		else
		{
			AT_PRINTF("+EVT:SEND OK\n");
		}

		if (g_is_using_battery)
		{
			MYLOG("APP", "On battery, switch off the RGB after 15 seconds");
			rgb_toggle.start();
		}
		if (!g_rx_fin_result)
		{
			// Increase fail send counter
			join_send_fail++;

			if (join_send_fail == 10)
			{
				// Too many failed sendings, reset node and try to rejoin
				delay(100);
				api_reset();
			}
		}
	}

	// LoRa data handling
	if ((g_task_event_type & LORA_DATA) == LORA_DATA)
	{
		g_task_event_type &= N_LORA_DATA;
		MYLOG("APP", "Received package over LoRa");

		if (g_lorawan_settings.lorawan_enable)
		{
			// Check if uplink was a send frequency change command
			if ((g_last_fport == 3) && (g_rx_data_len == 6))
			{
				if (g_rx_lora_data[0] == 0xAA)
				{
					if (g_rx_lora_data[1] == 0x55)
					{
						change_sendinterval();
					}
				}
			}

			char rx_msg[512] = {0};
			int len = sprintf(rx_msg, "+EVT:RX_1:%d:%d:UNICAST:%d:", g_last_rssi, g_last_snr, g_last_fport);
			for (int idx = 0; idx < g_rx_data_len; idx++)
			{
				sprintf(&rx_msg[len], "%02X", g_rx_lora_data[idx]);
				len += 2;
			}
			AT_PRINTF("%s\n", rx_msg);
		}
		else
		{
			// Check if uplink was a send frequency change command
			if (g_rx_data_len == 6)
			{
				if (g_rx_lora_data[0] == 0xAA)
				{
					if (g_rx_lora_data[1] == 0x55)
					{
						change_sendinterval();
					}
				}
			}

			char rx_msg[512] = {0};
			int len = sprintf(rx_msg, "+EVT:RXP2P:%d:%d:", g_last_rssi, g_last_snr);
			for (int idx = 0; idx < g_rx_data_len; idx++)
			{
				sprintf(&rx_msg[len], "%02X", g_rx_lora_data[idx]);
				len += 2;
			}
			AT_PRINTF("%s\n", rx_msg);
		}
	}
}

/**
 * @brief Timer function used to avoid sending packages too often.
 * 			Delays the next package by 10 seconds
 *
 * @param unused
 * 			Timer handle, not used
 */
void send_delayed(TimerHandle_t unused)
{
	api_wake_loop(SEND_NOW);
	delayed_sending.stop();
}

/**
 * @brief Toggle RGB Visibility
 *
 * @param unused
 */
void do_rgb_toggle(TimerHandle_t unused)
{
	api_wake_loop(LED_REQ);
}

/**
 * @brief Change the send interval and restart the timer
 *
 */
void change_sendinterval(void)
{
	uint32_t new_send_frequency = 0;
	new_send_frequency |= (uint32_t)(g_rx_lora_data[2]) << 24;
	new_send_frequency |= (uint32_t)(g_rx_lora_data[3]) << 16;
	new_send_frequency |= (uint32_t)(g_rx_lora_data[4]) << 8;
	new_send_frequency |= (uint32_t)(g_rx_lora_data[5]);

	MYLOG("APP", "Received new send frequency %ld s\n", new_send_frequency);
	// Save the new send frequency
	g_lorawan_settings.send_repeat_time = new_send_frequency * 1000;

	// Set the timer to the new send frequency
	api_timer_restart(g_lorawan_settings.send_repeat_time);
	// Save the new send frequency
	save_settings();
}