//--------------------------------------------------------------------
//	FpgaFlash.h
//
//  (c) PE1MUD, PE1OBW 2024
//--------------------------------------------------------------------
#include "M25P80.h"
bool ReadM25P80(uint8_t, uint32_t, uint8_t*, uint16_t);
bool WriteM25P80(uint8_t, uint32_t, uint8_t*, uint16_t);
void  EraseSectorM25P80(uint32_t sector);