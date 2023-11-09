// X11 libraries

// Standard library
#include <stdio.h>

// Slacker headers
#include "error.h"
#include "utils.h"

int32_t xerrordummy(Display *dpy, XErrorEvent *ee)
{
	return 0;
}

/// @brief Startup Error handler to check if another window manager
/// is already running
int32_t xerrorstart(Display *dpy, XErrorEvent *ee)
{
	die("slacker: another window manager is already running\n");
	return -1;
}
