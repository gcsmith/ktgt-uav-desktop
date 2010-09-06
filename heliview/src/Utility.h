// -----------------------------------------------------------------------------
// File:    Utility.h
// Authors: Garrett Smith
// -----------------------------------------------------------------------------

#ifndef _HELIVIEW_UTILITY__H_
#define _HELIVIEW_UTILITY__H_

#define SafeDelete(x)       do { delete (x); (x) = NULL; } while (0)
#define SafeDeleteArray(x)  do { delete[] (x); (x) = NULL; } while (0)

#define RAD 3.141592654 / 180.0

#endif // _HELIVIEW_UTILITY__H_

