//---------------------------------------------------------------------------------------
//	BB-Controller.ino
//
//  (c) PE1MUD, PE1OBW 2024
//---------------------------------------------------------------------------------------
//  For all graphics functions the following library needs to be installed:
//
//  https://github.com/Bodmer/TFT_eSPI
//
//  The version this was built and tested with is 2.5.43
//
//  No other libraries that aren't part of the Arduino dev environment
//  have been used.
//
//  Note: some settings need to be changed that are part of the TFT_eSPI library!
//
//  Edit User_Setup.h in user\Documents\Arduino\libraries\TFT_eSPI
//  to use the following pins for the display:
//
//  #define TFT_MOSI 23
//  #define TFT_SCLK 18
//  #define TFT_CS    5  // Chip select control pin
//  #define TFT_DC   17  // Data Command control pin
//  #define TFT_RST  16  // Reset pin (could connect to RST pin)
//
//  The speed needs a change too, so you will have silky smooth PPM Meters.
//  For this, select ONLY this line:
//
//  #define SPI_FREQUENCY  55000000 // STM32 SPI1 only (SPI2 maximum is 27MHz)
//
//  Finally, to stop all the whining about not having a touch screen, uncomment:
//
//  #define DISABLE_ALL_LIBRARY_WARNINGS
//---------------------------------------------------------------------------------------
//  Configuring Arduino for the ESP32 that is in use:
//
//  Tools->Boards-> (search for) esp32 -> "ESP32 Dev Module" and select this.
//
//  The settings for this board:
//
//  Frequency: "240MHz"
//  Core Debug Level: "None"
//  Erase All Flash....: Disabled
//  Events Run On: "Core1"
//  Flash Frequency: "80MHz"
//  Flash Mode: "QIO"
//  Flash Size: "4MB (32Mb)"
//  JTAG Adapter: "Disabled"
//  Arduino Runs On: "Core1"
//  Partition Scheme: "Huge APP (3MB no OTA/1MB SPIFFS)"
//  PSRAM: "Disabled"
//  Upload Speed: "921600"
//
//  This disables the option of OTA updates, but provides 3MB of flash!
//  Indeed, much memory is needed!
//---------------------------------------------------------------------------------------
//  IMPORTANT NOTICE
//
//  You're allowed to use all associated sources, but if and when you
//  modify anything, you are required to disclose your modifications
//  in a project (branch). This project should be complete in a way 
//  that it can be compiled. Should you change anything major 
//  hardware-wise you must provide adequate details on your 
//  modifications in a read.me or by any other appropriate method.
//
//  If you find bugs, errors, inconsistencies, please provide feedback
//  to David PE1MUD, or Werner PE1OBW.
//
//  Do not remove headers or copyright notices.
//---------------------------------------------------------------------------------------
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <Wire.h>
#include <EEPROM.h>
#include "globals.h"
#include "config.h"
#include "mainmenu.h"
#include "testcardmenu.h"
#include "stdbool.h"
#include "eep.h"
#include "Presets.h"
#include "Support.h"

#define BUTTON !digitalRead(ROTARY_BUTTON)
#define NICAMMUTE !digitalRead(NICAM_MUTE_PIN)

//
// https://github.com/espressif/arduino-esp32/blob/c8215315aef8a11529743dd403f2e894350ad547/libraries/Wire/src/Wire.h#L34
//
// Despite Arduino's documentation claim, the ESP32 wire buffer size is 128 bytes, not 32 bytes, using an override
//

#include "top.h"

#include "FpgaFlash.h"
#include "version.h"
#include "digibbt.h"                    // Our startup screen

#define LEDPIN          2               // To drive the TFT's LED
#define INVERT          15              // When shorted to gnd, invert rotary direction

#define NO              -1
#define YES             1

// Use hw timer
hw_timer_t *Timer0_Cfg = NULL;

// Measure rotary speed
unsigned long StartTime = millis();
unsigned long CurrentTime = millis();
unsigned long ElapsedTime = 0;
int bessel0 = 0; // Turn off bessel null by default
int menuMode = 0; // 0 when not in a menu, 1 when in a menu, 2 when in an item in a menu

// These are used everywhere. Do not move them to the MainMenu.cpp as hopping back and forth will create and destroy them for each update
// Performance IS an issue here so we want these to be global!
TFT_eSPI tft = TFT_eSPI(240, 320);
TFT_eSprite ppmWindow= TFT_eSprite(&tft);
TFT_eSprite ppmHolder= TFT_eSprite(&tft);
TFT_eSprite ppmBar= TFT_eSprite(&tft);
TFT_eSprite ppmTicks= TFT_eSprite(&tft);
TFT_eSprite infoSprite= TFT_eSprite(&tft);
TFT_eSprite ppmValue = TFT_eSprite(&tft);
TFT_eSprite menuItem = TFT_eSprite(&tft);

