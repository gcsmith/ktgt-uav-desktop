// -----------------------------------------------------------------------------
// File:    Utility.h
// Authors: Garrett Smith
// -----------------------------------------------------------------------------

#ifndef _HELIVIEW_UTILITY__H_
#define _HELIVIEW_UTILITY__H_

#define SafeDelete(x)       do { delete (x); (x) = NULL; } while (0)
#define SafeDeleteArray(x)  do { delete[] (x); (x) = NULL; } while (0)

#ifndef PI
#define PI 3.141592654 
#endif

#define D2R(x) ((x) * PI / 180.0)
#define R2D(x) ((x) * 180.0 / PI)

#endif // _HELIVIEW_UTILITY__H_

