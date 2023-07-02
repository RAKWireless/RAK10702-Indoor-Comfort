/**
 * @file dr_calculator.cpp
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Calculate the required datarate from the payload size
 * @version 0.1
 * @date 2023-01-06
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "app.h"

uint16_t in865_eu433_ru864_eu868_ps[16] = {51, 51, 51, 115, 242, 242, 242, 242, 0, 0, 0, 0, 0, 0, 0, 0};
// uint16_t in865_ps[16] =                {51, 51, 51, 115, 242, 242, 242, 242, 0, 0, 0, 0, 0, 0, 0, 0};
// uint16_t eu868_ps[16] =                {51, 51, 51, 115, 242, 242, 242, 242, 0, 0, 0, 0, 0, 0, 0, 0};
// uint16_t eu433_ps[16] =                {51, 51, 51, 115, 242, 242, 242, 242, 0, 0, 0, 0, 0, 0, 0, 0};
// uint16_t ru864_ps[16] =                {51, 51, 51, 115, 222, 242, 242, 242, 0, 0, 0, 0, 0, 0, 0, 0};
uint16_t au915_ps[16] = {51, 51, 51, 115, 242, 242, 242, 0, 53, 129, 242, 242, 242, 242, 0, 0};
uint16_t cn470_kr920_ps[16] = {51, 51, 51, 115, 242, 242, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
// uint16_t cn470_ps[16] =    {51, 51, 51, 115, 242, 242, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
// uint16_t kr920_ps[16] =    {51, 51, 51, 115, 242, 242, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint16_t us915_ps[16] = {11, 53, 125, 242, 242, 0, 0, 0, 53, 129, 242, 242, 242, 242, 0, 0};
uint16_t as923_ps[16] = {0, 0, 19, 61, 133, 250, 250, 250, 0, 0, 0, 0, 0, 0, 0, 0};

uint16_t *region_map[13] = {as923_ps, au915_ps, cn470_kr920_ps, cn470_kr920_ps, in865_eu433_ru864_eu868_ps,
							in865_eu433_ru864_eu868_ps, in865_eu433_ru864_eu868_ps, cn470_kr920_ps, us915_ps, as923_ps, as923_ps, as923_ps, in865_eu433_ru864_eu868_ps};

// Function declarations
bool check_dr_valid(uint16_t payload_size);

/**
 * @brief Get the minimum datarate based on region and required payload size
 *
 * @param region LoRaWAN region
 *               0 = AS923-1, 1 = AU915, 2 = CN470, 3 = CN779, 4 = EU433, 5 = EU868, 6 = IN865, 7 = KR920,
 *               8 = US915, 9 = AS923-2, 10 = AS923-3, 11 = AS923-4, 12 = RU864)
 * @param payload_size required payload size
 * @return uint8_t datarate 0 to 15 or 16 if no matching DR could be found
 */
uint8_t get_min_dr(uint16_t region, uint16_t payload_size)
{
	MYLOG("DR_CALC", "Got region %d and payload size %d", region, payload_size);
	// Get the datarate - payload size map
	uint16_t *region_ps = region_map[region];
	// MYLOG("DR_CALC", "Available packet sizes:");
	// for (int idx = 0; idx < 15; idx++)
	// {
	// 	Serial.printf("DR %d max size %d\n", idx, region_ps[idx]);
	// }
	// Go through all payload sizes
	for (uint8_t idx = 0; idx < 16; idx++)
	{
		// Check if dr payload size is larger than requested payload size
		if (payload_size < region_ps[idx])
		{
			// Found a datarate that can carry the payload size
			return idx;
		}
	}
	// No matching datarate for the payload size found
	return 16;
}

bool check_dr_valid(uint16_t payload_size)
{
	LoRaMacTxInfo_t txInfo;
	if (LoRaMacQueryTxPossible(payload_size, &txInfo) != LORAMAC_STATUS_OK)
	{
		MYLOG("DR_CALC", "Max possible payload size %d", txInfo.MaxPossiblePayload);
		MYLOG("DR_CALC", "Current DR payload size   %d", txInfo.CurrentPayloadSize);
		return false;
	}
	MYLOG("DR_CALC", "Max possible payload size %d", txInfo.MaxPossiblePayload);
	MYLOG("DR_CALC", "Current DR payload size   %d", txInfo.CurrentPayloadSize);

	if (txInfo.MaxPossiblePayload < payload_size)
	{
		MYLOG("DR_CALC", "Max possible payload size < payload size");
		return false;
	}
	return true;
}