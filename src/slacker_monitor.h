#ifndef SLACKER_MONITOR_H
#define SLACKER_MONITOR_H

// X11 Libraries
#include <X11/Xlib.h>
#include <X11/keysym.h>

// Standard Libraries
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

// Slacker Headers
#include "slacker_constants.h"

////////////////////
/// Enumerations
////////////////////

// Color Schemes
enum SlackerColorscheme {
    SlackerColorscheme_Norm,
    SlackerColorscheme_Sel,
};

// Cursor
enum SlackerCursor {
    SlackerCursor_Normal,
    SlackerCursor_Resize,
    SlackerCursor_Move,
    SlackerCursor_Last
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

/// @brief Represents an argument to a function
typedef union Arg Arg;
union Arg {
    int32_t i;
    int32_t ui;
    float f;
    const void *v;
};

/// @brief Represents a button
typedef struct Button Button;
struct Button {
    uint32_t click;
    uint32_t mask;
    uint32_t button;
    void (*func)(const Arg *arg);
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
    Client *snext;
    Monitor *mon;
    Window win;
};

/// @brief A hotkey combination to be bound to a function
typedef struct KeyMap KeyMap;
struct KeyMap {
    // Modifier
    uint32_t mod;
    KeySym keysym;
    // Callback functions
    void (*func)(const Arg *);
    const Arg arg;
};

/// @brief Represents a layout
typedef struct Layout Layout;
struct Layout {
    const char *symbol;
    void (*arrange)(Monitor *);
};

/// @brief Represents a physical monitor
struct Monitor {
    char ltsymbol[16];
    float mfact;
    int32_t nmaster;
    int32_t num;
    int32_t by;             /* bar geometry */
    int32_t mx, my, mw, mh; /* screen size */
    int32_t wx, wy, ww, wh; /* window area  */
    uint32_t seltags;
    uint32_t sellt;
    uint32_t tagset[2];
    int32_t showbar;
    int32_t topbar;
    Client *clients;
    Client *sel;
    Client *stack;
    Monitor *next;
    Window barwin;
    const Layout *lt[2];
};

void monitor_layout_master_stack(Monitor *m);

void monitor_layout_monocle(Monitor *m);

/// @brief Window WINDOW_RULES
typedef struct WindowRule WindowRule;
struct WindowRule {
    const char *window_class;
    const char *instance;
    const char *title;
    uint32_t tags;
    int32_t isfloating;
    int32_t monitor;
};

#endif// SLACKER_H
