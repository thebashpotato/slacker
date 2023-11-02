#ifndef SLACKER_CONFIG_H
#define SLACKER_CONFIG_H

// Standard Libraries
#include <stdint.h>

// Slacker Headers
#include "constants.h"
#include "modifiers.h"
#include "monitor.h"

/// @brief Appearance
extern const uint32_t GLOBAL_BORDER_PIXEL;
extern const uint32_t GLOBAL_SNAP_PIXEL;
extern const int32_t GLOBAL_SHOW_BAR;
extern const int32_t GLOBAL_TOP_BAR;
extern const char GLOBAL_USER_FONT[];
extern const char GLOBAL_DMENU_FONT[];
extern const char GLOBAL_COLORSCHEME_BACKGROUND[];
extern const char GLOBAL_COLORSCHEME_BORDER[];
extern const char GLOBAL_COLORSCHEME_FOREGROUND[];
extern const char GLOBAL_COLORSCHEME_SECONDARY[];
extern const char GLOBAL_COLORSCHEME_PRIMARY[];

/// @brief Layouts
extern const float GLOBAL_MASTER_FACTOR;
extern const int32_t GLOBAL_MASTER_COUNT;
extern const int32_t GLOBAL_RESIZE_HINTS;
extern const int32_t GLOBAL_LOCK_FULLSCREEN;
extern const Layout GLOBAL_LAYOUTS[MAX_LAYOUTS];

/// @brief Foreground, Background and Border colors
extern const char *GLOBAL_COLORSCHEMES[MAX_COLOR_SCHEMES][MAX_COLOR_VARIANTS];

/// @brief Tags
extern const char *GLOBAL_TAGS[MAX_TAGS];

/// @brief Window Rules
extern const WindowRule GLOBAL_WINDOW_RULES[MAX_WINDOW_RULES];

/// @brief Terminal and command launcher definitions
extern char GLOBAL_DMENU_MONITOR[2];
extern const char *GLOBAL_DMENU_COMMAND[];
extern const char *GLOBAL_TERMINAL_COMMAND[];

/// @brief KeyMap Definitions
#define MODKEY Mod1Mask

#define TAGKEYS(KEY, TAG)                                                    \
    {MODKEY, KEY, view, {.ui = 1 << TAG}},                                   \
            {MODKEY | ControlMask, KEY, toggleview, {.ui = 1 << TAG}},       \
            {MODKEY | ShiftMask, KEY, tag, {.ui = 1 << TAG}},                \
    {                                                                        \
        MODKEY | ControlMask | ShiftMask, KEY, toggletag, { .ui = 1 << TAG } \
    }

/// @brief helper for spawning shell commands in the pre dwm-5.0 fashion
#define SHCMD(cmd)                                           \
    {                                                        \
        .v = (const char *[]) { "/bin/sh", "-c", cmd, NULL } \
    }

/// @brief Custom KeyMap Bindings
extern const KeyMap GLOBAL_KEYBINDINGS[MAX_KEY_BINDINGS];

/// @brief button definitions
///
/// @details click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin
extern const Button GLOBAL_CLICKABLE_BUTTONS[MAX_BUTTON_BINDINGS];

#endif// SLACKER_CONFIG_H
