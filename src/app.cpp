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
SoftwareTimer rgb_off;

/** Flag if delayed sending is already activated */
bool delayed_active = false;

/** Set the device name, max length is 10 characters */
char g_ble_dev_name[10] = "RAK";

/** Send Fail counter **/
uint8_t join_send_fail = 0;

/** LoRaWAN packet */
WisCayenne g_solution_data(255);

uint8_t unoccupied_counter = 0;

bool g_is_unoccupied = false;

bool g_is_using_battery = false;

char disp_txt[64] = {0};

bool rgb_on = true;

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

	// Enable PM and EPD power
	pinMode(EPD_POWER, OUTPUT);
	digitalWrite(EPD_POWER, HIGH);

	// CO2 & PM POWER
	pinMode(CO2_PM_POWER, OUTPUT);
	digitalWrite(CO2_PM_POWER, HIGH);

	// VOC POWER
	pinMode(VOC_POWER, OUTPUT);
	digitalWrite(VOC_POWER, HIGH);

#if HAS_EPD > 0
	MYLOG("APP", "Init RAK14000");
	init_rak14000();
#endif

	delay(500);
	// Scan the I2C interfaces for devices
	find_modules();

	// Initialize RGB LED
	init_rgb();

	// Initialize the User AT command list
	init_user_at();

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

	AT_PRINTF("============================");
	AT_PRINTF("Air Quality Sensor");
	AT_PRINTF("Built with RAK's WisBlock");
	AT_PRINTF("SW Version %d.%d.%d", g_sw_ver_1, g_sw_ver_2, g_sw_ver_3);
	AT_PRINTF("LoRa(R) is a registered trademark or service mark of Semtech Corporation or its affiliates.\nLoRaWAN(R) is a licensed mark.");
	AT_PRINTF("============================\n");
	// api_log_settings();

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

	set_rgb_color(255, 255, 0);

	// Check if device is running from battery
	MYLOG("APP", "Battery level is %.3f", read_batt());
	// g_is_using_battery = read_batt() < 50 ? false : true;

	// If on battery usage, start timer to
	if (g_is_using_battery)
	{
		MYLOG("APP", "Device is battery powered!");
		rgb_off.begin(15000, switch_rgb_off, NULL, false);
		rgb_off.start();
	}

	// Prepare timer to send after the sensors were awake for 30 seconds
	delayed_sending.begin(30000, send_delayed, NULL, false);

	if (!g_lorawan_settings.lorawan_enable)
	{
		api_wake_loop(STATUS);
	}
	return true;
}

/**
 * @brief Application specific event handler
 *        Requires as minimum the handling of STATUS event
 *        Here you handle as well your application specific events
 */
