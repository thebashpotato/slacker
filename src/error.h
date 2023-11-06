#ifndef SWM_ERROR_H
#define SWM_ERROR_H

// X11 Libraries
#include <X11/Xlib.h>

/// @brief X11 dummy error function
int32_t xerrordummy(Display *dpy, XErrorEvent *ee);

/// @brief X11 error start function
int32_t xerrorstart(Display *dpy, XErrorEvent *ee);

#endif
