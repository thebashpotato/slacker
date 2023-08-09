#ifndef SLACKER_H
#define SLACKER_H

// X11 Libraries
#include <X11/Xatom.h>
#include <X11/Xlib.h>

// Standard Libraries
#include <stdbool.h>
#include <stdint.h>

// Slacker Headers
#include "slacker_constants.h"
#include "slacker_monitor.h"


// typedef struct Slacker Slacker;
// struct Slacker {
//     const char *broken;
//     char stext[MAX_CLIENT_NAME_LEN];
//     int32_t screen;
//     int32_t screen_width;
//     int32_t screen_height;
//     int32_t bar_height;
//     int32_t left_right_text_padding_summation;
//     int32_t (*xerrorxlib)(Display *, XErrorEvent *);
//     uint32_t numlockmask;
//     Atom window_manager_atom[SlackerDefaultAtom_WMLast];
//     Atom net_atom[NetLast];
//     bool is_running;
//     Cur *cursor[CurLast];
//     Cur **scheme;
//     Display *display;
//     Drw *draw;
//     Monitor *monitors;
//     Monitor *selected_monitor;
//     Window root_window;
//     Window window_manager_check_window;
// };

#endif
