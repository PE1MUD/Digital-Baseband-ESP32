//--------------------------------------------------------------------
//	support.cpp - misc functions
//
//  (c) PE1MUD, PE1OBW 2024
//--------------------------------------------------------------------
#include <Arduino.h>
#include <stdint.h>
#include "globals.h"

bool CheckChange()
{
  return(memcmp((void *)&settings[0], (void *)&settings[1], sizeof(settings[0])));
}

void ClearOSD(uint8_t osdnum, uint8_t from)
{
  char *p_osd;
  p_osd = (char *)&esp32nvm.osdsettings[osdnum].osd; // Get the pointer to the string
  for (; from < sizeof(esp32nvm.osdsettings[osdnum].osd); from++) p_osd[from] = 0; // Clear (remainder) of the string
}

bool checkOverlap() // Check if carriers overlap
{
  int i,j;
  int fmbw[] = {131,181,231,281}; // add 15kHz to bandwidth....
  int bw[5];
  int freq[5];
  bool ena[5];
  bool ovl[5];

  ena[0] = settings[0].nicam.enable;
  ena[1] = settings[0].fm[0].enable;
  ena[2] = settings[0].fm[1].enable;
  ena[3] = settings[0].fm[2].enable;
  ena[4] = settings[0].fm[3].enable;

  // Read frequencies and bandwidth so we can do calculations with them and see if there is overlap
  freq[0] = settings[0].nicam.rf_frequency_khz;
  freq[1] = settings[0].fm[0].rf_frequency_khz;
  freq[2] = settings[0].fm[1].rf_frequency_khz;
  freq[3] = settings[0].fm[2].rf_frequency_khz;
  freq[4] = settings[0].fm[3].rf_frequency_khz;

  bw[0] = (settings[0].nicam.bandwidth == BW_500) ? 500: 700;
  bw[1] = settings[0].fm[0].am == 1 ? 32 : fmbw[settings[0].fm[0].bandwidth]; // in case of AM assume a BW of 32kHz ie 2 * 15kHz plus a wee bit
  bw[2] = settings[0].fm[1].am == 1 ? 32 : fmbw[settings[0].fm[1].bandwidth]; // in case of AM assume a BW of 32kHz ie 2 * 15kHz plus a wee bit
  bw[3] = fmbw[settings[0].fm[2].bandwidth];
  bw[4] = fmbw[settings[0].fm[3].bandwidth];

  for (i=0;i<5;i++)
  {
    ovl[i] = carrierOverlap[i];                // Remember state so we can compare later
    carrierOverlap[i] = false;
  }

  for (i=0;i<5;i++)
  {
    if (!ena[i]) continue;
    for (j=0;j<5;j++)
    {
      if (!ena[j]) continue;
      if (i==j) continue;
      if (freq[i] == freq[j]) // Check for equal frequencies
      {
        carrierOverlap[i] = true;
        carrierOverlap[j] = true;
      }
      if (freq[i] < freq[j])
      {
        if ((freq[i] + (bw[i]/2)) >= (freq[j] - (bw[j]/2)))
        {
        carrierOverlap[i] = true;
        carrierOverlap[j] = true;
        }
      }
      if (freq[i] > freq[j])
      {
        if ((freq[i] - (bw[i]/2)) <= (freq[j] + (bw[j]/2)))
        {
        carrierOverlap[i] = true;
        carrierOverlap[j] = true;
        }
      }
    }
  }
  for (i=0;i<5;i++) if (ovl[i] != carrierOverlap[i]) return true;
  return false;
}
