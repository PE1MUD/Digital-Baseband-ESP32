//--------------------------------------------------------------------
//	MainMenu.h
//
//  (c) PE1MUD, PE1OBW 2024
//--------------------------------------------------------------------
#ifndef MAINMENU_H
#define MAINMENU_H

// Parameters for creation of PPM bars and outlines
#define FRAME_W         22
#define FRAME_H         190
#define FRAME_MID       42
#define FRAME_X         38
#define FRAME_Y         44
#define FRAME_D         34
#define BAR_SPACE       6

// extern void drawMenuItems(int);
extern void drawMain();
extern void drawMenuItems(int);
extern void computePPM(int, int);
extern void drawMeter(int);
extern char tempString[60];
extern int peakHold[4];         // Peak pixel value
extern float peakHolddB[4];     // Peak dB value for texts above peakmeters
extern int peakDrop[4];         // peak Drop timers
extern int headroom, fmclip, nicamclip;
#endif