int ppmUpdate = 0;                          // update the PPM when this hits zero (only from main screen)
int countSecond = 0;                        // Keep track of seconds
int peakHold[4] = {0,0,0,0};                // Peak pixel value
float peakHolddB[4] = {-90,-90,-90,-90};    // Peak dB value for texts above peakmeters

int  peakDrop[4] = {0,0,0,0};                // peak Drop timers
bool carrierOverlap[5] = {false,false,false,false,false};

int headroom, fmclip, nicamclip;

int brightness = 20;                        // Initial brightness - need to move it to eeprom?

char freetext[] = "*** PE1OBW en PE1MUD ***";
uint8_t buffer[2048];                       // Used in I2C stuff
char OSDbuffer[640];                        // Used for the OSD

uint16_t serial = 0;

char tempString[60];                        // Used in sprintf's

uint8_t testLines[625];
bool nicmute = 0;

// used to keep track of memory modes and so on
ESP32NVM esp32nvm;

int rot = 0;
int speed = 0;
int position = 3;
uint8_t memory;
uint8_t preview_memory;
uint8_t show_memory;
uint32_t preset_status;

// FPGA related variables
HW_INPUTS hw_inputs;
SETTINGS settings[3];                       // 0 = live 1 = memory 2 = preview
INFO info;

int rotary;
byte rotarymode = 0;                        // no reverse
byte optenc = 0;                            // no optical encoder

int screen=BOOT;                            // keeps track of where we are
int m_line = 0;                             // Menu line #, 0 is none!
int m_item = 0;                             // Menu item # on a line
int menu_action = MENU_IDLE;                // Tracks menu navigation changes
int buttonhold = 0;                         // Used in timer to see if button is held to change screen
int buttonPress = 0;                        // Used to keep track if button is pressed

int charPos = -1;                           // character position of call being edited
int editString = CHAR_IDLE;                 // Keep track if string is being edited, set to idle. Is handled in config.cpp

int dir = 1;
float logje=-90.0f;
bool secondElapsed=false;
int menuTimeout = 0;;
int eepTimeout = 0;
int sc=0;
int hr=0;
int mn=0;

void IRAM_ATTR Timer0_ISR()
{
  int ctr;
  static int br;

  if (brightness < br)
    digitalWrite(LEDPIN, 0);               // The idea is to make some form of backlight adjustment here
  else
    digitalWrite(LEDPIN, 1);               // The idea is to make some form of backlight adjustment here

  // Adjust brightness in 500us interval
  if (br < 20) br++; else br = 0;
  if (br%4) return;                        // Do the rest only in 1ms interval

  // Handle button press and longpress
  if (!BUTTON)
  {
    if ((buttonhold>10) && (buttonhold < HOLDING)) buttonPress = 1;
    buttonhold = 0;

  }
  else if ((buttonhold < HOLDING) && (buttonhold > -1)) buttonhold++;

  if (ppmUpdate) ppmUpdate--;
  if (countSecond) countSecond--;
  if (!countSecond)
  {
    countSecond = 1000;
    secondElapsed = true;
    sc++;
    if (sc>59)
    {
      sc=0;
      mn++;
      if (mn>59)
      {
        mn=0;
        hr++;
        if (hr>23) hr = 0;
      }
    }
    if (menuTimeout>1) menuTimeout--;
    if (eepTimeout>1) eepTimeout--;
  }

  // Handle peak meters
  for (ctr=0;ctr<4;ctr++)
  {
    if (peakDrop[ctr]) peakDrop[ctr]--;
    if (!peakDrop[ctr])
    {
      peakDrop[ctr] = 20;
      if (peakHold[ctr] < 177) peakHold[ctr]++; // Counting pixels here, should have used defines, todo...
    }
  }
}

void read_encoder() {
  static uint8_t old_AB = 3;
  static int8_t encval = 0;
  static const int8_t enc_states[]  = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};

  old_AB <<= 2;

  if (digitalRead(ROTARY_PIN_A)) old_AB |= 0x02;
  if (digitalRead(ROTARY_PIN_B)) old_AB |= 0x01;
  encval += enc_states[( old_AB & 0x0f )];

  if (optenc == 1) {
    if (encval > 2) {
      if (rotarymode) rotary = -1; else rotary = 1;
      encval = 0;
    } else if (encval < -2) {
      if (rotarymode) rotary = 1; else rotary = -1;
      encval = 0;
    }
  } else {
    if (encval > 3) {
      if (rotarymode) rotary = -1; else rotary = 1;
      encval = 0;
    } else if (encval < -3) {
      if (rotarymode) rotary = 1; else rotary = -1;
      encval = 0;
    }
  }
  if (rotary != 0)
  {
    ElapsedTime = millis() - StartTime;
    StartTime = millis();
  }
  if (!digitalRead(INVERT)) rotarymode = 1; else rotarymode = 0;
}

