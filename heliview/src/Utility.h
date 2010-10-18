// -----------------------------------------------------------------------------
// File:    Utility.h
// Authors: Garrett Smith
// -----------------------------------------------------------------------------

#ifndef _HELIVIEW_UTILITY__H_
#define _HELIVIEW_UTILITY__H_

#include <stdint.h>

#define SafeDelete(x)       do { delete (x); (x) = NULL; } while (0)
#define SafeDeleteArray(x)  do { delete[] (x); (x) = NULL; } while (0)

#ifndef PI
#define PI 3.141592654 
#endif

#define D2R(x) ((x) * PI / 180.0)
#define R2D(x) ((x) * 180.0 / PI)

// output location log flags
#define LOG_LOC_DIALOG  0x1
#define LOG_LOC_FILE    0x2
#define LOG_LOC_ALL     0x3

// logging priority flags
#define LOG_PRIORITY_LOW    0
#define LOG_PRIORITY_HI     1

// logging modes
#define LOG_MODE_EXCESSIVE   1  // logs high and low priority messages
#define LOG_MODE_NORMAL      0  // logs high priority messages

// macro inverts the bit (y) in the number (x)
#define BIT_INV(x,y) ((((x) & (y)) ^ (y)) | ((x) & ~(y)))

#endif // _HELIVIEW_UTILITY__H_

