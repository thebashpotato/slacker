#include "config.h"
#include "constants.h"
#include "monitor.h"

// clang-format off
const uint32_t G_BORDER_PIXEL                  = 1;
const uint32_t G_SNAP_PIXEL                    = 32;
const int32_t G_SHOW_BAR                       = 1;
const int32_t G_TOP_BAR                        = 1;
const char G_USER_FONT[]                       = "CaskaydiaCove Nerd Font Mono:size=16";
const char G_DMENU_FONT[]                      = "CaskaydiaCove Nerd Font Mono:size=16";
const char G_COLORSCHEME_BACKGROUND[]          = "#111216";
const char G_COLORSCHEME_BORDER[]              = "#6d6d6d";
const char G_COLORSCHEME_FOREGROUND[]          = "#9e9e9e";
const char G_COLORSCHEME_SECONDARY[]           = "#f0dada";
const char G_COLORSCHEME_PRIMARY[]             = "#a32ea7";
const float G_MASTER_FACTOR                    = 0.55;
const int32_t G_MASTER_COUNT                   = 1;
const int32_t G_RESIZE_HINTS                   = 1;
const int32_t G_LOCK_FULLSCREEN                = 1;
char G_DMENU_MONITOR[]                         = "0";
const char *G_DMENU_COMMAND[]                  = {"dmenu_run", "-m", G_DMENU_MONITOR, "-fn", G_DMENU_FONT, "-nb", G_COLORSCHEME_BACKGROUND, "-nf", G_COLORSCHEME_FOREGROUND, "-sb", G_COLORSCHEME_PRIMARY, "-sf", G_COLORSCHEME_SECONDARY, NULL};
const char *G_TERMINAL_COMMAND[]               = {"konsole", NULL};

const char *G_COLORSCHEMES[MAX_COLOR_SCHEMES][MAX_COLOR_VARIANTS] = {
    // ================================================================================================
    // |                        fg                        bg                          border          |
    // ================================================================================================
    [SlackerColorscheme_Norm]    = { G_COLORSCHEME_FOREGROUND, G_COLORSCHEME_BACKGROUND,   G_COLORSCHEME_BORDER },
    [SlackerColorscheme_Sel]     = { G_COLORSCHEME_SECONDARY, G_COLORSCHEME_PRIMARY,    G_COLORSCHEME_PRIMARY  },
};

const char *G_TAGS[] = { "", "", "", "", "", "", "", "", "" };

const WindowRule G_WINDOW_RULES[] = {
    // ================================================================================================
    // | Class Name   Instance        Title           Tags            Mask     isfloating   monitor   |
    // ================================================================================================
    { "Gimp",        NULL,          NULL,           0,              1,      -1 },
    { "Firefox",     NULL,          NULL,           (1 << 8),       0,      -1 },
};

const Layout G_LAYOUTS[] = {
    // =================================
    // | Symbol       Arrange function |
    // =================================
    { "[]=",         Monitor__layout_master_stack    }, // first entry is default
    { "><>",         NULL                            }, // no layout function means floating behavior
    { "[M]",         Monitor__layout_monocle         }, // monocle layout
};

const KeyMap G_KEYBINDINGS[MAX_KEY_BINDINGS] = {
    // =============================================================================================
    // | Modifier            Key             Function                               Argument       |
    // =============================================================================================
    { MODKEY,                XK_p,           Slacker__spawn,                    {.v = G_DMENU_COMMAND}          },
    { MODKEY | ShiftMask,    XK_Return,      Slacker__spawn,                    {.v = G_TERMINAL_COMMAND}       },
    { MODKEY,                XK_b,           Slacker__togglebar,                {0}                             },
    { MODKEY,                XK_j,           Slacker__focus_stack,              {.i = +1}                       },
    { MODKEY,                XK_k,           Slacker__focus_stack,              {.i = -1}                       },
    { MODKEY,                XK_i,           Slacker__increment_n_master,       {.i = +1}                       },
    { MODKEY,                XK_d,           Slacker__increment_n_master,       {.i = -1}                       },
    { MODKEY,                XK_h,           Slacker__setmfact,                 {.f = -0.05}                    },
    { MODKEY,                XK_l,           Slacker__setmfact,                 {.f = +0.05}                    },
    { MODKEY,                XK_Return,      Slacker__zoom,                     {0}                             },
    { MODKEY,                XK_Tab,         Slacker__view,                     {0}                             },
    { MODKEY | ShiftMask,    XK_c,           Slacker__kill_client,              {0}                             },
    { MODKEY,                XK_t,           Slacker__setlayout,                {.v = &G_LAYOUTS[0]}            },
    { MODKEY,                XK_f,           Slacker__setlayout,                {.v = &G_LAYOUTS[1]}            },
    { MODKEY,                XK_m,           Slacker__setlayout,                {.v = &G_LAYOUTS[2]}            },
    { MODKEY,                XK_space,       Slacker__setlayout,                {0}                             },
    { MODKEY | ShiftMask,    XK_space,       Slacker__togglefloating,           {0}                             },
    { MODKEY,                XK_0,           Slacker__view,                     {.ui = ~0}                      },
    { MODKEY | ShiftMask,    XK_0,           Slacker__tag,                      {.ui = ~0}                      },
    { MODKEY,                XK_comma,       Slacker__focus_monitor,            {.i = -1}                       },
    { MODKEY,                XK_period,      Slacker__focus_monitor,            {.i = +1}                       },
    { MODKEY | ShiftMask,    XK_comma,       Slacker__tagmon,                   {.i = -1}                       },
    { MODKEY | ShiftMask,    XK_period,      Slacker__tagmon,                   {.i = +1}                       },
    { MODKEY | ShiftMask,    XK_q,           Slacker__quit,                     {0}                             },
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

const Button G_CLICKABLE_BUTTONS[MAX_BUTTON_BINDINGS] = {
    // ====================================================================================================================
    // | Click                      EventMask           Button          Function                            Argument      |
    // ====================================================================================================================
    { SlackerClick_LtSymbol,           0,              Button1,        Slacker__setlayout,                  {0}                         },
    { SlackerClick_LtSymbol,           0,              Button3,        Slacker__setlayout,                  {.v = &G_LAYOUTS[2]}        },
    { SlackerClick_WinTitle,           0,              Button2,        Slacker__zoom,                       {0}                         },
    { SlackerClick_StatusText,         0,              Button2,        Slacker__spawn,                      {.v = G_TERMINAL_COMMAND}   },
    { SlackerClick_ClientWin,          MODKEY,         Button1,        Slacker__move_with_mouse,            {0}                         },
    { SlackerClick_ClientWin,          MODKEY,         Button2,        Slacker__togglefloating,             {0}                         },
    { SlackerClick_ClientWin,          MODKEY,         Button3,        Slacker__resize_client_with_mouse,   {0}                         },
    { SlackerClick_TagBar,             0,              Button1,        Slacker__view,                       {0}                         },
    { SlackerClick_TagBar,             0,              Button3,        Slacker__toggleview,                 {0}                         },
    { SlackerClick_TagBar,             MODKEY,         Button1,        Slacker__tag,                        {0}                         },
    { SlackerClick_TagBar,             MODKEY,         Button3,        Slacker__toggletag,                  {0}                         },
};

// clang-format on
