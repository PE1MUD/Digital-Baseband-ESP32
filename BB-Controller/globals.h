//--------------------------------------------------------------------
//	globals.h
//
//  (c) PE1MUD, PE1OBW 2024
//--------------------------------------------------------------------
// Include always
#include <TFT_eSPI.h>                             // https://github.com/Bodmer/TFT_eSPI
#include "FONT16.h"                               // The anti aliased (smoothed) font

#include "hw_control_registers.h"                 // FPGA control registers
#include "baseband_i2c_interface.h"               // FPGA I2C interface
#include "twowire.h"                              // Two wire stuffs
#include "testcardmenu.h"

#ifndef GLOBALS_H
#define GLOBALS_H

#define OSD_CLOCK_POSITION  (13 + 0 * 40)

#define MODIFY m_line == line && menuMode == UPDATE_ITEM && rot && m_item ==
#define TOGGLE m_line == line && menuMode == UPDATE_ITEM && buttonhold == HOLDING && m_item ==

// Screen items
#define BOOT                0
#define MAIN                1
#define CONF                2
#define TCS                 3
#define OSD                 4

#define HOLDING             1000

// Define rotary pins
#define ROTARY_PIN_A        34
#define ROTARY_PIN_B        36
#define ROTARY_BUTTON       39

#define NICAM_MUTE_PIN      25

#define HEADROOM            12.0F               // 12.0 dB headroom over 0dB

#define MENU_IDLE           0 // NO menu or item action
#define NEXT_ITEM           1 // Next menu item
#define UPDATE_ITEM         2 // Update item mode
#define TOGGLE_ITEM         3 // when button is held...
#define UPDATE_ACTIVE       4 // Update the active menu item only
#define UPDATE_ALL          5 // Update all menu items

#define CHAR_IDLE           0 // editString while not touching a string
#define CHAR_POS            1 // editString while editing char position
#define CHAR_MOVED          2 // Set to this when the char pos was actually moved
#define CHAR_EDIT           3 // editString while editing character

#define POS                 0
#define NEG                 1

// Some color definitions
#define FILL1               0x2104/2  // Colors changed with a lib update, argh!
#define FRAMECOLOR          0x7bef
#define TFT_DARKGRAY        0x4208
#define FONT                0xFFF0

#define MORSECODE           1

// Text editing modes statemachine
#define TEXTSTART           0         // Returned from textedit (shortpress leaves menu item, scroll selects textpos mode)
#define TEXTPOS             1         // Did some positioning (shortpress selects edit mode for char)
#define TEXTEDIT            2         // Edit a character (shortpress selects textstart mode for char)

extern int fontColor[5];
extern int fillColor[5];

extern char OSDbuffer[];
extern char tempString[];                        // Used in sprintf's
extern TFT_eSPI tft;
extern int buttonPress;
extern int buttonhold;
extern int screen;
extern int m_line;
extern int m_item;
extern int rot;
extern int bessel0;
extern int charPos;
extern int editString;
extern int menuMode;
extern int textMode;
extern int ppmUpdate;
extern int eepTimeout;
extern uint8_t testLines[];
extern uint8_t memory;
extern uint8_t preview_memory;
extern uint8_t show_memory;
extern uint32_t preset_status;
// FPGA related variables
extern HW_INPUTS hw_inputs;
extern SETTINGS settings[];
extern INFO info;
extern bool checkOverlap();
extern bool carrierOverlap[5];
extern unsigned long ElapsedTime;
extern void writeUserOSD(int);

extern TFT_eSPI tft;
extern TFT_eSprite menuItem;
extern TFT_eSprite ppmWindow;
extern TFT_eSprite ppmHolder;
extern TFT_eSprite ppmBar;
extern TFT_eSprite ppmTicks;
extern TFT_eSprite infoSprite;
extern TFT_eSprite ppmValue;
extern TFT_eSprite menuItem;


#endif