void app_event_handler(void)
{
	// Toggle RGB visibility
	if ((g_task_event_type & LED_REQ) == LED_REQ)
	{
		g_task_event_type &= N_LED_REQ;
		MYLOG("APP", "RGB toggle every 15 seconds");

		if (rgb_on)
		{
			set_rgb_color(0, 0, 0);
			rgb_on = false;
		}
		else
		{
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
			rgb_on = true;
		}
	}

	// Unoccupied event
	if ((g_task_event_type & ROOM_EMPTY) == ROOM_EMPTY)
	{
		g_task_event_type &= N_ROOM_EMPTY;
		MYLOG("APP", "Room is not occupied, start power savings");
		api_timer_restart(g_lorawan_settings.send_repeat_time * 2);
		// set_rgb_color(0, 0, 0);
		g_is_unoccupied = true;
	}

	// Occupied event
	if ((g_task_event_type & MOTION_TRIGGER) == MOTION_TRIGGER)
	{
		g_task_event_type &= N_MOTION_TRIGGER;
		MYLOG("APP", "Room is occupied, stop power savings");
		api_timer_restart(g_lorawan_settings.send_repeat_time);
		g_is_unoccupied = false;
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
			rgb_off.start();
		}
	}

	if ((g_task_event_type & STATUS) == STATUS)
	{
		MYLOG("APP", "Wake-up, power up sensors");
		// power_modules(true);
		g_task_event_type &= N_STATUS;
		delayed_sending.start();
	}

	// Timer triggered event
	if ((g_task_event_type & SEND_NOW) == SEND_NOW)
	{
		g_task_event_type &= N_SEND_NOW;
		MYLOG("APP", "Start reading and sending");

		// If BLE is enabled, restart Advertising
		if (g_enable_ble)
		{
			restart_advertising(15);
		}

		// Reset the packet
		g_solution_data.reset();

		// Get values from the connected modules
		get_sensor_values();

		// Get battery level
		float batt_level_f = read_batt();
		g_solution_data.addVoltage(LPP_CHANNEL_BATT, batt_level_f / 1000.0);

		// Add occupation information
		g_solution_data.addPresence(LPP_CHANNEL_SWITCH, g_is_unoccupied);

		MYLOG("APP", "Packetsize %d", g_solution_data.getSize());
		bool refresh_without_send = true;
		if (g_lorawan_settings.lorawan_enable)
		{
			if (g_lpwan_has_joined)
			{
				lmh_error_status result = send_lora_packet(g_solution_data.getBuffer(), g_solution_data.getSize());
				switch (result)
				{
				case LMH_SUCCESS:
					MYLOG("APP", "Packet enqueued");
					refresh_without_send = false;
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
			uint8_t packet_buffer[g_solution_data.getSize() + 8];
			memcpy(packet_buffer, g_lorawan_settings.node_device_eui, 8);
			memcpy(&packet_buffer[8], g_solution_data.getBuffer(), g_solution_data.getSize());

			// Send packet over LoRa
			if (send_p2p_packet(packet_buffer, g_solution_data.getSize() + 8))
			{
				MYLOG("APP", "Packet enqueued");
				refresh_without_send = false;
			}
			else
			{
				AT_PRINTF("+EVT:SIZE_ERROR\n");
				MYLOG("APP", "Packet too big");
			}
		}
		// Reset the packet
		g_solution_data.reset();

#if HAS_EPD > 0
		if (refresh_without_send)
		{
			// Refresh display
			MYLOG("APP", "Refresh RAK14000");
			wake_rak14000();
		}
#endif
		// Power down the modules
		// power_modules(false);
	}

	// VOC read request event
	if ((g_task_event_type & VOC_REQ) == VOC_REQ)
	{
		g_task_event_type &= N_VOC_REQ;

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
		}
		else
		{
			MYLOG("APP", "Join network failed");
			AT_PRINTF("+EVT:JOIN FAILED\n");
			/// \todo here join could be restarted.
			lmh_join();

			// If BLE is enabled, restart Advertising
			if (g_enable_ble)
			{
				restart_advertising(15);
			}

			join_send_fail++;
			if (join_send_fail == 10)
			{
				// Too many failed join requests, reset node and try to rejoin
				delay(100);
				api_reset();
			}
		}
		if (join_send_fail < 2)
		{
			// Force a sensor reading
			// Force a sensor reading
			api_wake_loop(STATUS);
		}
	}

	// LoRa TX finished handling
	if ((g_task_event_type & LORA_TX_FIN) == LORA_TX_FIN)
	{
		g_task_event_type &= N_LORA_TX_FIN;

#if HAS_EPD > 0
		// Refresh display
		MYLOG("APP", "Refresh RAK14000");
		wake_rak14000();
#endif
		MYLOG("APP", "LoRa TX cycle %s", g_rx_fin_result ? "finished ACK" : "failed NAK");

		if ((g_lorawan_settings.confirmed_msg_enabled) && (g_lorawan_settings.lorawan_enable))
		{
			AT_PRINTF("+EVT:SEND CONFIRMED %s\n", g_rx_fin_result ? "SUCCESS" : "FAIL");
		}
		else
		{
			AT_PRINTF("+EVT:SEND OK\n");
		}

		// if (g_is_using_battery)
		// {
		// 	MYLOG("APP", "On battery, switch off the RGB after 15 seconds");
		// 	rgb_off.start();
		// }
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
		// Check if uplink was a send frequency change command
		if ((g_last_fport == 3) && (g_rx_data_len == 6))
		{
			if (g_rx_lora_data[0] == 0xAA)
			{
				if (g_rx_lora_data[1] == 0x55)
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
			}
		}

		if (g_lorawan_settings.lorawan_enable)
		{
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
void switch_rgb_off(TimerHandle_t unused)
{
	api_wake_loop(LED_REQ);
}