void setup()
{
  int c, i,j, bit;
  uint16_t flash_timeout;
  int8_t choice = NO;
  uint32_t flashAddress;
  uint8_t update_available = 0;
  char *cp;

  EEPROM.begin(sizeof(esp32nvm));                                         // Reserves eeprom space for memory structs of EEPROM in cache. EEPROM get and put uses the cache. EEPROM.commit() commits to eeprom

  Serial.begin(115200);                                                   // Start the UART at a slow 115k2.

  Serial.println("Starting Digital Baseband!");                           // Wake up message

  headroom = int(32768.0F * (pow10(-HEADROOM/20)));                       // Define our headrom to FS of the audio ADC's
  // Serial.print("Headroom ");
  // Serial.println(headroom);
  fmclip =   1792;                                                        // Define our FM clip level
  nicamclip = 4096;//1536;                                                // Define our NICAM clip level

  tft.init();
  tft.setRotation(3);
  tft.setSwapBytes(true);

  // Create the sprites we need.
  ppmHolder.createSprite(FRAME_W,FRAME_H);
  ppmBar.createSprite(FRAME_W - 2 * BAR_SPACE,FRAME_H - 2 * BAR_SPACE);
  ppmValue.createSprite(FRAME_D, 16);
  ppmTicks.createSprite(FRAME_W - 2 * BAR_SPACE,FRAME_H - 2 * BAR_SPACE);
  ppmWindow.createSprite(172,240);
  menuItem.createSprite(138,18);

  pinMode(ROTARY_BUTTON, INPUT);
  pinMode(ROTARY_PIN_A, INPUT);
  pinMode(ROTARY_PIN_B, INPUT);
  attachInterrupt(digitalPinToInterrupt(ROTARY_PIN_A), read_encoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ROTARY_PIN_B), read_encoder, CHANGE);

  tft.pushImage(0,0,320,240,digibbt);                                     // Load our 'splash screen'
  pinMode(LEDPIN, OUTPUT);                                                // Switch on the background LED
  digitalWrite(LEDPIN, 1);

  // IIC Master begin
  Wire.setBufferSize(265);                                                // make the I2C buffer larger
  Wire.begin();
  Wire.setTimeout(250);                                                   // Set 250ms timeout on I2C
  Wire.setClock(400000);                                                  // Set 400kHz speed in I2C (fastest)

  // Set timer to 0.25ms interval, the LED is PWM'd from the IRQ and otherwise flickers
  Timer0_Cfg = timerBegin(0, 80, true);
  timerAttachInterrupt(Timer0_Cfg, &Timer0_ISR, true);
  timerAlarmWrite(Timer0_Cfg, 250, true);                                 // interval in us ticks
  timerAlarmEnable(Timer0_Cfg);

  delay(1000);                                                            // Wait for the FPGA to complete it's startup sequence

  WriteM25P80(WRITE_DISABLE, 0, 0, 0);                                    // Make sure the SPI is not in write enable mode (have seen it in that mode, after failed update)

  while (sizeof(info) != HWRead(I2C_ACCESS_INFO, (uint8_t *) &info, sizeof(info)))
  {
    Serial.print("Communication Error!");
    tft.fillRect(60, 60, 200, 120, TFT_BLACK);
    tft.drawString("Communication Error!", 80, 76, 2);
    tft.drawString("Boards disconnected?", 80, 108, 2);
    tft.drawString("! HALTING !", 80, 142, 2);
    while(1);
  }

  // serial = 10005;

  if (serial)
  {
    ReadM25P80(READ_DATA_BYTES, 0x7FFF0, &buffer[0], 2);              // Read two serial number bytes from 0x7FFF0
    if ((buffer[0] + 256*buffer[1]) == 65535)
    {
      buffer[0] = serial & 255;
      buffer[1] = serial / 256;
      Serial.print("Flashing serial number: ");
      Serial.println(serial);
      WriteM25P80(WRITE_ENABLE, 0, 0, 0);
      WriteM25P80(PAGE_PROGRAM, 0x7FFF0, &buffer[0], 2);              // Write two serial number bytes to 0x7FFF0
      delay(10);
      WriteM25P80(WRITE_DISABLE, 0, 0, 0);
    }
  }

  ReadM25P80(READ_DATA_BYTES, 0x7FFF0, &buffer[0], 2);                // Read two serial number bytes from 0x7FFF0
  serial = buffer[0] + 256*buffer[1];
  Serial.print("Serial number: ");
  Serial.println(serial);

  // if ((serial != 10013) && (serial != 10047) && (serial != 10004) && (serial != 10005)) while(1);

  WriteM25P80(WRITE_DISABLE, 0, 0, 0);                                         // Make sure the SPI is not in write enable mode (have seen it in that mode, after failed update)

  Serial.println("Reading local eeprom");
  EEPROM.get(0, esp32nvm);                                                     // Read ESP32's eeprom data

  cp = (char *)&esp32nvm; // Get the char pointer to the esp32nvm struct

  // //To reset the eeprom/flash area on the ESP32 - for testing purposes only
  // for (i=0; i<sizeof(esp32nvm); i++) 
  // {
  //   cp[i] = 255;
  // }

  if (esp32nvm.update.deferred > 1)                                            // check for virgin eeprom
  {
    Serial.println("Init local eeprom!");
    esp32nvm.update.deferred = 0;
    esp32nvm.update.major = SW_VERSION_MAJOR;
    esp32nvm.update.minor = SW_VERSION_MINOR;
  }

  if (SW_VERSION_MAJOR != info.sw_version_major || SW_VERSION_MINOR != info.sw_version_minor)
  {
    update_available = 1;
    tft.fillRect(60, 60, 200, 150, TFT_BLACK);
  }
  else
  {
    update_available = 0;
    tft.fillRect(60, 60, 200, 120, TFT_BLACK);
  }

  tft.drawString("HW Version: ", 84, 76, 2);
  sprintf(tempString, "%d", info.hw_version);
  tft.drawString(tempString, 200, 76, 2);

  tft.drawString("FPGA Version: ", 84, 92, 2);
  sprintf(tempString, "%d", info.fpga_version);
  tft.drawString(tempString, 200, 92, 2);

  tft.drawString("SW Version: ", 84, 108, 2);
  sprintf(tempString, "%d.%d", info.sw_version_major, info.sw_version_minor);
  tft.drawString(tempString, 200, 108, 2);

  tft.drawString("ESP32 Version: ", 84, 124, 2);

  String s = __DATE__;
  s.replace("Jan ","01"); // Date is like Jan 01 2024, so shorten it
  s.replace("Feb ","02");
  s.replace("Mar ","03");
  s.replace("Apr ","04");
  s.replace("May ","05");
  s.replace("Jun ","06");
  s.replace("Jul ","07");
  s.replace("Aug ","08");
  s.replace("Sep ","09");
  s.replace("Oct ","10");
  s.replace("Nov ","11");
  s.replace("Dec ","12");
  s.replace(" 20","");
  s.replace(" ","0");
  tft.drawString(s, 200, 124, 2);

  tft.drawString("Serial Number: ", 84, 140, 2);
  sprintf(tempString, "%d", serial);
  tft.drawString(tempString, 200, 140, 2);

  flash_timeout = 1000;                                                          // Used as timeout to FLASH the SW version if no action is taken.

  if (update_available)
  {
    choice = NO;
    i = 0;
    tft.drawString("New SW Version: ", 84, 158, 2);
    sprintf(tempString, "%d.%d", SW_VERSION_MAJOR, SW_VERSION_MINOR);
    tft.drawString(tempString, 200, 158, 2);
    if (!esp32nvm.update.deferred || (esp32nvm.update.major != SW_VERSION_MAJOR) || (esp32nvm.update.minor != SW_VERSION_MINOR))
    {
      choice = YES;
      ppmUpdate = 0;
      tft.drawString("Update FLASH?", 84, 174, 2);
      while (!BUTTON)
      {
        if (rotary != -1)
        {
          choice = YES;
          strncpy(tempString, "YES",5);
        }
        else
        {
          choice = NO;
          strncpy(tempString, "NO  ",5);
        }
        tft.drawString(tempString, 200, 174, 2);
        if (!ppmUpdate)
        {
          flash_timeout--;
          ppmUpdate = 250;
          switch (i)
          {
            case 0:
              strncpy(tempString, "/ ",5);
              j = 68;
              i++;
              break;
            case 1:
              strncpy(tempString, " - ",5);
              j = 63;
              i++;
              break;
            case 2:
              strncpy(tempString, "\\ ",5);
              j = 68;
              i++;
              break;
            case 3:
            default:
              strncpy(tempString, " | ",5);
              j = 63;
              i = 0;
              break;
          }
          tft.drawString(tempString, j, 174, 2);
          tft.drawString(tempString, j + 175, 174, 2);
        }
        else ppmUpdate--;
        if (!flash_timeout) break;
      }

      if (choice == NO)
      {
        Serial.println("Deferring Update!");
        esp32nvm.update.deferred = 1;
        // Remember the pending version when deferring the update. In the future, if a new pending version is found we can ignore the defer flag.
        esp32nvm.update.major = SW_VERSION_MAJOR;
        esp32nvm.update.minor = SW_VERSION_MINOR;
      }
      else
      {
        esp32nvm.update.deferred = 0;
        // Clear the version as we always will want to be able to upgrade.
        esp32nvm.update.major = 0;
        esp32nvm.update.minor = 0;
      }
    }
    else
    {
      tft.drawString("HOLD rotary to update!", 84, 174, 2);
      delay(5000);
      if (BUTTON) choice = YES;
    }
  }
  else
    delay(4000);

  if (choice == YES) // Flash when choice was set to YES
  {
    tft.fillRect(60, 60, 200, 150, TFT_BLACK);

    esp32nvm.update.deferred = 0;
    // Clear the version as we always will want to be able to upgrade.
    esp32nvm.update.major = 0;
    esp32nvm.update.minor = 0;

    tft.fillRect(60, 60, 200, 120, TFT_BLACK);
    // Could make single functions to delete and flash, but I want to do some screen things, indicating the update progress
    for (i=BASE; i < (8 + BASE); i++)
    {
      sprintf(tempString, "Erasing sector %i/8",(i+1)%9, BASE);
      tft.drawString(tempString, 84, 100, 2);
      EraseSectorM25P80(i * 0x10000);
    }
    sprintf(tempString, "Erasing sector 8/8, done.");
    tft.drawString(tempString, 84, 100, 2);

    flashAddress = BASE * 0x10000;
    for (i=0; i <= (topcnt/BLOCK); i++)
    {
      if (!(i % BLOCK))
      {
        sprintf(tempString, "Flashing sector %i/8",((i/BLOCK)+1)%9);
        tft.drawString(tempString, 84, 120, 2);
      }
      WriteM25P80(WRITE_ENABLE, 0, 0, 0);
      WriteM25P80(PAGE_PROGRAM, flashAddress, (uint8_t*)&top[i * BLOCK], BLOCK);
      ReadM25P80(READ_STATUS_REGISTER, 0, &buffer[0], 1);
      while(buffer[0] & WRITE_IN_PROGRESS) ReadM25P80(READ_STATUS_REGISTER, 0, &buffer[0], 1);
      flashAddress += BLOCK;
    }
    sprintf(tempString, "Flashing sector 8/8, done.");
    tft.drawString(tempString, 84, 120, 2);

    if (info.sw_version_major == 0 && info.sw_version_minor == 0)
    {
      Serial.print("Power cycle needed!");
      tft.drawString("Please powercycle!", 84, 140, 2);
      while(1);
    }

    tft.drawString("Rebooting!", 84, 140, 2);

    WriteM25P80(WRITE_DISABLE, 0, 0, 0);

    Wire.beginTransmission(I2C_ADDRESS/2);                            // transmit to device 0xB0/2
    Wire.write(I2C_ACCESS_COMMAND_REBOOT >> 8);                       // Reboot the FPGA
    Wire.write(I2C_ACCESS_COMMAND_REBOOT & 255);                      // Reboot the FPGA
    Wire.write(0x01);                                                 // Reboot the FPGA
    Wire.endTransmission();

    EEPROM.put(0, esp32nvm);
    EEPROM.commit();

    delay(1000);
    ESP.restart();
  }

  screen=BOOT;
  menu_action = 0;
  buttonhold = 1;

  // Load the last active settings
  while (sizeof(settings[0]) != HWRead(I2C_ACCESS_SETTINGS, (uint8_t *) &settings[0], sizeof(settings[0])))
  {
    Serial.print("Communication Error!");
    tft.fillRect(60, 60, 200, 120, TFT_BLACK);
    tft.drawString("Communication Error!", 80, 76, 2);
    tft.drawString("Boards disconnected?", 80, 108, 2);
    tft.drawString("! HALTING !", 80, 142, 2);
    while(1);
  }

  HWRead(I2C_ACCESS_READ_PRESET_STATUS, (uint8_t *) &preset_status, 4); // load the preset status
  Serial.print("preset_status = ");
  Serial.println(preset_status);
  memory = settings[0].general.last_recalled_presetnr;
  if (memory == 0) // no memory was read -> 1st run! Store current active settings in memory 1, and recall it
  {
    Serial.println("Memory is not valid, initializing it.");
    sprintf(settings[0].name, "DEFAULT");
    memory = 1;
    SavePreset(memory);
    Serial.print("preset_status = ");
    Serial.println(preset_status);
  }

  LoadPresetMirror(memory); // Load this preset into the mirror 

  // Serial.print("Last recalled memory = ");
  // Serial.println(settings[0].general.last_recalled_presetnr);

  // Read the pointers to the misc variables
  HWRead(I2C_ACCESS_PATTERN_MEMORY, (uint8_t *) &testLines, sizeof(testLines));
  if (esp32nvm.tclsettings.testcard > NUMTESTCARDS) // if outlier value, correct it
  {
    // Serial.println("Testcard init");
    esp32nvm.tclsettings.testcard = 0;
    esp32nvm.tclsettings.vits17_line = CCIR17;
    esp32nvm.tclsettings.vits18_line = MULTIBURST;
    esp32nvm.tclsettings.wss_line = 0;
  }

  testLines[16] = esp32nvm.tclsettings.vits17_line;
  testLines[17] = esp32nvm.tclsettings.vits18_line;
  testLines[22] = esp32nvm.tclsettings.wss_line;
  drawTC(esp32nvm.tclsettings.testcard);

  for (i=0;i<2;i++)
  {
    if (esp32nvm.osdsettings[i].x > 40 || esp32nvm.osdsettings[i].y > 16 || esp32nvm.osdsettings[i].x == 0 || esp32nvm.osdsettings[i].y == 0)
    { // If out of range, initialize the values
      esp32nvm.osdsettings[i].enable = 0;
      esp32nvm.osdsettings[i].x = 1;
      esp32nvm.osdsettings[i].y = 1 + i*15;
      esp32nvm.osdsettings[i].inverted = 0;
      ClearOSD(i, 0);
    }
  }

  writeUserOSD(-1);                                     // show the user OSD, as appropriate

  EEPROM.put(0, esp32nvm);
  EEPROM.commit();

  pinMode(NICAM_MUTE_PIN, INPUT_PULLUP);

  nicmute = NICAMMUTE;

  checkOverlap();           // Get initial state
  drawMain();
}


