// #include "sys_arch.h"
//--------------------------------------------------------------------
//	TestCardMenu.cpp
//
//  (c) PE1MUD, PE1OBW 2024
//--------------------------------------------------------------------
#include "WString.h"
#include "TFT_eSPI.h"
#include <Arduino.h>
#include <stdint.h>
#include <Wire.h>                       
#include "globals.h"
#include "TestcardMenu.h"
#include "Support.h"

#include "hw_control_registers.h"       // FPGA control registers
#include "baseband_i2c_interface.h"     // FPGA I2C interface

static const char* const lineName[20][13] = {  "Disabled", "Staircase",  "Multiburst CCIR18",  "Ramp",  "2T/20T CCIR17", "16:9 anamorph", "16:9 show 4:3", "16:9 show top part", "undef",  "undef",  "undef",  "undef" ,  "undef" };
static const char* const testCard[30][13] = {  "Disabled", "Staircase, Ramp, 2T/20T, Multiburst", "Staircase, Multiburst",  "Ramp only",  "Multiburst only",  "Staircase only",  "2T/20T only",  "-",  "-",  "-",  "-",  "-",  "-" };

uint16_t tcs_Line[] = {1, 21, 19, 19, 19, 21, 21, 19, 19, 21, 19, 19}; // Determines line to line distance and how wider seperation lines are drawn
uint16_t tcs_Item[] = {0, 3, 50, 70, 170, 210, 230}; // x position of items in menu

uint8_t tcs_items[] = {0, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 3, 1}; // lists the number of items for each line

uint8_t nr_tcs_lines = sizeof(tcs_Line)/2; // Read the number of lines from tcsLine

// This one doesn't need to update at higher speeds, so just drawing
TFT_eSprite tcsLine = TFT_eSprite(&tft);

void tcsString(char* strng, int line, int itemIdx, int menuIdx, int charno = -1)
{
  int i;
  int x;
  char c;
  int txtColor = TFT_WHITE;
  
  if (m_line==line)
    if (m_item==menuIdx)
    {
      txtColor = (menuMode ? TFT_GREEN : TFT_YELLOW);
      // if (menuMode) txtColor = TFT_GREEN;
    }
    else
      txtColor = 0x2104;

  if (charno > -1)
  {
    x = tcs_Item[itemIdx];
    strng[sizeof(esp32nvm.osdsettings[0].osd)-1] = 0;
    for(i=0; i < sizeof(esp32nvm.osdsettings[0].osd)-1; i++)
    {
      c = strng[i];
      if (!c) c = ' ';
      // if (c>96) c -= 32; // uppercase
      sprintf(tempString,"%c",c);
      if (i==charno)
      {
        tcsLine.setTextColor(TFT_WHITE);
        if (editString == CHAR_POS || editString == CHAR_MOVED)
        {
          sprintf(tempString,"_");
          tcsLine.drawString(tempString ,x ,2 ,2);
          tcsLine.setTextColor(txtColor);
        }
        sprintf(tempString,"%c",c);
        x += tcsLine.drawString(tempString ,x ,2 ,2);
      }
      else
      {
        tcsLine.setTextColor(txtColor);
        sprintf(tempString,"%c",c);
        x += tcsLine.drawString(tempString ,x ,2 ,2);
      }
    }
  }
  else
  {
    // if (charno<-1) txtColor = TFT_RED; // set to -10 on overlap
    tcsLine.setTextColor(txtColor);
    tcsLine.drawString(strng, tcs_Item[itemIdx], 2 , 2);
  }
}

