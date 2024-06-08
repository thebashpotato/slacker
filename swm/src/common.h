#ifndef SWM_COMMON_H
#define SWM_COMMON_H

// X11 Libraries
#include <X11/Xlib.h>

// Standard Libraries
#include <stdint.h>

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

/// @brief Supported EWMH atoms
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

/// @brief Supported WMAtoms
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

/// @brief Window WINDOW_RULES
typedef struct SlackerWindowRule SlackerWindowRule;
struct SlackerWindowRule {
	const char *window_class;
	const char *instance;
	const char *title;
	uint32_t tags;
	int32_t isfloating;
	int32_t monitor;
};

/// @brief Represents an argument to a function
typedef union Arg Arg;
union Arg {
	int32_t i;
	uint32_t ui;
	float f;
	const void *v;
};

typedef struct Button Button;
typedef void (*ButtonHandler)(const Arg *arg);

/// @brief Represents a button
struct Button {
	// SlackerClick enum
	uint32_t click;
	// Event mask
	uint32_t event_mask;
	// X11 button number
	uint32_t id;
	// Callback function to handle the button press
	ButtonHandler handler;
	// Argument to the callback function
	const Arg arg;
};

typedef struct KeyMap KeyMap;
typedef void (*KeyMapHandler)(const Arg *arg);

/// @brief A hotkey combination to be bound to a function
struct KeyMap {
	/// Modifier key
	uint32_t mod;
	/// X11 keysym representation of the key
	KeySym keysym;
	/// Callback function
	KeyMapHandler handler;
	/// Argument to the callback function
	const Arg arg;
};

#endif