void loop()
{
  int i,j;
  static int x=0;
  char cp;
  static char posi=0;
  static char dir = 1;

  if (buttonPress && m_line == 0)             // Only change to a different page if not in a menu
  {
    menuTimeout = 0;
    menuMode = 0;
    menu_action = UPDATE_ALL;                   // Build the entire menu
    screen++;
    if (screen > TCS) screen = BOOT;
    buttonPress = 0;
    rot = 0;
    m_item = 0;
  }

  // for testcard/line insterter settings, which aren't held in the baseband
  if (eepTimeout == 1)
  {
    eepTimeout = 0;
    // Serial.println("Committing esp32nvmn to flash.");
    EEPROM.put(0, esp32nvm);
    EEPROM.commit();
  }

  // Added nicam mute option on pin 23 (IO25), ground to mute. Requested by PE1TER.
  if (NICAMMUTE != nicmute && screen < CONF)
  {
    nicmute = NICAMMUTE;
    settings[0].nicam.enable = nicmute ? 0: settings[1].nicam.enable;
    HWWrite(I2C_ACCESS_SETTINGS, (uint8_t *) &settings[0], sizeof(settings[0]));
    HWUpdate();
    // Serial.print("Nicam mute = ");
    // Serial.println(nicmute ? "OFF" : "ON");
    if (screen == MAIN) drawMenuItems(UPDATE_ALL);
  }

  switch (screen)
  {
    case BOOT:
      if (menu_action & UPDATE_ALL)
      {
        // Serial.println("Drawing splash screen");
        tft.pushImage(0,0,320,240,digibbt);
        menu_action = MENU_IDLE;
      }
      if (rot) brightness = constrain(brightness+rot, 1 , 20);
      rot = 0;
      break;
    case MAIN:
      if (menu_action & UPDATE_ALL)
        {
          drawMain();
          menu_action = MENU_IDLE;
        }
      if (menuTimeout == 1 && !menuMode) // If and when a timeout is hit, return to menu only if not in menu
      {
        menuTimeout = 0;
        menu_action = MENU_IDLE;
        m_line = 0;
        m_item = 0;
        menuMode = 0;
        drawMenuItems(UPDATE_ALL);
      }
      if (rot && menuMode == MENU_IDLE)
        {
          // Serial.println("Browsing items");
          menuTimeout = 10;
          drawMenuItems(NEXT_ITEM); // next can also be previous; depending on 'rot' [rotary] value 
          buttonPress = 0;
        }
      if (buttonPress)
      {
        menuTimeout = 10;
        if (menuMode == MENU_IDLE)
        {
          // Serial.println("Changing item");
          menuMode = UPDATE_ITEM;
          drawMenuItems(UPDATE_ITEM); // next can also be previous; depending on 'rot' [rotary] value 
          // buttonPress = 0;
        }
        else 
        {
          // (buttonPress && menuMode == UPDATE_ITEM)
          // Serial.println("Back to idle mode");
          // menuTimeout = 10;
          menuMode = MENU_IDLE;
          drawMenuItems(MENU_IDLE);
        }
        buttonPress = 0;
      }
      if (menuMode == UPDATE_ITEM && buttonhold == HOLDING)
        {
          menuMode = UPDATE_ITEM;
          menuTimeout = 10;
          // Serial.println("Do a toggle");
          drawMenuItems(TOGGLE_ITEM);
          buttonhold++;
          HWWrite(I2C_ACCESS_SETTINGS, (uint8_t *) &settings[0], sizeof(settings[0]));
          HWUpdate();
          // menu_action = MENU_IDLE;
        }
      if (rot && menuMode == UPDATE_ITEM)
      {
        // The rotary is rotated while in menu, so compute, change, write, update
        // Serial.println("Change a value");
        menuTimeout = 10;
        drawMenuItems(UPDATE_ITEM);
        rot = 0;
        if (m_line > 0) // If in main menu or memory menu, do not send settings to FPGA
        {
          HWWrite(I2C_ACCESS_SETTINGS, (uint8_t *) &settings[0], sizeof(settings[0]));
          HWUpdate();
        }
      }
      if (ppmUpdate == 0)
      {
        ppmUpdate = 20; // Do again after 20ms
        HWRead(I2C_ACCESS_READOUT, (uint8_t *) &hw_inputs, sizeof(HW_INPUTS));

        if (hw_inputs.adc1_left_audio_peak<32769) computePPM(0, hw_inputs.adc1_left_audio_peak);
        if (hw_inputs.adc1_right_audio_peak<32769) computePPM(1, hw_inputs.adc1_right_audio_peak);
        if (hw_inputs.adc2_left_audio_peak<32769) computePPM(2, hw_inputs.adc2_left_audio_peak);
        if (hw_inputs.adc2_right_audio_peak<32769) computePPM(3, hw_inputs.adc2_right_audio_peak);
        for (i=1; i<8; i++) drawMeter(i);
      }
      break;
    case CONF:
      if (bessel0) menuTimeout = 10;
      if (menu_action & UPDATE_ALL)
      {
        // Serial.println("Drawing config");
        drawConfigMenu(UPDATE_ALL);
        menu_action = MENU_IDLE;
      }
      if (menuTimeout == 1 && !menuMode) // If and when a timeout is hit, return to menu
      {
        if (m_line==1)
        {
          HWRead(I2C_ACCESS_SETTINGS, (uint8_t *) &settings[0], sizeof(settings[0]));
        }
        menuTimeout = 0;
        menuMode = MENU_IDLE;
        bessel0 = 0;
        m_line = 0;
        drawConfigMenu(UPDATE_ALL);
      }

      if (buttonPress)
      {
        menuTimeout = 10;
        if ((m_line == 9 || m_line ==1) && m_item == 2) // if on the call or the name; this is a special case!
        {
          switch (editString)
          {
            case CHAR_IDLE:
              // Serial.println("CHAR MOVE MODE");
              editString = CHAR_MOVED;
              menuMode = UPDATE_ITEM;
              charPos = 0;
              break;
            case CHAR_POS:
              // Serial.println("EXIT EDIT MODE");
              editString = CHAR_IDLE;
              charPos = -1;                         // No character position selected
              menuMode = MENU_IDLE;                 // Return to item selection mode
              break;
            case CHAR_EDIT:
              // Serial.println("CHAR POS MODE");
              editString = CHAR_POS;
              menuMode = UPDATE_ITEM;                 // Return to item selection mode
              HWWrite(I2C_ACCESS_SETTINGS, (uint8_t *) &settings[0], sizeof(settings[0]));
              HWUpdate();
              break;
            case CHAR_MOVED:
              // Serial.println("CHAR EDIT MODE");
              editString = CHAR_EDIT;
              break;
            default: // escape - should not happen
              editString = CHAR_IDLE;
              charPos = -1;
              menuMode = MENU_IDLE;
              break;
          }
          buttonPress = 0;
          drawConfigMenu(UPDATE_ITEM);
        }
        else
        {
          if (menuMode == MENU_IDLE)
          {
            menuMode = UPDATE_ITEM;
            drawConfigMenu(UPDATE_ITEM);
            buttonPress = 0;
          }
          else
          {
            if (!bessel0)
            {
              menuMode = MENU_IDLE;
              drawConfigMenu(MENU_IDLE);
            }
            buttonPress = 0;
          }
        }
      }
      if (rot && menuMode == 0)
        {
          menuTimeout = 10;
          drawConfigMenu(NEXT_ITEM); // next can also be previous; depending on 'rot' [rotary] value 
          buttonPress = 0;
        }

      if (menuMode == UPDATE_ITEM && buttonhold == HOLDING)
        {
          menuTimeout = 10;
          // Serial.println("Do a toggle");
          drawConfigMenu(TOGGLE_ITEM);
          buttonhold++;
          HWWrite(I2C_ACCESS_SETTINGS, (uint8_t *) &settings[0], sizeof(settings[0]));
          HWUpdate();
          menu_action = 0;
        }
      if (rot && menuMode == 2)
      {
        // Serial.println("Rotary action");
        if (m_line != 0) menuTimeout = 10;
        drawConfigMenu(UPDATE_ITEM);
        rot = 0;
        if (m_line > 1) // If in main menu or memory menu, do not send settings to FPGA
        {
          HWWrite(I2C_ACCESS_SETTINGS, (uint8_t *) &settings[0], sizeof(settings[0]));
          HWUpdate();
        }
        ElapsedTime = 0;
      }
      break;
    case TCS:
      if (menu_action & UPDATE_ALL)
      {
        // Serial.println("Drawing TestCard screen");
        drawTCSMenu(UPDATE_ALL);
        menu_action = MENU_IDLE;
      }
      if (menuTimeout == 1 && !menuMode) // If and when a timeout is hit, return to menu only if not in menu
      {
        menuTimeout = 0;
        menu_action = MENU_IDLE;
        m_line = 0;
        m_item = 0;
        menuMode = 0;
        drawTCSMenu(UPDATE_ALL);
      }
      if (rot && menuMode == MENU_IDLE)
        {
          // Serial.println("Browsing items");
          menuTimeout = 10;
          drawTCSMenu(NEXT_ITEM); // next can also be previous; depending on 'rot' [rotary] value 
          buttonPress = 0;
        }
      if (buttonPress)
      {
        if ((m_line == 7 || m_line ==10) && m_item == 1) // if on OSD1/2; this is a special case!
        {
          switch (editString)
          {
            case CHAR_IDLE:
              // Serial.println("CHAR MOVE MODE");
              editString = CHAR_MOVED;
              menuMode = UPDATE_ITEM;
              charPos = 0;
              break;
            case CHAR_POS:
              // Serial.println("EXIT EDIT MODE");
              editString = CHAR_IDLE;
              charPos = -1;                         // No character position selected
              menuMode = MENU_IDLE;                 // Return to item selection mode
              break;
            case CHAR_EDIT:
              // Serial.println("CHAR POS MODE");
              editString = CHAR_POS;
              menuMode = UPDATE_ITEM;                 // Return to item selection mode
              HWWrite(I2C_ACCESS_SETTINGS, (uint8_t *) &settings[0], sizeof(settings[0]));
              HWUpdate();
              break;
            case CHAR_MOVED:
              // Serial.println("CHAR EDIT MODE");
              editString = CHAR_EDIT;
              break;
            default: // escape - should not happen
              editString = CHAR_IDLE;
              charPos = -1;
              menuMode = MENU_IDLE;
              break;
          }
          buttonPress = 0;
          drawTCSMenu(UPDATE_ITEM);
        }
        else
        {
          if (menuMode == MENU_IDLE)
          {
            // Serial.println("Changing item");
            menuTimeout = 10;
            menuMode = UPDATE_ITEM;
            drawTCSMenu(UPDATE_ITEM); // next can also be previous; depending on 'rot' [rotary] value 
            buttonPress = 0;
          }
          else 
          {
            // (buttonPress && menuMode == UPDATE_ITEM)
            // Serial.println("Back to idle mode");
            menuTimeout = 10;
            menuMode = MENU_IDLE;
            drawTCSMenu(MENU_IDLE);
            buttonPress = 0;
          }
        }
      }
      if (buttonhold == HOLDING)
        {
          menuTimeout = 10;
          // Serial.println("Do a toggle");
          drawTCSMenu(TOGGLE_ITEM);
          buttonhold++;
          // HWWrite(I2C_ACCESS_SETTINGS, (uint8_t *) &settings[0], sizeof(settings[0]));
          // HWUpdate();
          // menu_action = 0;
        }

      if (rot && menuMode == UPDATE_ITEM)
        {
          menuTimeout = 10;
          drawTCSMenu(UPDATE_ITEM); // next can also be previous; depending on 'rot' [rotary] value 
          buttonPress = 0;
        }
      rot = 0;
      break;
    default:
      break;
  }

  if (rotary!=0)
  {
    rot = rotary;
    rotary = 0;
  }

  if(secondElapsed && settings[0].video.show_menu)
  {
    secondElapsed = false;
    sprintf(tempString, "%02d:%02d:%02d",hr,mn,sc);
    HWWrite(I2C_ACCESS_DISPLAY+ OSD_CLOCK_POSITION, (uint8_t *)&tempString, 8);
  }
}