void changeosdString(char* strng, int pos)
{
  // const char osdChars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-+,.// \0"; // List of allowable OSD characters
  const char osdChars[] =  " !#$&()*+,-.//0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]abcdefghijklmnopqrstuvwxyz"; // List of allowable OSD characters
  char c;
  int i;
  int found = 0;
  
  // Read char from string
  c = strng[pos];
  if (c < 1) c = ' ';
  // Find next char in morseChars array
  while (!found)
  {
    c += rot; // Next char!
    // Check for presence in morseChars
    for (i=0; i<sizeof(osdChars)-1; i++)
      if (c == osdChars[i]) // If this is it, we found our next char, skip for loop and exit while loop
      {
        found = 1;
        break;
      }
  }

  // Store new char in string  
  strng[pos] = c;

  // Replace trailing spaces by 0's and midway 0's by spaces, if any
  found = 0;
  c = 0;
  strng[sizeof(esp32nvm.osdsettings[0].osd)-1] = c;
  for(i=sizeof(esp32nvm.osdsettings[0].osd)-2; i; i--) // reverse order search in string
  {
    c = strng[i];
    if (!c && found) c = 0x20; // Already found a printable char, change preceding zeroes to spaces
    if ((c == 0x20) && !found) c = 0; // Found a space, replace by zero
    else if (c) found = 1; // found a printable char, now replace zeroes by spaces
    strng[i] = c; // Store new char in string  
  }
}

void fillLines(int start, int stop, uint8_t fill) // beware not to overfill!
{
  int num;
  if (stop > 310) stop = 310;
  if (start < 24) start = 24;
  for (num=start; num<stop; num++)
  {
    testLines[num-1] = fill;
    testLines[num-1+312] = fill;
  }
}

void drawTC(uint8_t tcnum)
{
  uint8_t i;
  switch (tcnum)
  {
    case 0: // disabled; remove all references from visible area
      fillLines(24, 310, 0);
      break;
    case 1:
      fillLines(24, 141, STAIRCASE);
      fillLines(141, 201, RAMP);
      fillLines(201, 251, CCIR17);
      fillLines(251, 310, MULTIBURST);
      break;
    case 2:
      fillLines(24, 166, STAIRCASE);
      fillLines(166, 310, MULTIBURST);
      break;
    case 3:
      fillLines(24, 310, RAMP);
      break;
    case 4:
      fillLines(24, 310, MULTIBURST);
      break;
    case 5:
      fillLines(24, 310, STAIRCASE);
      break;
    case 6:
      fillLines(24, 310, CCIR17);
      break;
    default:  
      break;
  }
  HWWrite(I2C_ACCESS_PATTERN_MEMORY, (uint8_t *) &testLines, 625);
}

void writeUserOSD(int i)
{
  int value;
  int x;
  uint8_t inverted;
  
  if (i != settings[0].video.show_menu || i == -1)
  {
    if (i==-1) i = settings[0].video.show_menu; // if init, use current value of show_menu
    else
    {
      settings[0].video.show_menu = i;
      HWWrite(I2C_ACCESS_SETTINGS, (uint8_t *) &settings[0], sizeof(settings[0]));
      HWUpdate();
    }
    if (i == 0)
    {
      for (value=0; value< 640; value++) OSDbuffer[value] = 0x00;                         // Clear the OSD buffer
      if (esp32nvm.osdsettings[0].enable)
      {
        for (value=0;value<sizeof(esp32nvm.osdsettings[0].osd)-1;value++) if (!esp32nvm.osdsettings[0].osd[value]) break;
        inverted = 128 * esp32nvm.osdsettings[0].inverted;
        for (x=0; x < value; x++)
          OSDbuffer[(x + esp32nvm.osdsettings[0].x - 1)%40 + (esp32nvm.osdsettings[0].y - 1) * 40] = esp32nvm.osdsettings[0].osd[x] + inverted;
      }
      if (esp32nvm.osdsettings[1].enable)
      {
        for (value=0;value<sizeof(esp32nvm.osdsettings[1].osd)-1;value++) if (!esp32nvm.osdsettings[1].osd[value]) break;
        inverted = 128 * esp32nvm.osdsettings[1].inverted;
        for (x=0; x < value; x++)
          OSDbuffer[(x + esp32nvm.osdsettings[1].x - 1)%40 + (esp32nvm.osdsettings[1].y - 1) * 40] = esp32nvm.osdsettings[1].osd[x] + inverted;
      }
      HWWrite(I2C_ACCESS_DISPLAY , (uint8_t *)&OSDbuffer, 40*16);                         // Write the OSD
    }
  }
}

