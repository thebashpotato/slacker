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

// typedef struct State State;
// struct State {
// 	/// screen id
// 	int32_t screen;
// 	/// screen width
// 	int32_t sw;
// 	/// screen height
// 	int32_t sh;
// 	/// bar height
// 	int32_t bh;
// 	/// sum of left and right text padding
// 	int32_t lrpad;
// 	/// num lock mask
// 	uint32_t numlockmask;
// 	/// List of monitors
// 	Monitor *mons;
// 	/// selected monitor
// 	Monitor *selmon;
// };

typedef struct Slacker Slacker;
struct Slacker {
	/// Client text which displays in the bar when the client is broken
	const char *broken;
	/// Current client text which displays in the bar
	char stext[MAX_CLIENT_NAME_LEN];
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
	Display *dpy;
	/// Drawable abstraction
	Drw *drw;
	/// All connected monitors
	Monitor *mons;
	/// Currently selected monitor
	Monitor *selmon;
	/// Root window
	Window root;
	/// Window manager check window
	Window wmcheckwin;
};

#endif
