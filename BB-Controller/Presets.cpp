//--------------------------------------------------------------------
//	Presets.cpp
//
//  (c) PE1MUD, PE1OBW 2024
//--------------------------------------------------------------------
#include <Arduino.h>
#include <stdint.h>
#include "globals.h"

void LoadPreset(uint8_t mem)
{
	HWWrite(I2C_ACCESS_COMMAND_READ_PRESET, &mem, 1);
	CommandWait(I2C_ACCESS_COMMAND_READ_PRESET);
	HWRead(I2C_ACCESS_SETTINGS, (uint8_t *) &settings[0], sizeof(settings[0]));
	settings[0].general.last_recalled_presetnr = mem; // Save the last recalled value
	// Make a working copy of current settings, used to track changes
	memcpy((void *)&settings[1], (void *)&settings[0], sizeof(settings[0]));
}

void LoadPreview(uint8_t mem)
{
	HWWrite(I2C_ACCESS_COMMAND_VIEW_PRESET, &mem, 1);
	CommandWait(I2C_ACCESS_COMMAND_VIEW_PRESET);
	HWRead(I2C_ACCESS_VIEW_SETTINGS, (uint8_t *) &settings[2], sizeof(settings[2]));
	// Serial.print("Last recalled preset during preview: ");
  // Serial.println(settings[0].general.last_recalled_presetnr);
	// // Make a working copy of current settings, used to track changes
	// memcpy((void *)&settings[1], (void *)&settings[0], sizeof(settings[0]));
}

void LoadPresetMirror(uint8_t mem)
{
	HWWrite(I2C_ACCESS_COMMAND_READ_PRESET, &mem, 1);
	CommandWait(I2C_ACCESS_COMMAND_READ_PRESET);
	HWRead(I2C_ACCESS_SETTINGS, (uint8_t *) &settings[1], sizeof(settings[1]));
	// settings[0].general.last_recalled_presetnr = mem; // Save the last recalled value
	// // Make a working copy of current settings, used to track changes
	// memcpy((void *)&settings[1], (void *)&settings[0], sizeof(settings[0]));
}

void SavePreset(uint8_t mem)
{
  HWWrite(I2C_ACCESS_COMMAND_STORE_PRESET, &mem, 1);
  CommandWait(I2C_ACCESS_COMMAND_STORE_PRESET);
  HWRead(I2C_ACCESS_READ_PRESET_STATUS, (uint8_t *) &preset_status, 4); // read the preset status
  memcpy((void *)&settings[1], (void *)&settings[0], sizeof(settings[0])); // the preset was saved, update the shadow settings
}

void ErasePreset(uint8_t mem)
{
  HWWrite(I2C_ACCESS_COMMAND_ERASE_PRESET, &mem, 1);
  CommandWait(I2C_ACCESS_COMMAND_ERASE_PRESET);
  HWRead(I2C_ACCESS_READ_PRESET_STATUS, (uint8_t *) &preset_status, 4); // read the preset status
}