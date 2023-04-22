/**
 * @file user_at_cmd.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Handle user defined AT commands
 * @version 0.4
 * @date 2022-01-29
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "app.h"
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>
using namespace Adafruit_LittleFS_Namespace;

/** Filename to save GPS precision setting */
static const char batt_name[] = "BATT";

/** File to save battery check status */
File batt_check(InternalFS);

/** Filename to save UI setting */
static const char ui_name[] = "UI";

/** File to save UI status */
File ui_check(InternalFS);

/*****************************************
 * Set UI commands
 *****************************************/

/**
 * @brief Set UI display selection
 *
 * @param str selected UI as String, 0 = scientific, 1 = iconized
 * @return int AT_SUCCESS if ok, AT_ERRNO_PARA_FAIL if invalid value
 */
static int at_set_ui(char *str)
{
	long new_ui = strtol(str, NULL, 0);

	if (new_ui > 1)
	{
		return AT_ERRNO_PARA_NUM;
	}
	g_ui_selected = new_ui;
	save_ui_settings(new_ui);
	return AT_SUCCESS;
}

/**
 * @brief Select UI mode
 *
 * @return int AT_SUCCESS
 */
int at_query_ui(void)
{
	AT_PRINTF("%d", g_ui_selected);
	return AT_SUCCESS;
}

/**
 * @brief List of all available commands with short help and pointer to functions
 *
 */
atcmd_t g_user_at_cmd_list_ui[] = {
	/*|    CMD    |     AT+CMD?      |    AT+CMD=?    |  AT+CMD=value |  AT+CMD  | Permissions |*/
	// Module commands
	{"+UI", "Switch display UI, 0 = scientific, 1 = iconized", at_query_ui, at_set_ui, NULL, "RW"},
};

/**
 * @brief Read saved setting UI selection
 *
 */
void read_ui_settings(void)
{
	if (InternalFS.exists(ui_name))
	{
		g_ui_selected = 0;
		MYLOG("USR_AT", "File found, set UI 0 (scientific)");
	}
	else
	{
		g_ui_selected = 1;
		MYLOG("USR_AT", "File not found, set UI 1 (iconized)");
	}

	save_ui_settings(g_ui_selected);
}

/**
 * @brief Save the UI settings
 *
 */
void save_ui_settings(uint8_t ui_selected)
{
	if (ui_selected == 0)
	{
		ui_check.open(ui_name, FILE_O_WRITE);
		ui_check.write("1");
		ui_check.close();
		MYLOG("USR_AT", "Created File for UI selection 0");
	}
	else
	{
		InternalFS.remove(ui_name);
		MYLOG("USR_AT", "Remove File for UI selection 1");
	}
}

/*****************************************
 * Query modules AT commands
 *****************************************/

/**
 * @brief Query found modules
 *
 * @return int 0
 */
int at_query_modules(void)
{
	announce_modules();
	return 0;
}

/**
 * @brief List of all available commands with short help and pointer to functions
 *
 */
atcmd_t g_user_at_cmd_list_modules[] = {
	/*|    CMD    |     AT+CMD?      |    AT+CMD=?    |  AT+CMD=value |  AT+CMD  | Permissions |*/
	// Module commands
	{"+MOD", "List all connected I2C devices", at_query_modules, NULL, at_query_modules, "RW"},
};

/*****************************************
 * RTC AT commands
 *****************************************/

/**
 * @brief Set RTC time
 *
 * @param str time as string, format <year>:<month>:<date>:<hour>:<minute>
 * @return int 0 if successful, otherwise error value
 */
static int at_set_rtc(char *str)
{
	uint16_t year;
	uint8_t month;
	uint8_t date;
	uint8_t hour;
	uint8_t minute;

	char *param;

	param = strtok(str, ":");

	// year:month:date:hour:minute

	if (param != NULL)
	{
		/* Check year */
		year = strtoul(param, NULL, 0);

		if (year > 3000)
		{
			return AT_ERRNO_PARA_VAL;
		}

		/* Check month */
		param = strtok(NULL, ":");
		if (param != NULL)
		{
			month = strtoul(param, NULL, 0);

			if ((month < 1) || (month > 12))
			{
				return AT_ERRNO_PARA_VAL;
			}

			// Check day
			param = strtok(NULL, ":");
			if (param != NULL)
			{
				date = strtoul(param, NULL, 0);

				if ((date < 1) || (date > 31))
				{
					return AT_ERRNO_PARA_VAL;
				}

				// Check hour
				param = strtok(NULL, ":");
				if (param != NULL)
				{
					hour = strtoul(param, NULL, 0);

					if (hour > 24)
					{
						return AT_ERRNO_PARA_VAL;
					}

					// Check minute
					param = strtok(NULL, ":");
					if (param != NULL)
					{
						minute = strtoul(param, NULL, 0);

						if (minute > 59)
						{
							return AT_ERRNO_PARA_VAL;
						}

						set_rak12002(year, month, date, hour, minute);

						return 0;
					}
				}
			}
		}
	}
	return AT_ERRNO_PARA_NUM;
}

