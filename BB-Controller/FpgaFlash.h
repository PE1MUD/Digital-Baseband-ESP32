//--------------------------------------------------------------------
//	FpgaFlash.h
//
//  (c) PE1MUD, PE1OBW 2024
//--------------------------------------------------------------------
#include "M25P80.h"
bool ReadM25P80(uint8_t, uint32_t, uint8_t*, uint16_t);
bool WriteM25P80(uint8_t, uint32_t, uint8_t*, uint16_t);
void  EraseSectorM25P80(uint32_t sector);


#define GP  0x107   /* x^8 + x^2 + x + 1 */
#define DI  0x07


static unsigned char crc8_table[256];     /* 8-bit table */
static int table=0;

//      Call first
static void init_crc8()
{
  int i,j;
  unsigned char crc;

  if (!table) {
    for (i=0; i<256; i++) {
      crc = i;
      for (j=0; j<8; j++)
        crc = (crc << 1) ^ ((crc & 0x80) ? DI : 0);
      crc8_table[i] = crc & 0xFF;
      // printf("table[%d] = %d (0x%X)\n", i, crc, crc);
    }
    table=1;
  }
}