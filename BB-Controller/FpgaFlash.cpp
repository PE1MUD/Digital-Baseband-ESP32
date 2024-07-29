//--------------------------------------------------------------------
//	FpgaFlash.cpp
//
//  (c) PE1MUD, PE1OBW 2024
//--------------------------------------------------------------------
#include <Arduino.h>
#include <stdint.h>
#include <Wire.h>                       

#include "globals.h"
#include "M25P80.h"
#include "hw_control_registers.h"       // FPGA control registers
#include "baseband_i2c_interface.h"     // FPGA I2C interface

bool ReadM25P80(uint8_t command, uint32_t address, uint8_t* buffer, uint16_t num)
{
  int extra = 0;
  extra = (command == READ_DATA_BYTES || command == PAGE_PROGRAM || command == SECTOR_ERASE) ? 3:0;
  Wire.beginTransmission(I2C_ADDRESS/2);                          // transmit to device 0xB0/2 
  if(Wire.write((I2C_ACCESS_FLASH + 1 + num + extra) / 256))      // Compound address, see the baseband_i2c_interface.h header
  {
    Wire.write((I2C_ACCESS_FLASH + 1 + num + extra) & 255);
    Wire.write(command);                                          // Transmit the command
    if (extra > 0)                                                  
    {
      Wire.write((address>>16) & 255);
      Wire.write((address>>8) & 255);
      Wire.write(address & 255);
    }
    if (num > 0)
    {
      Wire.endTransmission(false);
      Wire.requestFrom(I2C_ADDRESS/2, num);                         // Ask I2C to read num bytes
      while(Wire.available()) *buffer++ = Wire.read();              // Get the data to the buffer
    }
    return true;
  }
  return false;
}

bool WriteM25P80(uint8_t command, uint32_t address, uint8_t* buffer, uint16_t num)
{
  int extra = 0;
  extra = (command == READ_DATA_BYTES || command == PAGE_PROGRAM || command == SECTOR_ERASE) ? 3:0;
  Wire.beginTransmission(I2C_ADDRESS/2);                            // transmit to device 0xB0/2 
  if (Wire.write((I2C_ACCESS_FLASH + 1 + num + extra) / 256))       // Compound address, see the baseband_i2c_interface.h header
  {
    Wire.write((I2C_ACCESS_FLASH + 1 + num + extra) & 255);
    Wire.write(command);                                            // Transmit the command
    if (extra > 0)
    {
      Wire.write((address>>16) & 255);
      Wire.write((address>>8) & 255);
      Wire.write(address & 255);
    }    
    if (num > 0) while(num--) Wire.write(*buffer++);                // Write the data in the buffer
    Wire.endTransmission();
    return true;
  }
  Wire.endTransmission();
  return false;
}

void  EraseSectorM25P80(uint32_t sector)
{
  uint8_t readstat;

  WriteM25P80(WRITE_ENABLE, 0, 0, 0);
  WriteM25P80(SECTOR_ERASE, sector, 0, 0);
  ReadM25P80(READ_STATUS_REGISTER, 0, &readstat, 1);
  while(readstat & WRITE_IN_PROGRESS) ReadM25P80(READ_STATUS_REGISTER, 0, &readstat, 1);
}