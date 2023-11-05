#ifndef SLACKER_H
#define SLACKER_H

// X11 Libraries
#include <X11/Xatom.h>
#include <X11/Xlib.h>

// Standard Libraries
#include <stdbool.h>
#include <stdint.h>

// Slacker Headers
#include "constants.h"
#include "drawable.h"
#include "monitor.h"

typedef struct Slacker Slacker;
struct Slacker {
	/// Status text that is displayed in the top right corner of the bar
	char status_text[MAX_STATUS_BAR_TEXT_LEN];
	/// X screen id
	int32_t screen;
	/// X display screen geometry width
	int32_t screen_width;
	/// X display screen geometry height
	int32_t screen_height;
	/// Bar height
	int32_t bar_height;
	/// Sum of left and right padding for text
	int32_t left_right_padding_sum;
	/// X error callback function
	int32_t (*xerrorxlib)(Display *, XErrorEvent *);
	/// Num lock mask, defaults to 0
	uint32_t numlockmask;
	/// Window manager Atoms
	Atom wmatom[SlackerDefaultAtom_WMLast];
	/// Net Atoms
	Atom netatom[SlackerEWMHAtom_NetLast];
	/// Is the window manager running, defaults to true
	bool is_running;
	/// Slacker cursor states
	SlackerCursor *cursor[SlackerCursorState_Last];
	/// Slacker color schemes
	SlackerColor **scheme;
	/// X display
	Display *display;
	/// Drawable abstraction
	Drw *draw;
	/// Linked list of all connected monitors
	Monitor *monitor_list;
	/// Currently selected monitor
	Monitor *selected_monitor;
	/// Root window
	Window root;
	/// Window manager check window
	Window wmcheckwin;
};

extern Slacker g_slacker;

#endif
