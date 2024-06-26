#ifndef SWM_CONFIG_H
#define SWM_CONFIG_H

// X11 libraries
#include <X11/X.h>

// Standard Libraries
#include <bits/stdint-intn.h>
#include <bits/stdint-uintn.h>

// Slacker Headers
#include "constants.h"
#include "common.h"
#include "monitor.h"
#include "modifiers.h" // clang-analyzer complains since the functions from this header are used in the TAGKEYS macro.

/// @brief Appearance control variables
extern const uint32_t G_BORDER_PIXEL;
extern const uint32_t G_SNAP_PIXEL;
extern const uint32_t G_GAP_PIXEL;
extern const int32_t G_SHOW_BAR;
extern const int32_t G_TOP_BAR;
extern const char G_USER_FONT[];
extern const char G_DMENU_FONT[];
extern const char G_COLORSCHEME_BACKGROUND[];
extern const char G_COLORSCHEME_BORDER[];
extern const char G_COLORSCHEME_FOREGROUND[];
extern const char G_COLORSCHEME_SECONDARY[];
extern const char G_COLORSCHEME_PRIMARY[];

/// @brief Layout control variables
extern const float G_MASTER_FACTOR;
extern const int32_t G_MASTER_COUNT;
extern const int32_t G_RESIZE_HINTS;
extern const int32_t G_LOCK_FULLSCREEN;
extern const Layout G_LAYOUTS[MAX_LAYOUTS];

/// @brief Tags which are displayed in the bar
///
/// @details If you want more than 9 tags, you will need to change the
/// MAX_TAGS constant in constants.h to match the number of tags you want.
extern const char *G_TAGS[MAX_TAGS];

/// @brief Foreground, Background and Border colors
extern const char *G_COLORSCHEMES[MAX_COLOR_SCHEMES][MAX_COLOR_VARIANTS];

/// @brief Add rules for applications which need to be managed in a specific way.
/// @details The rules are matched against the window class name, role, and title.
/// which the user can obtain using the xprop program.
extern const SlackerWindowRule G_WINDOW_RULES[MAX_WINDOW_RULES];

/// @brief Terminal and command launcher definitions
extern char G_DMENU_MONITOR[2];
extern const char *G_DMENU_COMMAND[];
extern const char *G_TERMINAL_COMMAND[];

/// @brief Custom KeyMap Bindings
extern const KeyMap G_KEYBINDINGS[MAX_KEY_BINDINGS];

/// @brief button definitions
///
/// @details click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin
extern const Button G_CLICKABLE_BUTTONS[MAX_BUTTON_BINDINGS];

/// @brief KeyMap Definitions
#define MODKEY Mod4Mask

/// @brief Helper for defining the tag key bindings
#define TAGKEYS(KEY, TAG)                                                  \
	{ MODKEY, KEY, Swm__view, { .ui = 1 << TAG } },                    \
		{ MODKEY | ControlMask,                                    \
		  KEY,                                                     \
		  Swm__toggleview,                                         \
		  { .ui = 1 << TAG } },                                    \
		{ MODKEY | ShiftMask, KEY, Swm__tag, { .ui = 1 << TAG } }, \
	{                                                                  \
		MODKEY | ControlMask | ShiftMask, KEY, Swm__toggletag,     \
		{                                                          \
			.ui = 1 << TAG                                     \
		}                                                          \
	}

/// @brief helper for spawning shell commands in the pre dwm-5.0 fashion
#define SHCMD(cmd)                                 \
	{                                          \
		.v = (const char *[])              \
		{                                  \
			"/bin/sh", "-c", cmd, NULL \
		}                                  \
	}

#endif // SWM_CONFIG_H
