//--------------------------------------------------------------------
//	MainMenu.cpp
//
//  (c) PE1MUD, PE1OBW 2024
//--------------------------------------------------------------------
#include "WString.h"
#include "TFT_eSPI.h"
#include <Arduino.h>
#include <stdint.h>
#include <Wire.h>                       
#include "globals.h"
#include "mainmenu.h"
#include "BESSEL0.h"
#include "ppm.h"                        // The True PPM logo
#include "support.h"
#include "presets.h"

static const char* const mainMenuItem[20][9] = {  "", "M",  "Baseband metering",  "Video ",  "Nicam: ",  "FM1: ",  "FM2: ",  "FM3: ",  "FM4: " };

// static const char* const MenuMode[8][3] = { "RUN ", "RUN*", "VIEW"};

int mainMenuItem_Ypos[] = {0, 3, 28, 58, 88, 118, 148, 178, 208};

int frame_x[4]={FRAME_X, FRAME_X + FRAME_D, FRAME_X + 2 * FRAME_D, FRAME_X + 3 * FRAME_D};

int mm_items[9] = {0, 1, 0, 2, 2, 2, 2, 2, 2};

int dBticks[9] = {0, 3, 6,  9, 12,  16,  21,  25,  29};
int dBval[9]   = {6, 3, 0, -3, -6, -10, -20, -40, -60};
int subticks[10] = {1, 2, 4, 5, 7, 8, 10, 11, 23, 27};

void drawMeter(int n)
{
  #define STARTX 180
  #define ENDX 314
  #define OVL 60
  int i, h, x1, x2, x3, x4, vu;
  int color1, color2, color3, color4, color5, color6;
  x2=0;

  // Get the height at which the meter must be drawn
  h = mainMenuItem_Ypos[n+1] + 21;

  switch (n)
  {
    case 1: // Baseband level meter
      color1 = TFT_DARKGRAY;
      color3 = TFT_DARKGRAY;
      x1 = constrain(hw_inputs.dac_out_min, 0, 4095);
      x2 = constrain(hw_inputs.dac_out_max, 0, 4095);

      if (x1 < 10 || x2 > 4085) // If BB DAC (almost) clips, color accordingly
        color2 = TFT_RED;
      else
        color2 = TFT_GREEN;
      // if (hw_inputs.baseband_clip != 0) color2 = TFT_RED; // also use clip indicator
      x1 = map(x1, 0, 4095, STARTX, ENDX);
      x2 = map(x2, 0, 4095, STARTX, ENDX);
      break;
    case 2: // Video ADC level
      color1 = TFT_DARKGRAY;
      color2 = TFT_DARKGRAY;
      color3 = TFT_DARKGRAY;
      x1 = constrain(hw_inputs.adc_in_min, 0, 1023);
      x2 = constrain(hw_inputs.adc_in_max, 0, 1023);
      while (x1>x2)
      {
        x1--;
        x2++;
      }
      if (x1 < 10 || x2>1013) // If video ADC (almost) clips, color accordingly
        color2 = TFT_RED;
      else
        color2 = TFT_GREEN;
      // if (hw_inputs.vid_adc_clip == 1) color2 = TFT_RED;
      x1 = map(x1, 0, 1023, STARTX, ENDX);
      x2 = map(x2, 0, 1023, STARTX, ENDX);
      break;
    case 3:
      color4 = TFT_DARKGRAY;
      color5 = TFT_DARKGRAY;
      color6 = TFT_RED;
      x4 = hw_inputs.nicam_right_peak  & 0xFF00;
      x4 = int(20.0 * log10(((float)x4)/256.0));
      x4 = map(x4, 0, 25, 0, 50);
    case 4:
    case 5:
    case 6:
    case 7:
      color1 = TFT_DARKGRAY;
      color2 = TFT_DARKGRAY;
      color3 = TFT_RED;
      // x1 = 0;
      if (n==3) // NICAM Left
      {
        x2 = hw_inputs.nicam_left_peak & 0xFF00;
        x2 = int(20.0 * log10(((float)x2)/256.0));//(float)nicamclip));
        x2 = map(x2, 0, 25, 0, 50);
      }
      else
      {
        if (n==4)
          x2 = hw_inputs.fm1_audio_peak & 0xFF00;
        if (n==5)
          x2 = hw_inputs.fm2_audio_peak & 0xFF00;
        if (n==6)
          x2 = hw_inputs.fm3_audio_peak & 0xFF00;
        if (n==7)
          x2 = hw_inputs.fm4_audio_peak & 0xFF00;
        if (x2==0) x2 = 256;
        vu = int(20.0 * log10(((float)x2)/256.0));
        vu = map(vu, 0, 25, 0, 50);
      }
      break;
    default:
      return;
      break;
  }

  if (n==3)
  {
    vu = x2;
    tft.fillRect(180, h, ENDX-STARTX, 3, color1);
    tft.fillRect(STARTX, h, vu, 3, TFT_GREEN);
    if (vu > 15) tft.fillRect(STARTX+15, h, vu-15, 3, TFT_ORANGE);
    if (vu > 31) 
    {
      vu -= 31;
      vu = constrain(vu, 0, 26); // set the end somewhere - heavy clip already
      vu = map(vu, 0, 26, 0, ENDX-STARTX-31);
      tft.fillRect(STARTX+31, h, vu, 3, TFT_RED);
    }
    h+=4;
    vu = x4;
    tft.fillRect(180, h, ENDX-STARTX, 3, color1);
    tft.fillRect(STARTX, h, vu, 3, TFT_GREEN);
    if (vu > 15) tft.fillRect(STARTX+15, h, vu-15, 3, TFT_ORANGE);
    if (vu > 31) 
    {
      vu -= 31;
      vu = constrain(vu, 0, 26); // set the end somewhere - heavy clip already
      vu = map(vu, 0, 26, 0, ENDX-STARTX-31);
      tft.fillRect(STARTX+31, h, vu, 3, TFT_RED);
    }
  }
  else
  {
    if (n>3 && n < 8) 
    {
      tft.fillRect(180, h, ENDX-STARTX, 7, color1);
      tft.fillRect(STARTX, h, vu, 7, TFT_GREEN);
      if (vu > 15) tft.fillRect(STARTX+15, h, vu-15, 7, TFT_ORANGE);
      if (vu > 31) 
      {
        vu -= 31;
        vu = constrain(vu, 0, 26); // set the end somewhere - heavy clip already
        vu = map(vu, 0, 26, 0, ENDX-STARTX-31);
        tft.fillRect(STARTX+31, h, vu, 7, TFT_RED);
      }
    }
    else
    {
      // Constrain vaules to avoid meter overruns and graphical mayhem
      // Used for bb and video level
      // x1 = constrain(x1, STARTX, ENDX);
      // x2 = constrain(x2, STARTX, ENDX);
      if (x1>STARTX) tft.fillRect(STARTX, h, x1-STARTX, 7, color1);
      tft.fillRect(x1, h, x2-x1, 7, color2);
      if(x2<ENDX) tft.fillRect(x2, h, ENDX-x2, 7, color3);
    }
  }
}

