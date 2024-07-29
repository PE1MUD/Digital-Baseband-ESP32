//--------------------------------------------------------------------
// Baseband i2c interface control.
//
// Write data to baseband
// ----------------------
// <start> <slaveaddr> <addr_hi> <addr_low> <data> ... <stop>
//
//
// Read data from baseband
// -----------------------
// <start> <slaveaddr> <addr_hi> <addr_low> <start> <slaveaddr|1> <read(ACK)> ... <read(NAK)> <stop>
//
//
// Using commands
// --------------
// To issue a command, write one of the I2C_ACCESS_COMMAND_xxx addresses (2 bytes), followed by at
// least one 'value' byte which must be > 0.
// To check if the command has been executed, the command value can be read back, it is set to 0
// after the command has been executed. This is necessary for most commands.
//
// For example, to 'view' the contents of a preset, write the I2C_ACCESS_COMMAND_VIEW_PRESET command,
// followed by the preset number.
// After that, read one byte from I2C_ACCESS_COMMAND_VIEW_PRESET in a loop until it returns 0.
// The preset contents can then be read using I2C_ACCESS_VIEW_SETTINGS.
//
// Accessing the M25P80 flash chip
// -------------------------------
// First write the I2C_ACCESS_FLASH base address, where the offset (bits 11..0) contain the count of
// bytes to read and/or write.
// Then continue writing and/or reading this count of bytes, which are redirected to the flash.
//
// ** IMPORTANT **: make sure the byte count is correct, because the CSn of the flash goes low
// on the first byte, but only goes high after this count has been read and/or written!
//
// Example: to read the flash ID register, issue the following i2c commands:
// <start> 0xB0		the baseband's slave address
// 0x70 0x04		Address the I2C_ACCESS_FLASH, we'll transfer 4 bytes: 1 write and 3 reads
// 0x9F 			write to FLASH: READ_IDENTIFICATION
// <start> 0xB1		repeated start and baseband slave address
// <read(ACK)		read manufacturer ID from flash (0x20)
// <read(ACK)		read memory type from flash (0x20)
// <read(NAK)		read capacity from flash (0x14)
// <stop>
//
//
// (c) PE1OBW, PE1MUD
//--------------------------------------------------------------------

#ifndef BASEBAND_I2C_INTERFACE_H_
#define BASEBAND_I2C_INTERFACE_H_

#define I2C_ADDRESS	0xB0

typedef struct {
	uint8_t	hw_version;
	uint8_t fpga_version;
	uint8_t sw_version_minor;
	uint8_t sw_version_major;
} INFO;

// Base addresses
#define I2C_ACCESS_DISPLAY					0x0000		// R/W, maps to display memory, 40 columns x 16 rows = 640 bytes
#define I2C_ACCESS_FONT_MEMORY				0x0800		// R/W, maps to font memory, 128 characters, each 8x16 pixels = 2048 bytes
#define I2C_ACCESS_SETTINGS					0x1000		// R/W, maps to SETTINGS
#define I2C_ACCESS_READOUT					0x2000		// RO   maps to HW_INPUTS
#define I2C_ACCESS_COMMAND					0x3000		// R/W  maps to commands, read gives status (0=done), no auto address increment!
#define I2C_ACCESS_VIEW_SETTINGS			0x4000		// RO	maps to SETTINGS preview (= the result from COMMAND_VIEW_PRESET)
#define I2C_ACCESS_READ_PRESET_STATUS		0x5000		// RO	maps to PRESET_FLAGS, 32 bits (4 bytes), bit=1 indicates if a preset is used
#define I2C_ACCESS_INFO						0x6000		// RO	maps to INFO
#define I2C_ACCESS_FLASH					0x7000		// R/W	maps to flash SPI interface, see description in file header
#define I2C_ACCESS_PATTERN_MEMORY			0x8000		// R/W, maps to pattern memory (8192 bytes)
#define I2C_ACCESS_IO_REGISTERS				0xA000		// R/W,	maps to hardware IO registers
#define I2C_ACCESS_ROUTING_REGISTERS		0xC000		// WO,  maps to hardware routing register

// Commands. To execute, write a byte (> 0) to the address. To retrieve status, read from this address
#define I2C_ACCESS_COMMAND_UPDATE_SETTINGS	0x3000		// <1>   	   Update hardware registers (activate settings)
#define I2C_ACCESS_COMMAND_READ_PRESET		0x3001		// <preset nr> Read config from preset 1..31 and activate it
#define I2C_ACCESS_COMMAND_STORE_PRESET		0x3002		// <preset nr> Store current config in preset 1..31
#define I2C_ACCESS_COMMAND_ERASE_PRESET		0x3003		// <preset nr> Erase current config in preset 1..31
#define I2C_ACCESS_COMMAND_VIEW_PRESET		0x3004		// <preset nr> Read config from preset 1..31 and copy to 'preview' settings
#define I2C_ACCESS_COMMAND_REBOOT           0x3005		// <1> 		   Reboot FPGA board after 500ms delay
#define I2C_ACCESS_COMMAND_SET_DEFAULT		0x3006		// <1>         Load defaults

#endif /* BASEBAND_I2C_INTERFACE_H_ */
