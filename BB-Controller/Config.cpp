//--------------------------------------------------------------------
//	Config.cpp
//
//  (c) PE1MUD, PE1OBW 2024
//--------------------------------------------------------------------
#include "WString.h"
#include "TFT_eSPI.h"
#include <Arduino.h>
#include <stdint.h>
#include <Wire.h>                       
#include "globals.h"
#include "Config.h"
#include "BESSEL0.h"
#include "hansPOV.h"
#include "Support.h"
#include "Presets.h"

#define PUPDATE                         0
#define PACTIVATE                       1
#define PSTORE                          2
#define PDELETE                         3

SETTINGS settings_backup; // placeholder to keep setti

// Create all strings we may want to show, depending on the settings
static const char* const videoSync[8][3] = { "INPUT", "BOARD", "AUTO" };
static const char* const videoPolarity[8][2] = { "POS", "NEG" };
static const char* const videoFilter[8][2] = { "ON", "OFF" };
static const char* const videoMode[8][4] = {"FLAT", "PAL", "NTSC", "SECAM"};
static const char* const osdMode[8][3] = { "OFF", "ON", "AUTO" };
static const char* const fmBandwith[8][4] = { "130k", "180k", "230k", "280k" };
static const char* const audioInput[8][12] = { "IN1L", "IN1R", "IN2L", "IN2R", "EX1L", "EX1R", "EX2L", "EX2R", "IN1LR", "IN2LR", "EX1LR", "EX2LR", "MUTE" };
static const char* const preEmphasis[8][4] = { "50us", "75us", "J17", "FLAT" };
static const char* const carrName[8][5] = { "FM1:", "FM2:", "FM3:", "FM4:" };
static const char* const cwSpeed[8][4] = { "30wpm", "15wpm", "10wpm", "7.5wpm" };
static const char* const besselNul[8][4] = { "OFF", "27MHz" };
static const char* const memoryItem[12][5] = {"Preview", "Load", "Save", "Erase", "Save only!"};

uint16_t confLine[] = {1, 21, 19, 21, 19, 19, 19, 19, 21, 19, 19, 21}; // Determines line to line distance and how wider seperation lines are drawn
uint16_t confItem[] = {0,3, 56, 120, 165, 210, 275}; // x position of items in menu

uint8_t m_items[] = {0, 3, 3, 3, 3, 6, 6, 5, 5, 2, 4, 5, 2}; // lists the number of items for each line

uint8_t nr_lines = sizeof(confLine)/2; // Read the number of lines from confLine

// This one doesn't need to update at higher speeds, so just drawing
TFT_eSprite configLine = TFT_eSprite(&tft);

// String, line, itemIdx, menuIdx
void confString(char* strng, int line, int itemIdx, int menuIdx, int charno = -1)
{
  int i;
  int x;
  int size = 15;
  char c;
  int txtColor = TFT_WHITE;

  if (m_line ==1) size = 11; // Limit edit size of memory name

  if (memory != preview_memory) txtColor = TFT_ORANGE;
  if (!(preset_status & (1<<preview_memory))) txtColor = TFT_DARKGRAY;


  if (m_line==line)
    if (m_item==menuIdx)
    {
      txtColor = (menuMode ? TFT_GREEN : TFT_YELLOW);
      // if (menuMode) txtColor = TFT_GREEN;
    }
    else
      txtColor = 0x2104;

  if (charno > -1 && (line==m_line))
  {
    x = confItem[itemIdx];
    for(i=0; i < size; i++)
    {
      c = strng[i];
      if (!c) c = ' ';
      if (c>96) c -= 32; // uppercase
      sprintf(tempString,"%c",c);
      if (i==charno)
      {
        configLine.setTextColor(TFT_WHITE);
        if (editString == CHAR_POS || editString == CHAR_MOVED)
        {
          sprintf(tempString,"_");
          configLine.drawString(tempString ,x ,2 ,2);
          configLine.setTextColor(txtColor);
        }
        sprintf(tempString,"%c",c);
        x += configLine.drawString(tempString ,x ,2 ,2);
      }
      else
      {
        configLine.setTextColor(txtColor);
        sprintf(tempString,"%c",c);
        x += configLine.drawString(tempString ,x ,2 ,2);
      }
    }
  }
  else
  {
    if (charno<-1) txtColor = TFT_RED; // set to -10 on overlap
    configLine.setTextColor(txtColor);
    configLine.drawString(strng, confItem[itemIdx], 2 , 2);
  }
}

