//--------------------------------------------------------------------
//	Version nr
//
// (c) PE1OBW, PE1MUD
//--------------------------------------------------------------------

#ifndef VERSION_H_
#define VERSION_H_

#include "is_golden.h"

#if IS_GOLDEN == 0
	#define SW_VERSION_MAJOR	1
	#define SW_VERSION_MINOR	3
#else
	#define SW_VERSION_MAJOR	0
	#define SW_VERSION_MINOR	0
#endif

#endif /* VERSION_H_ */
