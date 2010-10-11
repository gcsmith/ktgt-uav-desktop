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

// macro inverts the bit (y) in the number (x)
#define BIT_INV(x,y) ((((x) & (y)) ^ (y)) | ((x) & ~(y)))

#endif // _HELIVIEW_UTILITY__H_