void computePPM(int chnl, int ppm)
{
  float ref, logval;
  int intval;
  int pixel;

  ref = float(ppm)/float(headroom);
  logval = 20*log10(ref);

  if (logval>6.0f || peakHold[chnl] == -1)
  {
    if (peakHold[chnl] != -1) peakDrop[chnl] = 1000;
    peakHold[chnl] = -1;
    sprintf(tempString,"OVL");
    ppmValue.setTextColor(TFT_RED);
  }
  else
  {
    ppmValue.setTextColor(FONT);
    if (logval > -10.0f)
      dtostrf( logval, 3, 1, tempString );
    else
      dtostrf( logval, 3, 0, tempString );
      if (logval<-70.0f) sprintf(tempString,"-inf");
  }

  if (logval > 6.0f) logval = 6.0f; // limit range
  
  logval *= 10.0f;

  intval = int(logval);

  if(intval<-600) // lower than -60 db is 10dB per line, calculate number of lines
  {
    intval = intval / -100;
    pixel = 9 - intval;
  }
  else
  {
   if (intval < -199) // up to -20 range, 24 pixels represent a range of 20dB
    {
      //
      pixel = map(intval, -600, -200, 3, 51);
    }
   else
   {
    if(intval < -99) // from -20 to -10, it's 30 pixels per 10dB
    {
      pixel = map(intval, -200, -100, 51, 81);
    }
    else
    { // from -9,9 dB and up, it's 6 pixels per dB
      pixel = map(intval, -100, 60, 81, 177);
    }
   }
  }
  pixel = 177 - pixel;

  ppmBar.fillSprite(TFT_BLACK);

  if (pixel < 36) // going into red)
  {
    ppmBar.fillRect(0, pixel, FRAME_W-2*BAR_SPACE, FRAME_MID-BAR_SPACE, TFT_RED);
    ppmBar.fillRect(0, 36, FRAME_W-2*BAR_SPACE, FRAME_H-2*BAR_SPACE, TFT_GREEN);
  }
  else
    ppmBar.fillRect(0, pixel, FRAME_W-2*BAR_SPACE, FRAME_H-2*BAR_SPACE, TFT_GREEN);

  ppmBar.setSwapBytes(false);
  ppmTicks.pushToSprite(&ppmBar,0,0,TFT_TRANSPARENT);
  ppmBar.setSwapBytes(true);

  // Draw the peak indicator, if needed
  if ((pixel < peakHold[chnl]) || (logval < -700.0f) || (peakHold[chnl] == -1))
  {
    peakHold[chnl] = pixel;
    peakDrop[chnl] = 500; // Initially, hold for 1/2 second
    if (logval)
    ppmValue.fillSprite(FILL1);
    ppmValue.drawString(tempString,FRAME_D,0,2);
    ppmValue.pushSprite(frame_x[chnl]-12,FRAME_Y-16);
  }

  if (peakHold[chnl] < 177)
    if(peakHold[chnl] < pixel)
    {
      if(peakHold[chnl] < 36)
      {
        if(peakHold[chnl] == -1)
        {
          ppmBar.drawLine(0, 0, FRAME_W-2*BAR_SPACE, 0, 0xfc30);
          ppmBar.drawLine(0, 1, FRAME_W-2*BAR_SPACE, 1, 0xfc30);
        }
        else
        {
          ppmBar.drawLine(0, peakHold[chnl]+0, FRAME_W-2*BAR_SPACE, peakHold[chnl], 0xfc30);
          ppmBar.drawLine(0, peakHold[chnl]+1, FRAME_W-2*BAR_SPACE, peakHold[chnl]+1, 0xfc30);
        }
      }
      else
      {
        ppmBar.drawLine(0, peakHold[chnl]+0, FRAME_W-2*BAR_SPACE, peakHold[chnl], TFT_GREEN);
        ppmBar.drawLine(0, peakHold[chnl]+1, FRAME_W-2*BAR_SPACE, peakHold[chnl]+1, TFT_GREEN);
      }
    }

  ppmBar.pushSprite(frame_x[chnl]+BAR_SPACE, FRAME_Y+BAR_SPACE);
}

