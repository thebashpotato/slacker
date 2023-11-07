#ifndef SWM_ERROR_H
#define SWM_ERROR_H

// X11 Libraries
#include <X11/Xlib.h>

/// @brief X11 dummy error function
///
/// @details This is required when we need to pass a event handler callback
/// to the XSetErrorHandler function, but we don't actually care about the errors
/// X will emit.
int32_t xerrordummy(Display *xconn, XErrorEvent *ee);

/// @brief X11 error start function
///
/// @details Used for startup error handling
int32_t xerrorstart(Display *xconn, XErrorEvent *ee);

#endif
