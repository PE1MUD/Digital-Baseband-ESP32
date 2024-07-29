//--------------------------------------------------------------------
//	TestcardMenu.h
//
//  (c) PE1MUD, PE1OBW 2024
//--------------------------------------------------------------------
#ifndef TCSMENU_H
#define TCSMENU_H

// Pointers to the waveforms
#define STAIRCASE       1*20
#define MULTIBURST      2*20
#define RAMP            3*20
#define CCIR17          4*20
#define WIDE_ANAMORPH   5*20
#define WIDE            6*20
#define WIDE_TOP        7*20

#include "eep.h"

extern void drawTCSMenu(int);
extern void drawTC(uint8_t);

#endif