void changeString(char* strng, int size, int pos)
{
  const char morseChars[] = " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"; // List of allowable MORSE CODE characters
  char c;
  int i;
  int found = 0;
  
  // Read char from string
  c = strng[pos];
  Serial.println((uint8_t)c);
  if (c < 1) c = ' ';
  // Find next char in morseChars array
  while (!found)
  {
    c += rot; // Next char!
    Serial.println((uint8_t)c);
    // Check for presence in morseChars
    for (i=0;i<sizeof(morseChars)-1;i++)
      if (c == morseChars[i]) // If this is it, we found our next char, skip for loop and exit while loop
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
  strng[size-1] = c;
  for(i=size-2; i; i--) // reverse order search in string
  {
    c = strng[i];
    if (!c && found) c = ' '; // Already found a printable char, change preceding zeroes to spaces
    if ((c == 0x20) && !found) c = 0; // Found a space, replace by zero
    else if (c) found = 1; // found a printable char, now replace zeroes by spaces
    strng[i] = c; // Store new char in string  
  }
}

void drawConfigMenu(int action)
{
  uint8_t y=2;
  uint8_t i;
  uint8_t line;
  int value;
  int txtColor;
  int extend;
  int speed=0;
  int search_memory;
  char c;
  bool change = false;
  static uint8_t memory_action = 0;
  
  // Serial.println("Entering drawconfigmenu!");
  if (bessel0) action = UPDATE_ITEM;
  if (action == NEXT_ITEM)
  {
    m_item += rot; // Go to next item (or previous)
    if (m_items[m_line] < m_item || m_item < 1 || !m_line) // On overflow go to the next line / and set appropriate item
    {
      // Serial.println("Moving to next item");
      m_line += rot;
      if (m_line < 0) m_line = nr_lines; // wrap it around!
      if (m_line && !m_items[m_line]) m_line += rot; // skip lines with no items
      if (m_line > nr_lines || m_line < 0) m_line = 0; // Get out of menu
      m_item = (rot == 1 ? 1 : m_items[m_line]);
    }
    if (!m_line) menuMode = 0;
    // If MORSE CODE is active, skip the ON/OFF option for NICAM
    if (settings[show_memory].general.audio_nco_mode == MORSECODE && m_line == 11 && m_item == 1) 
      if (rot == 1) m_item++;
      else m_item =m_items[--m_line];
    rot = 0;
  }

  configLine.createSprite(314,18);
  configLine.setSwapBytes(true);
  configLine.loadFont(FONT16);

  if (action == UPDATE_ALL) 
  {
    tft.drawSmoothRoundRect(0, 0, 5, 4, 319, 239, TFT_BLUE, TFT_BLACK, 0xF);
    tft.fillSmoothRoundRect(2,2, 316, 236, 4, FILL1);
  }

  speed = rot;
  // if (ElapsedTime>5 && ElapsedTime < 10) speed *= 3;
  if (ElapsedTime < 2) speed *= 5;
  
  if ((m_line != 9 && m_line != 1) || m_item != 2) editString = CHAR_IDLE; // remove status that we were editing a string/call

  for (line = 0; line <= nr_lines; line++)
  {
    extend = 0;
    if (bessel0) // If bessel is activated, do not draw all lines before line 12!
    {
      // Serial.println("Bessel is active");
      if (line < nr_lines)
      {
        y += confLine[line];
        continue;
      }
    }
    if (line == m_line) // Highlight this line?
    {
      if (menuMode) configLine.fillSprite(0x200); // Color when configuring
      else  configLine.fillSprite(0x000B); // Color when browsing items
    }
    else
      configLine.fillSprite(FILL1);

    switch(line)
    {
      case 1:
        change = CheckChange();
        memory = settings[0].general.last_recalled_presetnr;
        if (m_line != 1 || menuMode == 0)
        {
          show_memory = 0;
          preview_memory = memory;
        }

        if (MODIFY 1)
        {
          preview_memory = constrain(preview_memory + rot, 1, 31);
          LoadPreview(preview_memory);
          show_memory = 2; // show the preview memory
          rot = 0;
        }

        if (TOGGLE 1)
        {
          Serial.println("Running memory");
          Serial.print("memory_action: ");
          // A longpress on running memory saves the changes
          if(CheckChange()==true && (preview_memory == memory))
            SavePreset(memory);
          // A longpress on a different memory loads that different memory, if it's a valid memory
          if ((preview_memory != memory) &&  (preset_status & (1<<preview_memory)))
          {
            memory = preview_memory;
            LoadPreset(memory);
            show_memory = 0;
            settings[0].general.last_recalled_presetnr = memory;
          }
          else 
          {
            memory = preview_memory;
            SavePreset(memory);
            LoadPreset(memory);
            settings[0].general.last_recalled_presetnr = memory;
            show_memory = 0;
          }
        }

        menuItem.setTextColor(TFT_GREEN);
        if (preview_memory == memory)
        {
          show_memory = 0;
          sprintf(tempString, "%sActive M%02d:", (CheckChange()==true ? "*":""), memory);
          if (memory_action != 3) memory_action = PUPDATE;
        }
        else
        {
          if ((preset_status & (1<<preview_memory)))
          {
            sprintf(tempString, "View M%02d:", preview_memory);
            memory_action = PACTIVATE;
          }
          else
          {
            sprintf(tempString, "Empty M%02d:", preview_memory);
            memory_action = PSTORE;
          }
        }
        confString(tempString, line, 1, 1);

        if ((m_line != 9 && m_line !=1) || m_item != 2) editString = CHAR_IDLE;
        if (MODIFY 2)
        {
          Serial.print("Editing pos: ");
          Serial.println(charPos);
          if (editString == CHAR_POS || editString == CHAR_MOVED)
          {
            charPos = constrain(charPos+rot, 0, 10);
            editString = CHAR_MOVED;
          } 
          if (editString == CHAR_EDIT) changeString(settings[show_memory].name, sizeof(settings[show_memory].name), charPos);
          rot = 0;
        }
        if (editString) confString(settings[show_memory].name, line, 3, 2, charPos);
        else
        {
          charPos = -1;
          if (m_item == 1)
            confString(settings[show_memory].name, line, 3, 1); // show the name when on item one, for ease of selection
          else
            confString(settings[show_memory].name, line, 3, 2); // show the name on item two, for editing of the string
        }
        configLine.pushSprite(3, y);


        // Add option to delete the current memory, under conditions
        if (MODIFY 3 && memory_action == PUPDATE && memory != 1)
          memory_action = PDELETE;

        switch (memory_action)
        {
          case PUPDATE:
            sprintf(tempString, "%s", (CheckChange()==true ? "UPDATE":"ACTUAL"), memory); // String depends on change of memory - or not
            break;
          case PACTIVATE:
            sprintf(tempString, "LOAD");
            break;
          case PSTORE:
            sprintf(tempString, "STORE");
            break;
          case PDELETE:
            sprintf(tempString, "DELETE");
            break;
          default:
            sprintf(tempString, "UNKNOWN");
            break;
        }
        if (m_item == 1)
          confString(tempString, line, 5, 1);
        else
          confString(tempString, line, 5, 3);

        // Delete the selected memory
        if (TOGGLE 3)
        {
          if (memory_action == PUPDATE)
          {
            Serial.println("Updating current memory");
            SavePreset(memory);
            y = 2;
            line = 0;
            memory_action = PUPDATE;
            buttonhold++;
            break;
          }
          if (memory_action == PDELETE)
          {
            Serial.println("Deleting current memory");
            ErasePreset(preview_memory);
            memory = 1;
            preview_memory = 1;
            LoadPreset(memory);
            y = 2;
            line = 0;
            memory_action = PUPDATE;
            buttonhold++;
            break;
          }
        }
        configLine.pushSprite(3, y);
        break;
      case 2:
        confString((char *)"VIDEO:", line, 1, 1);
        if (MODIFY 1) settings[show_memory].video.video_mode = (VIDEO_MODE)constrain(settings[show_memory].video.video_mode+rot, 0, 1);
        confString((char *)videoMode[0][settings[show_memory].video.video_mode], line, 2, 1);
        confString((char *)"POL:", line, 3, 2);
        if (MODIFY 2) settings[show_memory].video.invert_video = rot == 1? 0 : 1;
        confString((char *)videoPolarity[0][settings[show_memory].video.invert_video], line, 4, 2);
        confString((char *)"LEVEL:", line, 5, 3);
        if (TOGGLE 3) settings[show_memory].video.enable = 1 - settings[show_memory].video.enable;
        if (settings[show_memory].video.enable)
        {
          value = (int) round(20. * log10((float)settings[show_memory].video.video_level/128.0f));
          if (MODIFY 3)
          {
            value = constrain(value+rot, -15, 0);
            settings[show_memory].video.video_level = (int) round(128. * pow10(((float)value/20.)));
          }
          sprintf(tempString,"%ddB",value+3);
        }
        else
          sprintf(tempString,"OFF");
        confString(tempString, line, 6, 3);
        configLine.pushSprite(3, y);
        break;
      case 3:
        confString((char *)"SYNC:", line, 1, 1);
        if (MODIFY 1) settings[show_memory].video.video_in = (VIDEO_INPUT)constrain(settings[show_memory].video.video_in+rot, 0, 2);
        confString((char *)videoSync[0][settings[show_memory].video.video_in], line, 2, 1);
        confString((char *)"OSD:", line, 3, 2);
        if (MODIFY 2) settings[show_memory].video.osd_mode = (OSD_MODE)constrain(settings[show_memory].video.osd_mode+rot, 0, 2);
        confString((char *)osdMode[0][settings[show_memory].video.osd_mode], line, 4, 2);
        confString((char *)"LPF:", line, 5, 3);
        if (MODIFY 3) settings[show_memory].video.filter_bypass = (OSD_MODE)constrain(settings[show_memory].video.filter_bypass-rot, 0, 1);
        confString((char *)videoFilter[0][settings[show_memory].video.filter_bypass], line, 6, 3);
        configLine.pushSprite(3, y);
        break;
      case 4:
        confString((char *)"NICAM:", line, 1, m_item, carrierOverlap[0] == true ? -2: -1); // -2 signals overlap, -1 all ok
        value = settings[show_memory].nicam.input_ch1;
        if (MODIFY 1) 
        {
          value = constrain(value + 2*rot, 0, 12);
          if (value==8) value = 12;
          if (value==10) value = 6;
          settings[show_memory].nicam.input_ch1 = (AUDIO_INPUT)value;
          settings[show_memory].nicam.input_ch2 = (AUDIO_INPUT)(value+(value<8 ? 1:0));
        }

        // confString(audioInput[0][settings[show_memory].nicam.input], line, 2, 0);
        sprintf(tempString,"%s%s", audioInput[0][settings[show_memory].nicam.input_ch1], settings[show_memory].nicam.input_ch1 < 8? "R":"");
        confString(tempString, line, 2, 1);
        confString((char *)"J17", line, 4, 0); // never highlight on this line
        value = settings[show_memory].nicam.rf_frequency_khz;
        if (MODIFY 2)
        {
          value = (rot == 1 ? 6552:5850);
          settings[show_memory].nicam.rf_frequency_khz = value;
          settings[show_memory].nicam.bandwidth = (value == 5850? BW_500: BW_700);
        }
        sprintf(tempString,"%d.%03dM",value/1000, value%1000);
        confString(tempString, line, 5, 2);
        // The BW is frequency dependant, so update it after
        confString((settings[show_memory].nicam.bandwidth == BW_500) ? (char *)"500k": (char *)"700k", line, 3, 0);
        if (TOGGLE 3) settings[show_memory].nicam.enable = 1 - settings[show_memory].nicam.enable;
        if (settings[show_memory].nicam.enable==true)
        {
          value = settings[show_memory].nicam.rf_level;
          // Serial.print("NICAM RF Level: ");
          // Serial.println(value);
          value = (int) round(20 * log10((float)value/1023.0f));
          if (MODIFY 3) 
          {
            value = constrain(value+rot,-20,0);
            settings[show_memory].nicam.rf_level = (int) round(1023.0f * pow10((float)value/20.0f));
          }
          sprintf(tempString,"%ddB",value);
        }
        else
          sprintf(tempString,"OFF");
        confString(tempString, line, 6, 3);
        configLine.pushSprite(3, y);
        break;
      case 5:
      case 6:
        extend = 1;
      case 7:
      case 8:
        sprintf(tempString, carrName[0][line-5]);
        if (line < 7)
        {
          if (MODIFY 1) settings[show_memory].fm[line-5].am = constrain(settings[show_memory].fm[line-5].am - rot, 0 , 1);
          if(settings[show_memory].fm[line-5].am) tempString[0]= 'A';
        }
        confString(tempString, line, 1, m_item, carrierOverlap[line-4] == true ? -2: -1); // -2 signals overlap, -1 all ok
        if (MODIFY 1 + extend) settings[show_memory].fm[line-5].input = (AUDIO_INPUT)constrain(settings[show_memory].fm[line-5].input + rot, 0, 12);
        confString((char *)audioInput[0][settings[show_memory].fm[line-5].input], line, 2, 1+extend);
        if (MODIFY 2 + extend) settings[show_memory].fm[line-5].bandwidth = (FM_BANDWIDTH)constrain(settings[show_memory].fm[line-5].bandwidth + rot, 0, 3);
        confString((char *)fmBandwith[0][settings[show_memory].fm[line-5].bandwidth], line, 3, 2+extend);
        if (MODIFY 3 + extend) settings[show_memory].fm[line-5].preemphasis = (PREEMPHASIS)constrain(settings[show_memory].fm[line-5].preemphasis + rot, 0, 3);
        confString((char *)preEmphasis[0][settings[show_memory].fm[line-5].preemphasis], line, 4, 3+extend);
        value = settings[show_memory].fm[line-5].rf_frequency_khz;
        if (MODIFY 4 + extend)
        {
          value += 10*speed;
          if (value > 9990) value = 5500;
          if (value < 5500) value = 9900;
          settings[show_memory].fm[line-5].rf_frequency_khz = value;
        }
        sprintf(tempString,"%d.%03dM",value/1000, value%1000);
        confString(tempString, line, 5, 4+extend);

        if (TOGGLE 5 + extend) settings[show_memory].fm[line-5].enable = 1 - settings[show_memory].fm[line-5].enable;
        if (settings[show_memory].fm[line-5].enable==true)
        {
          value = settings[show_memory].fm[line-5].rf_level;
          value = (int) round(20 * log10((float)value/1023.0f));
          if (MODIFY 5 + extend) 
          {
            value = constrain(value+rot,-20,0);
            settings[show_memory].fm[line-5].rf_level = (int) round(1023.0f * pow10((float)value/20.0f));
          }
          sprintf(tempString,"%ddB",value);
        }
        else
          sprintf(tempString,"OFF");
        confString(tempString, line, 6, 5+extend);
        configLine.pushSprite(3, y);
        // Serial.println("Checking overlap!");
        if (checkOverlap() == true) // If the new settings have a difference in overlap, redo the menu with the new colors
        {
          rot = 0;
          buttonhold++;
          y = 2;
          line = 0;
        }
        break;
      case 9:
        settings[show_memory].general.morse_message[15] = 0;
        confString((char *)"MODE:", line, 1, 1);
        if (MODIFY 1) settings[show_memory].general.audio_nco_mode = (NCO_MODE)constrain(settings[show_memory].general.audio_nco_mode + rot, 0, 1);
        confString((char *)(settings[show_memory].general.audio_nco_mode == 0 ? "TONE":"CALL"), line, 2, 1);

        confString((char *)"MSG:", line, 3, 2);
        if ((m_line != 9 && m_line !=1) || m_item != 2) editString = CHAR_IDLE;
        if (MODIFY 2)
        {
          if (editString == CHAR_POS || editString == CHAR_MOVED)
          {
            charPos = constrain(charPos+rot, 0, 14);
            editString = CHAR_MOVED;
          } 
          if (editString == CHAR_EDIT) changeString(settings[show_memory].general.morse_message, sizeof(settings[show_memory].general.morse_message), charPos);
          rot = 0;
        }
        if (editString) confString(settings[show_memory].general.morse_message, line, 4, 2, charPos);
        else
        {
          charPos = -1;
          confString(settings[show_memory].general.morse_message, line, 4, 2);
        }
        configLine.pushSprite(3, y);
        break;
      case 10:
        confString((char *)"TONE:", line, 1, m_item);
        if (MODIFY 1) 
        {
          if (buttonhold>20) 
          {
            value = 1000;
            if (settings[show_memory].general.audio_nco_frequency < 2000) value = 100;
            if (settings[show_memory].general.audio_nco_frequency < 200) value = 10;
            settings[show_memory].general.audio_nco_frequency /= value;
            settings[show_memory].general.audio_nco_frequency *= value;
            settings[show_memory].general.audio_nco_frequency = constrain(settings[show_memory].general.audio_nco_frequency + (rot*value), 20, 15000);
          }
          else settings[show_memory].general.audio_nco_frequency = constrain(settings[show_memory].general.audio_nco_frequency + speed, 20, 15000);
        }
        sprintf(tempString, "%dHz", settings[show_memory].general.audio_nco_frequency);
        confString(tempString, line, 2, 1);
        if (MODIFY 2) settings[show_memory].general.morse_message_repeat_time = constrain(settings[show_memory].general.morse_message_repeat_time + speed, 10, 300);
        sprintf(tempString, "%03ds", settings[show_memory].general.morse_message_repeat_time);
        confString(tempString, line, 4, 2);
        if (MODIFY 3) settings[show_memory].general.morse_speed = constrain(settings[show_memory].general.morse_speed - rot, 0, 3);
        confString((char *)cwSpeed[0][settings[show_memory].general.morse_speed], line, 5, 3);
        if (MODIFY 4) settings[show_memory].fm[0].generator_level = constrain(settings[show_memory].fm[0].generator_level - rot, 0, 8);
        settings[show_memory].nicam.generator_level_ch1 = settings[show_memory].fm[0].generator_level; // Copy to the other channels too...
        settings[show_memory].nicam.generator_level_ch2 = settings[show_memory].fm[0].generator_level;
        settings[show_memory].fm[1].generator_level = settings[show_memory].fm[0].generator_level;
        settings[show_memory].fm[2].generator_level = settings[show_memory].fm[0].generator_level;
        settings[show_memory].fm[3].generator_level = settings[show_memory].fm[0].generator_level;
        sprintf(tempString, "%ddB",(settings[show_memory].fm[0].generator_level) * -6);
        confString(tempString, line, 6, 4);
        configLine.pushSprite(3, y);
        break;
      case 11:
        confString((char *)"ENA:", line, 1, m_item);
        if (settings[show_memory].general.audio_nco_mode != MORSECODE)
        {
          value = settings[show_memory].nicam.generator_ena_ch1 + 2 * settings[show_memory].nicam.generator_ena_ch2;
          if (MODIFY 1) value = constrain(value+rot, 0, 3);
          switch (value)
          {
            case 0:
              sprintf(tempString, "OFF");
              settings[show_memory].nicam.generator_ena_ch1 = 0;
              settings[show_memory].nicam.generator_ena_ch2 = 0;
              break;
            case 1:
              sprintf(tempString, "NIC-L");
              settings[show_memory].nicam.generator_ena_ch1 = 1;
              settings[show_memory].nicam.generator_ena_ch2 = 0;
              break;
            case 2:
              sprintf(tempString, "NIC-R");
              settings[show_memory].nicam.generator_ena_ch1 = 0;
              settings[show_memory].nicam.generator_ena_ch2 = 1;
              break;
            case 3:
              sprintf(tempString, "NIC-L&R");
              settings[show_memory].nicam.generator_ena_ch1 = 1;
              settings[show_memory].nicam.generator_ena_ch2 = 1;
              break;
            default:
              sprintf(tempString, "OFF");
              settings[show_memory].nicam.generator_ena_ch1 = 0;
              settings[show_memory].nicam.generator_ena_ch2 = 0;
              break;
          }
        }
        else 
        {
          sprintf(tempString, "OFF!");
          settings[show_memory].nicam.generator_ena_ch1 = 0;
          settings[show_memory].nicam.generator_ena_ch2 = 0;
        }
        confString(tempString, line, 2, 1);

        if (MODIFY 2) settings[show_memory].fm[0].generator_ena = constrain(settings[show_memory].fm[0].generator_ena + rot, 0, 1);
        if (MODIFY 3) settings[show_memory].fm[1].generator_ena = constrain(settings[show_memory].fm[1].generator_ena + rot, 0, 1);
        if (MODIFY 4) settings[show_memory].fm[2].generator_ena = constrain(settings[show_memory].fm[2].generator_ena + rot, 0, 1);
        if (MODIFY 5) settings[show_memory].fm[3].generator_ena = constrain(settings[show_memory].fm[3].generator_ena + rot, 0, 1);

        confString((settings[show_memory].fm[0].generator_ena == true)? (char *)(settings[show_memory].fm[0].am == 1 ? "AM1":"FM1"):(char *)"OFF", line, 3, 2);
        confString((settings[show_memory].fm[1].generator_ena == true)? (char *)(settings[show_memory].fm[1].am == 1 ? "AM2":"FM2"):(char *)"OFF", line, 4, 3);
        confString((settings[show_memory].fm[2].generator_ena == true)? (char *)"FM3":(char *)"OFF", line, 5, 4);
        confString((settings[show_memory].fm[3].generator_ena == true)? (char *)"FM4":(char *)"OFF", line, 6, 5);
        configLine.pushSprite(3, y);
        break;
      case 12:
        confString((char *)"Bessel null:", line, 1, 1);
        if (MODIFY 1)
        {
          if (bessel0 < 1 && rot == 1)
          {
            if (bessel0 == 0)
            {
              if (serial== 10040) tft.pushImage(3,3,314,213,HansPOV);
              else tft.pushImage(3,3,314,213,BESSEL0);
              // Copy live settings into preview area
              memcpy((void *)&settings[2], (void *)&settings[0], sizeof(settings[0]));
              settings[0].video.enable = 0;
              settings[0].nicam.enable = 0;
              settings[0].fm[0].am = 0;
              settings[0].fm[0].generator_ena = 0;
              settings[0].fm[0].enable = 1;
              settings[0].fm[1].enable = 0;
              settings[0].fm[2].enable = 0;
              settings[0].fm[3].enable = 0;
              settings[0].fm[0].input = MUTE;
              settings[0].fm[0].rf_frequency_khz = 1580;
              settings[0].fm[0].rf_level = (int) round(1023.0f * pow10((float)-11/20.0f));
            }
            bessel0++;
          }
          else
          {
            if (bessel0 > 0 && rot == -1)
            {
              bessel0--;
              if (bessel0 == 0)
              {
                tft.fillSmoothRoundRect(2, 2, 316, y-5, 4, FILL1);
                // Copy live settings back from preview area
                memcpy((void *)&settings[0], (void *)&settings[2], sizeof(settings[2]));
                line=0;
                y=2;
                action = 0;
                rot = 0;
              }
            }
          }
        }
        confString((char *)besselNul[0][bessel0], line, 3, 1);

        if (!bessel0) confString((char *)"*LOAD DEFAULTS*", line, 4, 2);
        if (TOGGLE 2) 
        {
          configLine.fillSprite(0x200);
          confString((char *)"***LOADING!***", line, 4, 2);
          HWDefaults();
          HWRead(I2C_ACCESS_INFO, (uint8_t *) &info, sizeof(info));
          configLine.pushSprite(3, y);
          buttonhold++;
          y = 2;
          line = 0;
        }
        else configLine.pushSprite(3, y);
        break;
      default:
        break;
    }
    if (line < 12)
    {
        if (confLine[line]==21)
        {
          tft.drawLine(0, y + 18 ,319, y + 18 ,TFT_BLUE);
          tft.drawLine(0, y + 19 ,319, y + 19 ,TFT_BLUE);
        }
        else
          tft.drawLine(2, y + 18 ,317, y + 18 ,TFT_NAVY);
    }
    else
    {
      // Something changed, so rerun to indicate the * next to the memory (as appropriate)
      if (change != CheckChange()) 
      {
        change = CheckChange();
        y = 2;
        line = 0;
        rot = 0;                                 // No two rotary actions
        if (buttonhold == HOLDING) buttonhold++; // prevent double click on toggle
      }
    }
    y += confLine[line];
  }
  rot=0;
  speed=0;
}