int is_tick(int test)
{
  for (int i = 0; i < 9; i++)
  {
    if (dBticks[i] == test)
      return i;
  }
  return -1;
}

boolean is_subtick(int test)
{
  for (int i = 0; i < 10; i++)
  {
    if (subticks[i] == test)
      return true;
  }
  return false;
}

int mainMenuString(char* strng, int line, int x, int menuIdx, bool overlap = false)
{
  int txtColor = TFT_WHITE;
  
  if (memory != preview_memory) txtColor = TFT_ORANGE;
  
  if (m_line==line)
    if (m_item==menuIdx)
    {
      txtColor = (menuMode ? TFT_GREEN : TFT_YELLOW);
      if (menuMode) txtColor = TFT_GREEN;
    }
    else
      txtColor = 0x2104;
    if (overlap == true) txtColor = TFT_RED;              // Override on overlap
    menuItem.setTextColor(txtColor);
    return menuItem.drawString(strng, x, 2, 2);
}

void drawMenuItems(int action) // enum ALL, NEXT, UPDATE, TOGGLE, PREV
{
  int /*old,*/ line, x, y;
  int txtColor;
  int frequency;
  int status;
  int value;
  int width=0;
  int temp;
  int search_memory;
  bool change = CheckChange();

  if (action == NEXT_ITEM) // next item in the menu. Can also reverse (depends on value in 'rot')
  {
    m_item += rot; // Go to next item (or previous)
    if (mm_items[m_line] < m_item || m_item < 1 || !m_line) // On overflow go to the next line / and set appropriate item
    {
      m_line += rot;
      if (m_line < 0) m_line = 8; // wrap it around!
      if (m_line && !mm_items[m_line]) m_line += rot; // skip lines with no items
      if (m_line > 8 || m_line < 0) m_line = 0; // Get out of menu
      m_item = (rot == 1 ? 1 : mm_items[m_line]);
    }
    if (!m_line) menuMode = 0;
    rot = 0;
  }

  for (line = 1; line < 9; line++)
  {
    // if (!((action == UPDATE_ALL || buttonhold == HOLDING) && m_line == line)) continue; // no need to touch this item
    if (line == m_line) // Highlight this line?
    {
      if (menuMode) menuItem.fillSprite(0x200); // Color when configuring
      else  menuItem.fillSprite(0x000B); // Color when browsing items
    }
    else
      menuItem.fillSprite(FILL1);
    x = 2;
    switch (line)
    {
      case 1: // MEMORY line
        // Serial.print("menuMode: ");
        // Serial.println(menuMode);
        memory = settings[0].general.last_recalled_presetnr;
        // Check if it's needed to back to memory mode.
        if (m_line != 1 || menuMode == 0)
        {
          show_memory = 0;
          // Serial.println("Show running memory!");
          preview_memory = memory;
        }
        if (MODIFY 1)
        {
          search_memory = preview_memory;
          search_memory = constrain(search_memory + rot, 1, 31);
          while (!(preset_status & (1<<search_memory)) && search_memory < 32 && search_memory > 0) search_memory += rot; 
          if ((search_memory < 32 && search_memory > 0) && (preset_status & (1<<search_memory))) // channel is valid?
          {
            preview_memory = search_memory;
            LoadPreview(preview_memory);
            show_memory = 2; // show the preview memory
            // Serial.print("Loading preview memory: ");
            // Serial.println(preview_memory);
          }
          rot = 0;
        }
        // if the settings changed, longpress will save them!
        if (TOGGLE 1)
        {
          // A longpress on running memory saves the changes
          if(CheckChange()==true && (preview_memory == memory))
          {
            // Serial.println("Saving memory!");
            SavePreset(memory);
          }
          // A longpress on a different memory loads that different memory
          if (preview_memory != memory)
          {
            // Serial.println("Loading memory!");
            memory = preview_memory;
            LoadPreset(memory);
            show_memory = 0;
            settings[0].general.last_recalled_presetnr = memory;
          }
        }
        menuItem.setTextColor(TFT_GREEN);
        // settings[0].name[10] = 0; // make sure we have no more than 10 characters
        if (preview_memory == memory)
        {
          // Serial.println("Running memory");
          show_memory = 0;
          sprintf(tempString, "%sM%02d: %s", (CheckChange()==true ? "*":""), memory, settings[show_memory].name);
        }
        else
        {
          // Serial.print("Viewing memory: ");
          // Serial.println(preview_memory);
          sprintf(tempString, "V%02d: %s", preview_memory, settings[show_memory].name);
        }
        mainMenuString(tempString, line, x, 1);
        break;
      case 2:
        mainMenuString((char *)"Baseband metering", line, 2, 1);
        value = 0;
        break;
      case 3: // Video pos/neg line
        x += mainMenuString((char *)mainMenuItem[0][line], line, x, m_item);
        if (MODIFY 1) settings[show_memory].video.invert_video = rot == 1? 0 : 1;
        x += mainMenuString(settings[show_memory].video.invert_video ? (char *)"negative":(char *)"positive", line, x, 1);
        value = (int) round(20. * log10((float)settings[show_memory].video.video_level/128.0f));
        if (MODIFY 2)
        {
          value = constrain(value+rot, -15, 0);
          settings[show_memory].video.video_level = (int) round(128. * pow10(((float)value)/20.));
        }
        if (TOGGLE 2) settings[show_memory].video.enable = settings[show_memory].video.enable ? false:true;
        if (settings[show_memory].video.enable == true) sprintf(tempString, "%2ddB", value + 3);
        else sprintf(tempString, "OFF");
        width = mainMenuString(tempString, 2 ,200, 1); // print off-sprite, to find the text width
        mainMenuString(tempString, line, 134 - width, 2);
        break;
      case 4:
        x += mainMenuString((char *)"Nicam: ", 4, x, m_item, carrierOverlap[0]);
        if (MODIFY 1) settings[show_memory].nicam.rf_frequency_khz = rot == 1 ? 6552: 5850;
        x += mainMenuString(settings[show_memory].nicam.rf_frequency_khz == 5850 ? (char *)"5.850": (char *)"6.552", 4, x, 1);
        settings[show_memory].nicam.bandwidth = settings[show_memory].nicam.rf_frequency_khz == 5850 ?  BW_500 : BW_700;
        if (TOGGLE 2) settings[show_memory].nicam.enable = settings[show_memory].nicam.enable == true ? false : true;
        value = (int) round(20 * log10((float)settings[show_memory].nicam.rf_level/1023.0f));
        if (MODIFY 2) 
        {
          value = constrain(value+rot, -20, 0);
          settings[show_memory].nicam.rf_level = (int) round(1023.0f * pow10((float)value/20.0f));
        }
        if (settings[show_memory].nicam.enable == true) sprintf(tempString, "%2ddB", value);
        else sprintf(tempString, "OFF");
        width = mainMenuString(tempString, 4, 200, 1); // print off-sprite, to find the text width
        mainMenuString(tempString, 4, 134 - width , 2);
        break;
      case 5:
      case 6:
      case 7:
      case 8:
        sprintf(tempString, "FM%d: ",line-4);
        if (line < 7 && settings[show_memory].fm[line-5].am) tempString[0] = 'A';
        x += mainMenuString(tempString, line, x ,m_item, carrierOverlap[line-4]); // fm[0] starts at index 1 so line-4 not line-5
        frequency = settings[show_memory].fm[line-5].rf_frequency_khz;
        if (MODIFY 1)
          {
            frequency += 10 * rot;
            frequency /=10;
            frequency *=10;
            if (frequency > 9990) frequency = 5500;
            if (frequency < 5500) frequency = 9990;
            settings[show_memory].fm[line-5].rf_frequency_khz = frequency;
          }
        sprintf(tempString, "%d.%02dMHz",frequency/1000, (frequency%1000)/10);
        x += mainMenuString(tempString, line, x, 1);
        if (TOGGLE 2) settings[show_memory].fm[line-5].enable = settings[show_memory].fm[line-5].enable ? false : true;
        value = (int) round(20 * log10((float)settings[show_memory].fm[line-5].rf_level/1023.0f));
        if (MODIFY 2)
          {
            value = constrain(value+rot, -20, 0);
            settings[show_memory].fm[line-5].rf_level = (int) round(1023.0f * pow10((float)value/20.0f));
          }
        if (settings[show_memory].fm[line-5].enable == true) sprintf(tempString, "%2ddB", value);
        else sprintf(tempString, "OFF");
        width = mainMenuString(tempString, 2, 200 ,2); // print off-sprite, to find the text width
        mainMenuString(tempString, line, 134 - width , 2);
      default:
        break;
    }
    menuItem.pushSprite(178, mainMenuItem_Ypos[line]+3);
   
    if ((checkOverlap() == true) || (change != CheckChange())) // If the new settings have a difference in overlap, redo the menu with the new colors
    {
      // Serial.println("Some overlap changed!");
      change = CheckChange();
      rot = 0;
      buttonhold++;
      line = 0;
    }

  }
  rot = 0;
}

