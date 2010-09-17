// -----------------------------------------------------------------------------
// File:    WindowsGamepad.h
// Authors: Garrett Smith
// -----------------------------------------------------------------------------

#ifndef _HELIVIEW_WINDOWSGAMEPAD__H_
#define _HELIVIEW_WINDOWSGAMEPAD__H_

class WindowsGamepad: public Gamepad
{
public:
    virtual bool open(const QString &device);
    virtual void close();
};

#endif // _HELIVIEW_WINDOWSGAMEPAD__H_