void drawTCSMenu(int action)
{
  uint8_t y=2;
  uint8_t i;
  uint8_t line;
  int value;
  int txtColor;
  int speed=0;
  char c;
  
  // Serial.println("Entering testcard screen menu!");
  if (action == NEXT_ITEM)
  {
    m_item += rot; // Go to next item (or previous)
    if (tcs_items[m_line] < m_item || m_item < 1 || !m_line) // On overflow go to the next line / and set appropriate item
    {
      // Serial.println("Moving to next item");
      m_line += rot;
      if (m_line < 0) m_line = nr_tcs_lines; // wrap it around!
      if (m_line && !tcs_items[m_line]) m_line += rot; // skip lines with no items
      if (m_line > nr_tcs_lines || m_line < 0) m_line = 0; // Get out of menu
      m_item = (rot == 1 ? 1 : tcs_items[m_line]);
    }
    if (!m_line) menuMode = 0;
    rot = 0;
  }

  tcsLine.createSprite(314,18);
  tcsLine.setSwapBytes(true);
  tcsLine.loadFont(FONT16);

  if (action == UPDATE_ALL) 
  {
    tft.drawSmoothRoundRect(0, 0, 5, 4, 319, 239, TFT_BLUE, TFT_BLACK, 0xF);
    tft.fillSmoothRoundRect(2,2, 316, 236, 4, FILL1);
  }

  speed = rot;

  if (ElapsedTime < 2) speed *= 5;

  if ((m_line != 7 && m_line != 10) || m_item > 1) editString = CHAR_IDLE; // remove status that we were editing a string/call

  for (line = 0; line <= nr_tcs_lines; line++)
  {
    if (line == m_line) // Highlight this line?
    {
      if (menuMode) tcsLine.fillSprite(0x200); // Color when configuring
      else  tcsLine.fillSprite(0x000B); // Color when browsing items
    }
    else
      tcsLine.fillSprite(FILL1);

    switch(line)
    {
      case 1:
        if (MODIFY 1)
        {
          settings[0].video.pattern_enable = (VIDEO_MODE)constrain(settings[0].video.pattern_enable+rot, 0, 1);
          HWWrite(I2C_ACCESS_SETTINGS, (uint8_t *) &settings[0], sizeof(settings[0]));
          HWUpdate();
        }
        tcsString((char *)"Testcard/line inserter:", line, 1, 1);
        tcsString((char *)(settings[0].video.pattern_enable? "Enabled":"Disabled"), line, 4, 1);
        tcsLine.pushSprite(3, y);
        break;
      case 2:
        if (MODIFY 1) 
        {
          testLines[16] = constrain(testLines[16] + (rot*20), 0, 140);
          HWWrite(I2C_ACCESS_PATTERN_MEMORY, (uint8_t *) &testLines, 625);
          esp32nvm.tclsettings.vits17_line = testLines[16];
          eepTimeout = 20;
        }
        tcsString((char *)"VITS Line 17 (2T/20T):", line, 1, 1);
        tcsString((char *)lineName[0][testLines[16]/20], line, 4, 1);
        tcsLine.pushSprite(3, y);
        break;        
      case 3:
        if (MODIFY 1)
        {
          testLines[17] = constrain(testLines[17] + (rot*20), 0, 140);
          HWWrite(I2C_ACCESS_PATTERN_MEMORY, (uint8_t *) &testLines, 625);
          esp32nvm.tclsettings.vits18_line = testLines[17];
          eepTimeout = 20;
        }
        tcsString((char *)"VITS Line 18 (Multiburst):", line, 1, 1);
        tcsString((char *)lineName[0][testLines[17]/20], line, 4, 1);
        tcsLine.pushSprite(3, y);
        break;
      case 4:
        if (MODIFY 1)
        {
          testLines[22] = constrain(testLines[22] + (rot*20), 0, 140);
          HWWrite(I2C_ACCESS_PATTERN_MEMORY, (uint8_t *) &testLines, 625);
          esp32nvm.tclsettings.wss_line = testLines[22];
          eepTimeout = 20;
        }
        tcsString((char *)"WSS Line 23 (Disabled):", line, 1, 1);
        tcsString((char *)lineName[0][testLines[22]/20], line, 4, 1);
        tcsLine.pushSprite(3, y);
        break;
      case 5:
        i = esp32nvm.tclsettings.testcard;
        if (i > NUMTESTCARDS) i=0;
        if (MODIFY 1)
        {
          i = constrain(i + rot, 0, NUMTESTCARDS);
          esp32nvm.tclsettings.testcard = i;
          eepTimeout = 20;
          drawTC(i);
        }
        tcsString((char *)"Testcard:", line, 1, 1);
        tcsString((char *)testCard[0][i], line, 3, 1);
        tcsLine.pushSprite(3, y);
        break;

      case 6:
        i = settings[0].video.show_menu;
        if (MODIFY 1)
        {
          if (i != constrain(i - rot, 0, 1))
          {
            i = constrain(i - rot, 0, 1);
            writeUserOSD(i);
          }
        }
        tcsString((char *)"OSD (disables menu):", line, 1, 1);
        tcsString((char *) (i==0 ? "Enabled":"Disabled"), line, 4, 1);
        tcsLine.pushSprite(3, y);
        break;
      case 7:
        tcsString((char *)"OSD1:", line, 1, 1);
        if ((m_line != 7 && m_line !=10) || m_item != 1) editString = CHAR_IDLE;
        if (MODIFY 1)
        {
          if (editString == CHAR_POS || editString == CHAR_MOVED)
          {
            charPos = constrain(charPos+rot, 0, sizeof(esp32nvm.osdsettings[0].osd)-2);
            editString = CHAR_MOVED;
          }
          if (editString == CHAR_EDIT) 
          {
            changeosdString(esp32nvm.osdsettings[0].osd, charPos);
            eepTimeout = 20;
            writeUserOSD(-1);
          }
          rot = 0;
        }
        if (TOGGLE 1)
        {
          if (editString == CHAR_EDIT && charPos > 0) 
          {
            // Serial.printf("CLearing remainder of OSD 2");
            ClearOSD(0, charPos); // clear remainder of string on longpress
          }
        }
        if (m_line == 7)
        {
          if (editString) tcsString(esp32nvm.osdsettings[0].osd, line, 2, 1, charPos);
          else
          {
            charPos = -1;
            tcsString(esp32nvm.osdsettings[0].osd, line, 2, 1);
          }
        }
        else 
          tcsString(esp32nvm.osdsettings[0].osd, line, 2, 1);
        tcsLine.pushSprite(3, y);
        break;         
      case 8:
        if (MODIFY 1)
        {
          esp32nvm.osdsettings[0].x = constrain(esp32nvm.osdsettings[0].x+rot, 1, 40);
          eepTimeout = 20;
          writeUserOSD(-1);
        }
        if (MODIFY 2)
        {
          esp32nvm.osdsettings[0].y = constrain(esp32nvm.osdsettings[0].y+rot, 1, 16);
          eepTimeout = 20;
          writeUserOSD(-1);
        }
        if (MODIFY 3)
        {
          esp32nvm.osdsettings[0].inverted = constrain(esp32nvm.osdsettings[0].inverted+rot,0,1);
          eepTimeout = 20;
          writeUserOSD(-1);
        }
        sprintf(tempString, "X-pos: %d", esp32nvm.osdsettings[0].x);
        tcsString(tempString, line, 1, 1);
        sprintf(tempString, "Y-pos: %d", esp32nvm.osdsettings[0].y);
        tcsString(tempString, line, 3, 2);
        sprintf(tempString, "Inverted: %s", esp32nvm.osdsettings[0].inverted ? "Yes":"No");
        tcsString(tempString, line, 4, 3);
        tcsLine.pushSprite(3, y);
        break;      
      case 9:
        if (MODIFY 1)
        {
          esp32nvm.osdsettings[0].enable = constrain(esp32nvm.osdsettings[0].enable+rot,0,1);
          writeUserOSD(-1);
        }
        sprintf(tempString, "Enable OSD1: %s", esp32nvm.osdsettings[0].enable ? "Yes":"No");
        tcsString(tempString, line, 1, 1);
        tcsLine.pushSprite(3, y);
        break;      
      case 10:
        tcsString((char *)"OSD2:", line, 1, 1);
        if ((m_line != 7 && m_line !=10) || m_item != 1) editString = CHAR_IDLE;
        if (MODIFY 1)
        {
          if (editString == CHAR_POS || editString == CHAR_MOVED)
          {
            charPos = constrain(charPos+rot, 0, sizeof(esp32nvm.osdsettings[1].osd)-2);
            editString = CHAR_MOVED;
          }
          if (editString == CHAR_EDIT)
          {
            changeosdString(esp32nvm.osdsettings[1].osd, charPos);
            eepTimeout = 20;
            writeUserOSD(-1);
          }
          rot = 0;
        }
        if (TOGGLE 1)
        {
          if (editString == CHAR_EDIT && charPos > 0) 
          {
            // Serial.printf("CLearing remainder of OSD 2");
            ClearOSD(1, charPos); // clear remainder of string on longpress
          }
        }
        if (m_line == 10)
        {
          if (editString  && m_line == 10) tcsString(esp32nvm.osdsettings[1].osd, line, 2, 1, charPos);
          else
          {
            charPos = -1;
            tcsString(esp32nvm.osdsettings[1].osd, line, 2, 1);
          }
        }
        else
          tcsString(esp32nvm.osdsettings[1].osd, line, 2, 1);
        // tcsString(osdsettings[1].osd, line, 2, 1);
        tcsLine.pushSprite(3, y);
        break;
      case 11:
        if (MODIFY 1)
        {
          esp32nvm.osdsettings[1].x = constrain(esp32nvm.osdsettings[1].x+rot,1,40);
          eepTimeout = 20;
          writeUserOSD(-1);
        }
        if (MODIFY 2)
        {
          esp32nvm.osdsettings[1].y = constrain(esp32nvm.osdsettings[1].y+rot,1,16);
          eepTimeout = 20;
          writeUserOSD(-1);
        }
        if (MODIFY 3)
        {
          esp32nvm.osdsettings[1].inverted = constrain(esp32nvm.osdsettings[1].inverted+rot,0,1);
          eepTimeout = 20;
          writeUserOSD(-1);
        }
        sprintf(tempString, "X-pos: %d", esp32nvm.osdsettings[1].x);
        tcsString(tempString, line, 1, 1);
        sprintf(tempString, "Y-pos: %d", esp32nvm.osdsettings[1].y);
        tcsString(tempString, line, 3, 2);
        sprintf(tempString, "Inverted: %s", esp32nvm.osdsettings[1].inverted ? "Yes":"No");
        tcsString(tempString, line, 4, 3);
        tcsLine.pushSprite(3, y);
        break; 
      case 12:
        if (MODIFY 1)
        {
          esp32nvm.osdsettings[1].enable = constrain(esp32nvm.osdsettings[1].enable+rot, 0, 1);
          writeUserOSD(-1);
        }
        sprintf(tempString, "Enable OSD2: %s", esp32nvm.osdsettings[1].enable ? "Yes":"No");
        tcsString(tempString, line, 1, 1);
        tcsLine.pushSprite(3, y);
        break;      
      default:
        break;
    }
    if (line < 12)
    {
        if (tcs_Line[line]==21)
        {
          tft.drawLine(0, y + 18 ,319, y + 18 ,TFT_BLUE);
          tft.drawLine(0, y + 19 ,319, y + 19 ,TFT_BLUE);
        }
        else
          tft.drawLine(2, y + 18 ,317, y + 18 ,TFT_NAVY);

    }
    y += tcs_Line[line];
  }
  rot=0;
  speed=0;
}