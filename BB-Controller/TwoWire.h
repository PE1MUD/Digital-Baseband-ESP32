//--------------------------------------------------------------------
//	TwoWire.h
//
//  (c) PE1MUD, PE1OBW 2024
//--------------------------------------------------------------------
#ifndef TWOWIRE_H
#define TWOWIRE_H

extern uint32_t HWWrite(uint16_t, uint8_t*, uint32_t);
extern void HWUpdate();
extern uint32_t HWRead(uint16_t, uint8_t*, uint32_t);
extern void HWDefaults();
extern void CommandWait(uint16_t);

#endif