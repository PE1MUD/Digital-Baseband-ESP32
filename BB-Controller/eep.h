//--------------------------------------------------------------------
//	eep.h - some flash based persistent storage in the esp32
//
//  (c) PE1MUD, PE1OBW 2024
//--------------------------------------------------------------------
#ifndef EEP_H
#define EEP_H

// Adding stuff to make memories
#define MEMHOLD             0
#define MEMRUN              1
#define MEMPREVIEW          2
#define NUMTESTCARDS        6         // the highest index to a predefined testcard

typedef struct
  {
    uint8_t deferred = 0;
    uint8_t major = 0;
    uint8_t minor = 0;
  } UPDATE;

typedef struct {
	uint8_t	x;
	uint8_t y;
  uint8_t inverted;
  uint8_t enable;
  char osd[24];
} OSDSETTINGS;

typedef struct {
  uint8_t vits17_line;
  uint8_t vits18_line;
  uint8_t wss_line;
  uint8_t testcard;
} TCLSETTINGS;

typedef struct {
	UPDATE update;
	OSDSETTINGS osdsettings[2];
	TCLSETTINGS tclsettings;
} ESP32NVM;

extern ESP32NVM esp32nvm;

#endif