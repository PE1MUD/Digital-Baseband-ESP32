//--------------------------------------------------------------------
//	TwoWire.cpp - not an implementation of, but some support routines
//
//  (c) PE1MUD, PE1OBW 2024
//--------------------------------------------------------------------
#include <stdint.h>
#include <Wire.h>
#include <EEPROM.h>
#include "globals.h"

#include "hw_control_registers.h"                 // FPGA control registers
#include "baseband_i2c_interface.h"               // FPGA I2C interface

void PrintAddress(uint16_t address)
{
  return;

  switch (address)
  {
    case I2C_ACCESS_DISPLAY:
      Serial.println("I2C_ACCESS_DISPLAY");
      break;
    case I2C_ACCESS_FONT_MEMORY:
      Serial.println("I2C_ACCESS_FONT_MEMORY");
      break;
    case I2C_ACCESS_SETTINGS:
      Serial.println("I2C_ACCESS_SETTINGS");
      break;
    case I2C_ACCESS_READOUT:
      Serial.println("I2C_ACCESS_READOUT");
      break;
    // case I2C_ACCESS_COMMAND:
    //   Serial.println("I2C_ACCESS_COMMAND");
    //   break;
    case I2C_ACCESS_VIEW_SETTINGS:
      Serial.println("I2C_ACCESS_VIEW_SETTINGS");
      break;
    case I2C_ACCESS_READ_PRESET_STATUS:
      Serial.println("I2C_ACCESS_DISPLAY");
      break;
    case I2C_ACCESS_INFO:
      Serial.println("I2C_ACCESS_READ_PRESET_STATUS");
      break;
    case I2C_ACCESS_FLASH:
      Serial.println("I2C_ACCESS_FLASH");
      break;
    case I2C_ACCESS_PATTERN_MEMORY:
      Serial.println("I2C_ACCESS_PATTERN_MEMORY");
      break;
    case I2C_ACCESS_IO_REGISTERS:
      Serial.println("I2C_ACCESS_IO_REGISTERS");
      break;
    case I2C_ACCESS_ROUTING_REGISTERS:
      Serial.println("I2C_ACCESS_ROUTING_REGISTERS");
      break;
    case I2C_ACCESS_COMMAND_UPDATE_SETTINGS:
      Serial.println("I2C_ACCESS_COMMAND_UPDATE_SETTINGS");
      break;
    case I2C_ACCESS_COMMAND_READ_PRESET:
      Serial.println("I2C_ACCESS_COMMAND_READ_PRESET");
      break;
    case I2C_ACCESS_COMMAND_STORE_PRESET:
      Serial.println("I2C_ACCESS_COMMAND_STORE_PRESET");
      break;
    case I2C_ACCESS_COMMAND_ERASE_PRESET:
      Serial.println("I2C_ACCESS_COMMAND_ERASE_PRESET");
      break;
    case I2C_ACCESS_COMMAND_VIEW_PRESET:
      Serial.println("I2C_ACCESS_COMMAND_VIEW_PRESET");
      break;
    case I2C_ACCESS_COMMAND_REBOOT:
      Serial.println("I2C_ACCESS_COMMAND_REBOOT");
      break;
    case I2C_ACCESS_COMMAND_SET_DEFAULT:
      Serial.println("I2C_ACCESS_COMMAND_SET_DEFAULT");
      break;
    case OSD_CLOCK_POSITION:
      Serial.println("OSD_CLOCK_POSITION");
      break;
    default:
      Serial.print("Unknown: 0x");
      Serial.println(address, HEX);
      break;
  }
}

uint32_t HWWrite(uint16_t writeaddress, uint8_t* ptr, uint32_t size)
{
  int len=0;
  int x;
  uint32_t addr;
  // Serial.print("Writing some settings to address: ");
  // PrintAddress(writeaddress);
  while (len < size)
  {
    Wire.beginTransmission(I2C_ADDRESS/2);          // transmit to device I2C_ADDRESS
    addr = len + writeaddress;
    Wire.write(addr>>8);
    Wire.write(addr % 256);
    for (x=0; x<126; x++)                           // Max tx size of ESP32
    {
      Wire.write(*(ptr++));
      len++;
      if (len == size) break;;
    }
    Wire.endTransmission();
  }
  return (len);
}


void CommandWait(uint16_t commandaddress)
{
  uint8_t val = 1;
  uint8_t timeout = 10;
  val = 1;
  while(val && timeout) 
  {
    delay(10);
    HWRead(commandaddress, &val, 1);        // Check readyflag
    timeout--;
  }
}

void HWUpdate()
{
  uint8_t val = 1;
  HWWrite(I2C_ACCESS_COMMAND_UPDATE_SETTINGS, &val, 1);
  CommandWait(I2C_ACCESS_COMMAND_UPDATE_SETTINGS);
}

void HWDefaults()
{
  uint8_t val = 1;
    uint8_t timeout = 10;
  HWWrite(I2C_ACCESS_COMMAND_SET_DEFAULT, &val, 1);
  CommandWait(I2C_ACCESS_COMMAND_SET_DEFAULT);
  HWRead(I2C_ACCESS_SETTINGS, (uint8_t *) &settings[0], sizeof(settings[0]));
}

uint32_t HWRead(uint16_t readaddress, uint8_t* ptr, uint32_t size)
{
  uint32_t len=0;

  while (len < size)
  {
    Wire.beginTransmission(I2C_ADDRESS/2);           // transmit to device 0xB0/2
    Wire.write((len+readaddress) / 256);
    Wire.write((len+readaddress) % 256);
    Wire.endTransmission();
    if (Wire.requestFrom(I2C_ADDRESS/2, (size%126)) == 0) break;
    while(Wire.available())
    {
        *(ptr+len) = Wire.read();                   // receive bytes
        len++;
    }
  }
  return (len);
}

