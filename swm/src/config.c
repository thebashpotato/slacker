#include "config.h"
#include "constants.h"
#include "monitor.h"
#include "modifiers.h"

// clang-format off
const uint32_t G_BORDER_PIXEL                   = 2;
const uint32_t G_SNAP_PIXEL                     = 32;
const int32_t G_SHOW_BAR                        = 1;
const int32_t G_TOP_BAR                         = 1;
const char G_USER_FONT[]                        = "NotoSansM NFM:size=17:style=Bold";
const char G_DMENU_FONT[]                       = "NotoSansM NFM:size=18:style=Bold";
const char G_COLORSCHEME_BACKGROUND[]           = "#1a1b26";
const char G_COLORSCHEME_BORDER[]               = "#393939";
const char G_COLORSCHEME_FOREGROUND[]           = "#dde1e6";
const char G_COLORSCHEME_SECONDARY[]            = "#24283b";
const char G_COLORSCHEME_PRIMARY[]              = "#7aa2f7";
const float G_MASTER_FACTOR                     = 0.55;
const int32_t G_MASTER_COUNT                    = 1;
const int32_t G_RESIZE_HINTS                    = 1;
const int32_t G_LOCK_FULLSCREEN                 = 1;
char G_DMENU_MONITOR[]                          = "0";
const char *G_DMENU_COMMAND[]                   = {"dmenu_run", "-m", G_DMENU_MONITOR, "-fn", G_DMENU_FONT, "-nb", G_COLORSCHEME_BACKGROUND, "-nf", G_COLORSCHEME_FOREGROUND, "-sb", G_COLORSCHEME_PRIMARY, "-sf", G_COLORSCHEME_SECONDARY, NULL};
const char *G_TERMINAL_COMMAND[]                = {"kitty", NULL};
const char *G_TAGS[]                            = { "", "", "", "", "", "", "", "", "" };

const char *G_COLORSCHEMES[MAX_COLOR_SCHEMES][MAX_COLOR_VARIANTS] = {
    // ================================================================================================
    // |                        fg                        bg                          border          |
    // ================================================================================================
    [SlackerColorscheme_Norm]    = { G_COLORSCHEME_FOREGROUND, G_COLORSCHEME_BACKGROUND,   G_COLORSCHEME_BORDER },
    [SlackerColorscheme_Sel]     = { G_COLORSCHEME_SECONDARY, G_COLORSCHEME_PRIMARY,    G_COLORSCHEME_PRIMARY  },
};


const SlackerWindowRule G_WINDOW_RULES[] = {
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
    { MODKEY,                XK_p,           Swm__spawn,                    {.v = G_DMENU_COMMAND}          },
    { MODKEY | ShiftMask,    XK_Return,      Swm__spawn,                    {.v = G_TERMINAL_COMMAND}       },
    { MODKEY,                XK_b,           Swm__togglebar,                {0}                             },
    { MODKEY,                XK_j,           Swm__focus_stack,              {.i = +1}                       },
    { MODKEY,                XK_k,           Swm__focus_stack,              {.i = -1}                       },
    { MODKEY,                XK_i,           Swm__increment_n_master,       {.i = +1}                       },
    { MODKEY,                XK_d,           Swm__increment_n_master,       {.i = -1}                       },
    { MODKEY,                XK_h,           Swm__setmfact,                 {.f = -0.05}                    },
    { MODKEY,                XK_l,           Swm__setmfact,                 {.f = +0.05}                    },
    { MODKEY,                XK_Return,      Swm__zoom,                     {0}                             },
    { MODKEY,                XK_Tab,         Swm__view,                     {0}                             },
    { MODKEY | ShiftMask,    XK_c,           Swm__kill_client,              {0}                             },
    { MODKEY,                XK_t,           Swm__setlayout,                {.v = &G_LAYOUTS[0]}            },
    { MODKEY,                XK_f,           Swm__setlayout,                {.v = &G_LAYOUTS[1]}            },
    { MODKEY,                XK_m,           Swm__setlayout,                {.v = &G_LAYOUTS[2]}            },
    { MODKEY,                XK_space,       Swm__setlayout,                {0}                             },
    { MODKEY | ShiftMask,    XK_space,       Swm__togglefloating,           {0}                             },
    { MODKEY,                XK_0,           Swm__view,                     {.ui = ~0}                      },
    { MODKEY | ShiftMask,    XK_0,           Swm__tag,                      {.ui = ~0}                      },
    { MODKEY,                XK_comma,       Swm__focus_monitor,            {.i = -1}                       },
    { MODKEY,                XK_period,      Swm__focus_monitor,            {.i = +1}                       },
    { MODKEY | ShiftMask,    XK_comma,       Swm__tagmon,                   {.i = -1}                       },
    { MODKEY | ShiftMask,    XK_period,      Swm__tagmon,                   {.i = +1}                       },
    { MODKEY | ShiftMask,    XK_q,           Swm__quit,                     {0}                             },
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
    { SlackerClick_LtSymbol,           0,              Button1,        Swm__setlayout,                  {0}                         },
    { SlackerClick_LtSymbol,           0,              Button3,        Swm__setlayout,                  {.v = &G_LAYOUTS[2]}        },
    { SlackerClick_WinTitle,           0,              Button2,        Swm__zoom,                       {0}                         },
    { SlackerClick_StatusText,         0,              Button2,        Swm__spawn,                      {.v = G_TERMINAL_COMMAND}   },
    { SlackerClick_ClientWin,          MODKEY,         Button1,        Swm__move_with_mouse,            {0}                         },
    { SlackerClick_ClientWin,          MODKEY,         Button2,        Swm__togglefloating,             {0}                         },
    { SlackerClick_ClientWin,          MODKEY,         Button3,        Swm__resize_client_with_mouse,   {0}                         },
    { SlackerClick_TagBar,             0,              Button1,        Swm__view,                       {0}                         },
    { SlackerClick_TagBar,             0,              Button3,        Swm__toggleview,                 {0}                         },
    { SlackerClick_TagBar,             MODKEY,         Button1,        Swm__tag,                        {0}                         },
    { SlackerClick_TagBar,             MODKEY,         Button3,        Swm__toggletag,                  {0}                         },
};

// clang-format on
