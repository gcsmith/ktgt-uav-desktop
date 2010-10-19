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

// logging modes
#define LOG_MODE_EXCESSIVE      2  // logs normal and debug plus excessive
#define LOG_MODE_NORMAL_DEBUG   1  // logs normal (below) plus debug messages
#define LOG_MODE_NORMAL         0  // logs fails, errors, warnings, information

// logging types
#define LOG_TYPE_FAILURE        0x0001
#define LOG_TYPE_ERROR          0x0002
#define LOG_TYPE_WARNING        0x0004
#define LOG_TYPE_INFO           0x0008
#define LOG_TYPE_DEBUG          0x0010
#define LOG_TYPE_EXTRADEBUG     0x0020

// macro inverts the bit (y) in the number (x)
#define BIT_INV(x,y) ((((x) & (y)) ^ (y)) | ((x) & ~(y)))

#endif // _HELIVIEW_UTILITY__H_