/**
 * @brief Get RTC time
 *
 * @return int 0
 */
static int at_query_rtc(void)
{
	// Get date/time from the RTC
	read_rak12002();
	AT_PRINTF("%d.%02d.%02d %d:%02d:%02d", g_date_time.year, g_date_time.month, g_date_time.date, g_date_time.hour, g_date_time.minute, g_date_time.second);
	return 0;
}

atcmd_t g_user_at_cmd_list_rtc[] = {
	/*|    CMD    |     AT+CMD?      |    AT+CMD=?    |  AT+CMD=value |  AT+CMD  | Permissions |*/
	// RTC commands
	{"+RTC", "Get/Set RTC time and date", at_query_rtc, at_set_rtc, at_query_rtc, "RW"},
};

/** Number of user defined AT commands */
uint8_t g_user_at_cmd_num = 0;

/** Pointer to the combined user AT command structure */
atcmd_t *g_user_at_cmd_list;

#define TEST_ALL_CMDS 0

/**
 * @brief Initialize the user defined AT command list
 *
 */
void init_user_at(void)
{
	uint16_t index_next_cmds = 0;
	uint16_t required_structure_size = sizeof(g_user_at_cmd_list_modules);
	MYLOG("USR_AT", "Structure size %d Modules", required_structure_size);
	required_structure_size += sizeof(g_user_at_cmd_list_ui);
	MYLOG("USR_AT", "Structure size %d UI", required_structure_size);

	// Get required size of structure
	if (found_sensors[RTC_ID].found_sensor)
	{
		required_structure_size += sizeof(g_user_at_cmd_list_rtc);

		MYLOG("USR_AT", "Structure size %d RTC", required_structure_size);
	}

	// Reserve memory for the structure
	g_user_at_cmd_list = (atcmd_t *)malloc(required_structure_size);

	// Add AT commands to structure
	MYLOG("USR_AT", "Adding module AT commands");
	g_user_at_cmd_num += sizeof(g_user_at_cmd_list_modules) / sizeof(atcmd_t);
	memcpy((void *)&g_user_at_cmd_list[index_next_cmds], (void *)g_user_at_cmd_list_modules, sizeof(g_user_at_cmd_list_modules));
	index_next_cmds += sizeof(g_user_at_cmd_list_modules) / sizeof(atcmd_t);
	MYLOG("USR_AT", "Index after adding modules check %d", index_next_cmds);
	MYLOG("USR_AT", "Adding UI AT commands");
	g_user_at_cmd_num += sizeof(g_user_at_cmd_list_ui) / sizeof(atcmd_t);
	memcpy((void *)&g_user_at_cmd_list[index_next_cmds], (void *)g_user_at_cmd_list_ui, sizeof(g_user_at_cmd_list_ui));
	index_next_cmds += sizeof(g_user_at_cmd_list_ui) / sizeof(atcmd_t);
	MYLOG("USR_AT", "Index after adding UI commands %d", index_next_cmds);

	if (found_sensors[RTC_ID].found_sensor)
	{
		MYLOG("USR_AT", "Adding RTC user AT commands");
		g_user_at_cmd_num += sizeof(g_user_at_cmd_list_rtc) / sizeof(atcmd_t);
		memcpy((void *)&g_user_at_cmd_list[index_next_cmds], (void *)g_user_at_cmd_list_rtc, sizeof(g_user_at_cmd_list_rtc));
		index_next_cmds += sizeof(g_user_at_cmd_list_rtc) / sizeof(atcmd_t);
		MYLOG("USR_AT", "Index after adding RTC %d", index_next_cmds);
	}
}
