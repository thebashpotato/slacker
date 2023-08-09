#include "slacker_config.h"
#include "slacker_constants.h"
#include "slacker_monitor.h"

// clang-format off

const uint32_t GLOBAL_BORDER_PIXEL                  = 1;
const uint32_t GLOBAL_SNAP_PIXEL                    = 32;
const int32_t GLOBAL_SHOW_BAR                       = 1;
const int32_t GLOBAL_TOP_BAR                        = 1;
const char GLOBAL_USER_FONT[]                       = "CaskaydiaCove Nerd Font Mono:size=16";
const char GLOBAL_DMENU_FONT[]                      = "CaskaydiaCove Nerd Font Mono:size=16";
const char GLOBAL_COLORSCHEME_BACKGROUND[]          = "#222222";
const char GLOBAL_COLORSCHEME_BORDER[]              = "#444444";
const char GLOBAL_COLORSCHEME_FOREGROUND[]          = "#bbbbbb";
const char GLOBAL_COLORSCHEME_SECONDARY[]           = "#eeeeee";
const char GLOBAL_COLORSCHEME_PRIMARY[]             = "#005577";
const float GLOBAL_MASTER_FACTOR                    = 0.55;
const int32_t GLOBAL_MASTER_COUNT                   = 1;
const int32_t GLOBAL_RESIZE_HINTS                   = 1;
const int32_t GLOBAL_LOCK_FULLSCREEN                = 1;
char GLOBAL_DMENU_MONITOR[]                         = "0";
const char *GLOBAL_DMENU_COMMAND[]                  = {"dmenu_run", "-m", GLOBAL_DMENU_MONITOR, "-fn", GLOBAL_DMENU_FONT, "-nb", GLOBAL_COLORSCHEME_BACKGROUND, "-nf", GLOBAL_COLORSCHEME_FOREGROUND, "-sb", GLOBAL_COLORSCHEME_PRIMARY, "-sf", GLOBAL_COLORSCHEME_SECONDARY, NULL};
const char *GLOBAL_TERMINAL_COMMAND[]               = {"alacritty", NULL};

const char *GLOBAL_COLORSCHEMES[MAX_COLOR_SCHEMES][MAX_COLOR_VARIANTS] = {
    // ================================================================================================
    // |                        fg                        bg                          border          |
    // ================================================================================================
    [SlackerColorscheme_Norm]    = { GLOBAL_COLORSCHEME_FOREGROUND, GLOBAL_COLORSCHEME_BACKGROUND,   GLOBAL_COLORSCHEME_BORDER },
    [SlackerColorscheme_Sel]     = { GLOBAL_COLORSCHEME_SECONDARY, GLOBAL_COLORSCHEME_PRIMARY,    GLOBAL_COLORSCHEME_PRIMARY  },
};

const char *GLOBAL_TAGS[] = { "", "", "", "", "", "", "", "", "" };

const WindowRule GLOBAL_WINDOW_RULES[] = {
    // ================================================================================================
    // | Class Name   Instance        Title           Tags            Mask     isfloating   monitor   |
    // ================================================================================================
    { "Gimp",        NULL,          NULL,           0,              1,      -1 },
    { "Firefox",     NULL,          NULL,           (1 << 8),       0,      -1 },
};

const Layout GLOBAL_LAYOUTS[] = {
    // =================================
    // | Symbol       Arrange function |
    // =================================
    { "[]=",         monitor_layout_master_stack    }, // first entry is default
    { "><>",         NULL                           }, // no layout function means floating behavior
    { "[M]",         monitor_layout_monocle         }, // monocle layout
};

