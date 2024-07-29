//--------------------------------------------------------------------
//	MSP25P80.h
//
//  (c) PE1MUD, PE1OBW 2024
//--------------------------------------------------------------------
#ifndef M25P80_H
#define M25P80_H

// Defining only those commands we need

#define WRITE_ENABLE					    0x06
#define WRITE_DISABLE					    0x04
#define READ_IDENTIFICATION				0x9F
#define READ_STATUS_REGISTER			0x05
#define WRITE_STATUS_REGISTER			0x01
#define READ_DATA_BYTES					  0x03
#define PAGE_PROGRAM					    0x02
#define SECTOR_ERASE					    0xD8
#define BULK_ERASE						    0xC7
#define WRITE_IN_PROGRESS         1

#define BASE                      8             // Base sector
#define BLOCK                     256           // Block write size

#endif