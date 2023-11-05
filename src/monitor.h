#ifndef SLACKER_MONITOR_H
#define SLACKER_MONITOR_H

// X11 Libraries
#include <X11/Xlib.h>
#include <X11/keysym.h>

// Standard Libraries
#include <bits/stdint-uintn.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

// Slacker Headers
#include "constants.h"

////////////////////
/// Enumerations
////////////////////

// Color Schemes
enum SlackerColorscheme {
	SlackerColorscheme_Norm,
	SlackerColorscheme_Sel,
};

// Cursor
enum SlackerCursorState {
	SlackerCursorState_Normal,
	SlackerCursorState_Resize,
	SlackerCursorState_Move,
	SlackerCursorState_Last
};

// EWMH atoms
// TODO: Link to docs
enum SlackerEWMHAtom {
	SlackerEWMHAtom_NetSupported,
	SlackerEWMHAtom_NetWMName,
	SlackerEWMHAtom_NetWMState,
	SlackerEWMHAtom_NetWMCheck,
	SlackerEWMHAtom_NetWMFullscreen,
	SlackerEWMHAtom_NetActiveWindow,
	SlackerEWMHAtom_NetWMWindowType,
	SlackerEWMHAtom_NetWMWindowTypeDialog,
	SlackerEWMHAtom_NetClientList,
	SlackerEWMHAtom_NetLast
};

// Default Atoms
// TODO: Link to docs
enum SlackerDefaultAtom {
	SlackerDefaultAtom_WMProtocols,
	SlackerDefaultAtom_WMDelete,
	SlackerDefaultAtom_WMState,
	SlackerDefaultAtom_WMTakeFocus,
	SlackerDefaultAtom_WMLast
};

// Clicks
enum SlackerClick {
	SlackerClick_TagBar,
	SlackerClick_LtSymbol,
	SlackerClick_StatusText,
	SlackerClick_WinTitle,
	SlackerClick_ClientWin,
	SlackerClick_RootWin,
	SlackerClick_Last
};

//////////////////////////
/// Data Structures
//////////////////////////

typedef union Arg Arg;

/// @brief Represents an argument to a function
union Arg {
	int32_t i;
	uint32_t ui;
	float f;
	const void *v;
};

typedef struct Button Button;

/// @brief Represents a button
struct Button {
	// SlackerClick enum
	uint32_t click;
	// Event mask
	uint32_t mask;
	// X11 button number
	uint32_t button;
	// Callback function to handle the button press
	void (*button_handler_callback)(const Arg *arg);
	// Argument to the callback function
	const Arg arg;
};

typedef struct Monitor Monitor;

/// @brief Represents an X client window
typedef struct Client Client;
struct Client {
	char name[MAX_CLIENT_NAME_LEN];
	float mina, maxa;
	int32_t x, y, w, h;
	int32_t oldx, oldy, oldw, oldh;
	int32_t basew, baseh, incw, inch, maxw, maxh, minw, minh, hintsvalid;
	int32_t bw, oldbw;
	uint32_t tags;
	int32_t isfixed;
	int32_t isfloating;
	int32_t isurgent;
	int32_t neverfocus;
	int32_t oldstate;
	int32_t isfullscreen;
	Client *next;
	Client *stack_next;
	Monitor *mon;
	Window win;
};

/// @brief A hotkey combination to be bound to a function
typedef struct KeyMap KeyMap;
struct KeyMap {
	/// Modifier
	uint32_t mod;
	/// X11 keysym
	KeySym keysym;
	/// Callback functions
	void (*keymap_callback)(const Arg *);
	/// Argument to the callback function
	const Arg arg;
};

/// @brief Represents a layout
typedef struct Layout Layout;
struct Layout {
	/// Layout symbol thats displayed in the bar
	const char *symbol;
	/// Layout callback function
	void (*layout_arrange_callback)(Monitor *);
};

/// @brief Represents a physical monitor
struct Monitor {
	/// Layout symbol thats displayed in the bar
	char layout_symbol[MAX_LAYOUT_SYMBOL_LEN];
	/// Master width factor (how much space the master window takes up)
	float mfact;
	/// Number of windows in the master area, default is 1
	int32_t nmaster;
	/// Number of all monitors?
	int32_t num;
	/// Bar geometry
	int32_t by;
	/// Screen geometry
	int32_t mx, my, mw, mh;
	/// Window area
	int32_t wx, wy, ww, wh;
	/// Selected tags
	uint32_t seltags;
	/// Selected layout
	uint32_t selected_layout;
	/// Tag set
	uint32_t tagset[MAX_TAG_SETS];
	/// Bar is shown
	int32_t showbar;
	/// Bar is on top 1 for true, 0 for false
	int32_t topbar;
	/// Currently selected client
	Client *selected_client;
	/// A linked list of all the clients on this monitor
	Client *client_list;
	/// Stack of clients, used to preserve order
	Client *client_stack;
	/// Next monitor in the linked list of monitors
	Monitor *next;
	/// Xid for the bar window
	Window barwin;
	/// Layouts
	const Layout *layouts[MAX_LAYOUTS];
};

/// @brief Sets the layout to master stack for a monitor
void Monitor__layout_master_stack(Monitor *m);

/// @brief Sets the layout to monocle for a monitor
void Monitor__layout_monocle(Monitor *m);

/// @brief Window WINDOW_RULES
typedef struct SlackerWindowRule WindowRule;
struct SlackerWindowRule {
	const char *window_class;
	const char *instance;
	const char *title;
	uint32_t tags;
	int32_t isfloating;
	int32_t monitor;
};

#endif // SLACKER_H