const KeyMap GLOBAL_KEYBINDINGS[MAX_KEY_BINDINGS] = {
    // =============================================================================
    // | Modifier           Key             Function                Argument       |
    // =============================================================================
    { MODKEY,                XK_p,           spawn,          {.v = GLOBAL_DMENU_COMMAND}        },
    { MODKEY | ShiftMask,    XK_Return,      spawn,          {.v = GLOBAL_TERMINAL_COMMAND}     },
    { MODKEY,                XK_b,           togglebar,      {0}                                },
    { MODKEY,                XK_j,           focusstack,     {.i = +1}                          },
    { MODKEY,                XK_k,           focusstack,     {.i = -1}                          },
    { MODKEY,                XK_i,           incnmaster,     {.i = +1}                          },
    { MODKEY,                XK_d,           incnmaster,     {.i = -1}                          },
    { MODKEY,                XK_h,           setmfact,       {.f = -0.05}                       },
    { MODKEY,                XK_l,           setmfact,       {.f = +0.05}                       },
    { MODKEY,                XK_Return,      zoom,           {0}                                },
    { MODKEY,                XK_Tab,         view,           {0}                                },
    { MODKEY | ShiftMask,    XK_c,           killclient,     {0}                                },
    { MODKEY,                XK_t,           setlayout,      {.v = &GLOBAL_LAYOUTS[0]}          },
    { MODKEY,                XK_f,           setlayout,      {.v = &GLOBAL_LAYOUTS[1]}          },
    { MODKEY,                XK_m,           setlayout,      {.v = &GLOBAL_LAYOUTS[2]}          },
    { MODKEY,                XK_space,       setlayout,      {0}                                },
    { MODKEY | ShiftMask,    XK_space,       togglefloating, {0}                                },
    { MODKEY,                XK_0,           view,           {.ui = ~0}                         },
    { MODKEY | ShiftMask,    XK_0,           tag,            {.ui = ~0}                         },
    { MODKEY,                XK_comma,       focusmon,       {.i = -1}                          },
    { MODKEY,                XK_period,      focusmon,       {.i = +1}                          },
    { MODKEY | ShiftMask,    XK_comma,       tagmon,         {.i = -1}                          },
    { MODKEY | ShiftMask,    XK_period,      tagmon,         {.i = +1}                          },
    { MODKEY | ShiftMask,    XK_q,           quit,           {0}                                },
    TAGKEYS(XK_1, 0),
    TAGKEYS(XK_2, 1),
    TAGKEYS(XK_3, 2),
    TAGKEYS(XK_4, 3),
    TAGKEYS(XK_5, 4),
    TAGKEYS(XK_6, 5),
    TAGKEYS(XK_7, 6),
    TAGKEYS(XK_8, 7),
    TAGKEYS(XK_9, 8),
};

const Button GLOBAL_CLICKABLE_BUTTONS[MAX_BUTTON_BINDINGS] = {
    // ===============================================================================================
    // | Click              EventMask        Button          Function           Argument             |
    // ===============================================================================================
    { SlackerClick_LtSymbol,           0,              Button1,        setlayout,      {0}                              },
    { SlackerClick_LtSymbol,           0,              Button3,        setlayout,      {.v = &GLOBAL_LAYOUTS[2]}        },
    { SlackerClick_WinTitle,           0,              Button2,        zoom,           {0}                              },
    { SlackerClick_StatusText,         0,              Button2,        spawn,          {.v = GLOBAL_TERMINAL_COMMAND}   },
    { SlackerClick_ClientWin,          MODKEY,         Button1,        movemouse,      {0}                              },
    { SlackerClick_ClientWin,          MODKEY,         Button2,        togglefloating, {0}                              },
    { SlackerClick_ClientWin,          MODKEY,         Button3,        resizemouse,    {0}                              },
    { SlackerClick_TagBar,             0,              Button1,        view,           {0}                              },
    { SlackerClick_TagBar,             0,              Button3,        toggleview,     {0}                              },
    { SlackerClick_TagBar,             MODKEY,         Button1,        tag,            {0}                              },
    { SlackerClick_TagBar,             MODKEY,         Button3,        toggletag,      {0}                              },
};

// clang-format on