void drawMainMenu()
{
  int line, y;
  ppmWindow.fillSprite(TFT_BLACK); // clear sprite
  ppmWindow.drawSmoothRoundRect(0, 0, 5, 4, 145, 239, TFT_BLUE, TFT_BLACK, 0xF);
  ppmWindow.fillSmoothRoundRect(2,2, 141, 236, 4, FILL1);
  for (line=1; line<8; line++)  // Put the 8 menu items on screen
  {
    y = mainMenuItem_Ypos[line+1];
    if (line == 1) 
      ppmWindow.drawLine(1, y-1, 175, y-1, TFT_BLUE);
    ppmWindow.drawLine(1, y, 175, y, TFT_BLUE);
  }
  ppmWindow.pushSprite(174,0);
  drawMenuItems(UPDATE_ALL);
}

void drawMain(void)
{
  int x,y,i,seg;
  int index, temp;

  screen = MAIN;

  tft.fillScreen(TFT_BLACK);

  ppmHolder.setSwapBytes(true);
  ppmHolder.fillSprite(FILL1);
  ppmHolder.setTextColor(FONT);

  ppmBar.setSwapBytes(true);
  ppmBar.fillSprite(TFT_BLACK);

  ppmValue.setSwapBytes(true);
  ppmValue.fillSprite(TFT_BLACK);

  ppmValue.setTextColor(FONT);
  ppmValue.setTextDatum(TR_DATUM);

  ppmTicks.setSwapBytes(true);
  ppmTicks.fillSprite(TFT_TRANSPARENT);

  ppmWindow.setSwapBytes(true);
  ppmWindow.unloadFont();
  ppmWindow.setTextColor(FONT);
  ppmWindow.loadFont(FONT16);
  ppmWindow.fillSprite(TFT_BLACK);

  menuItem.unloadFont();
  menuItem.loadFont(FONT16);
  menuItem.fillSprite(FRAMECOLOR);

  // Blue outer rectangle with FILL1 internal
  ppmWindow.drawSmoothRoundRect(0, 0, 5, 4, 171, 239, TFT_BLUE, TFT_BLACK, 0xF);
  ppmWindow.fillSmoothRoundRect(2,2, 167, 236, 4, FILL1);

  // Top PPM outline
  ppmHolder.fillRoundRect(1 ,1, FRAME_W - 2, FRAME_MID - 2, 4, TFT_BLACK);
  ppmHolder.drawSmoothRoundRect(0 ,0, 5, 4, FRAME_W - 1, FRAME_MID - 0,FRAMECOLOR, TFT_BLACK, 0xf);

  // Bottom PPM outline
  ppmHolder.fillRoundRect(1 ,FRAME_MID + 1, FRAME_W-2, FRAME_H - FRAME_MID - 2, 4, TFT_BLACK);
  ppmHolder.drawSmoothRoundRect(0 ,FRAME_MID, 5, 4, FRAME_W-1, FRAME_H - FRAME_MID - 1, FRAMECOLOR, TFT_BLACK, 0xf);

  // Put True PPM in the upper left corner.
  ppmWindow.pushImage(4,4,31,27,PPM31x27);

  // Put the darkred and darkreen reference bars on screen
  ppmWindow.fillRect(FRAME_X-8, FRAME_Y + BAR_SPACE, 4, FRAME_MID - BAR_SPACE, 0xa9a6);
  ppmWindow.fillRect(FRAME_X-8, FRAME_Y + FRAME_MID, 4, FRAME_H - FRAME_MID - BAR_SPACE, 0x3566);

  // Make ticks and subticks
  for(y=0; y<32; y++)
  {
    temp = is_tick(y);
    if (temp != -1)
    {
      ppmWindow.drawLine(FRAME_X-12, FRAME_Y + BAR_SPACE + y * 6, FRAME_X+50, FRAME_Y + BAR_SPACE + y * 6,FRAMECOLOR);
      ppmWindow.drawLine(FRAME_X+70, FRAME_Y + BAR_SPACE + y * 6, FRAME_X+120, FRAME_Y + BAR_SPACE + y * 6,FRAMECOLOR);
      ppmTicks.drawLine(1, y * 6, FRAME_W-2*BAR_SPACE-2, y * 6, FRAMECOLOR);
      sprintf(tempString,"%2d",dBval[temp]);
      x = ppmWindow.drawString(tempString, 320,240, 2) + 13;
      ppmWindow.drawString(tempString, FRAME_X-x, FRAME_Y + BAR_SPACE + (y * 6) - 7, 2);
    }

    if (is_subtick(y))
    {
      ppmWindow.drawLine(FRAME_X-12, FRAME_Y + BAR_SPACE + y * 6, FRAME_X-4, FRAME_Y + BAR_SPACE + y * 6,FRAMECOLOR);
      ppmTicks.drawLine(3, y * 6, FRAME_W-2*BAR_SPACE-4, y * 6, FRAMECOLOR);
    }
  }

  ppmWindow.drawString("L-IN1-R", FRAME_X + 3, 9, 2);
  ppmWindow.drawSmoothRoundRect(FRAME_X-2, 5, 5, 4, FRAME_X - 2 + (1* FRAME_W), 21, 0x7bef, FILL1);

  ppmWindow.drawString("L-IN2-R", FRAME_X + 3 + 2 * FRAME_D, 9, 2);
  ppmWindow.drawSmoothRoundRect(FRAME_X -2 + 2 * FRAME_D, 5, 5, 4, FRAME_X - 2 + (1* FRAME_W), 21, 0x7bef, FILL1);

  ppmWindow.pushSprite(0,0);

  for (i=0; i<4; i++)
    ppmHolder.pushSprite(frame_x[i], FRAME_Y);

  drawMainMenu();
}
