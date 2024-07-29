//--------------------------------------------------------------------
//	Version nr
//
//  (c) PE1MUD, PE1OBW 2024
//--------------------------------------------------------------------

#ifndef VERSION_H_
#define VERSION_H_

#include "is_golden.h"

#if IS_GOLDEN == 0
	#define SW_VERSION_MAJOR	1
	#define SW_VERSION_MINOR	0
#else
	#define SW_VERSION_MAJOR	0
	#define SW_VERSION_MINOR	0
#endif

#endif /* VERSION_H_ */
