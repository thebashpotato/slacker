#ifndef SWM_H
#define SWM_H

// X11 Libraries
#include <X11/Xatom.h>
#include <X11/Xlib.h>

// Standard Libraries
#include <stdbool.h>
#include <stdint.h>

// Slacker Headers
#include "drawable.h"
#include "error.h"
#include "common.h"
#include "constants.h"
#include "monitor.h"

///////////////////////////////////////////////////////
/// 				Helper Macros
//////////////////////////////////////////////////////

/// @brief Get the text width of a string using the current font.
/// Pad it with the left and right padding + 2.
#define TEXTW(X)                                 \
	(drw_fontset_getwidth(g_swm.draw, (X)) + \
	 g_swm.left_right_padding_sum + 2)

/// @brief Clean a mask of all the key modifiers
#define CLEANMASK(mask)                                              \
	(mask & ~(g_swm.numlockmask | LockMask) &                    \
	 (ShiftMask | ControlMask | Mod1Mask | Mod2Mask | Mod3Mask | \
	  Mod4Mask | Mod5Mask))

/// @brief Detech if a button was pressed
#define BUTTONMASK (ButtonPressMask | ButtonReleaseMask)

/// @brief Detect mouse movement
#define MOUSEMASK (BUTTONMASK | PointerMotionMask)

/// @brief Bit mask to flip between tags
#define TAGMASK ((1 << LENGTH(G_TAGS)) - 1)

///////////////////////////////////////////////////////
///					Definitions
//////////////////////////////////////////////////////

typedef struct Swm Swm;
typedef int32_t (*SlackerXErrorHandler)(Display *, XErrorEvent *);

/// @brief The main structure for the slacker window manager.
///
/// @details This struct is used to store all the global state of the window manager, and uses
/// all of the other data structures in the source tree.
struct Swm {
	/// Status text that is displayed in the top right corner of the bar
	char status_text[MAX_STATUS_BAR_TEXT_LEN];
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
	SlackerXErrorHandler xerror_callback;
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
	/// X display conenction
	Display *xconn;
	/// Drawable abstraction
	Drw *draw;
	/// Linked list of all connected monitors
	Monitor *monitor_list;
	/// Currently selected monitor
	Monitor *selected_monitor;
	/// Root window id
	Window root_wid;
	/// _NET_SUPPORTING_WM_CHECK window id for the EWMH
	/// (Extended Window Manager Hints) protocol
	Window ewmh_support_wid;
};

/// @brief Global mutable instance of the slacker window manager.
///
/// @details Since slacker is single threaded and synchronous,
/// we can get away with this, and it's really fast.
extern Swm g_swm;

/// @brief Initializes the X11, cleans up processes, checks for other running window managers.
void Swm__startup(void);

/// @brief Frees all memory allocated by the window manager.
///
/// @details The following resources are freed:
/// - All monitors
/// - All cursors
/// - All color schemes
/// - The check window
/// - The drawable abstraction
void Swm__delete(void);

/// @brief X11 error handler
///
/// @details There's no way to check accesses to destroyed windows, thus those cases are
/// ignored (especially on UnmapNotify's). Other types of errors call Xlibs
/// default error handler, which may call exit.
int32_t Swm__xerror_handler(Display *xconn, XErrorEvent *ee);

/// @brief Applies X window rules to a client
///
/// @param `client` The client to apply rules to
void Swm__applyrules(Client *client);

/// @brief Apply window size hints to a client
///
/// x, y, w, and h should all be in a Rect struct.
/// the only slacker variables that are used are screen_width and height
/// which should be moved into their own struct called Screen.
int32_t Swm__applysizehints(Client *client, int *x, int *y, int *w, int *h,
			    int interact);

/// @brief Top level arrange function. Manages n monitors.
///
/// @details If the monitor is NULL, iterate through the linked list of monitors and
/// decided whether to show or hide them.
///
/// @param `monitor` Takes a single monitor instead of a list of monitors, since a Client holds
/// a pointer to a single monitor.
void Swm__arrange_monitors(Monitor *monitor);

/// @brief	Not sure what the purpose of this is, Debugging required.
///
/// @param `dir` TODO:
Monitor *Swm__dir_to_monitor(int dir);

/// @brief Draws the bar for a monitor.
///
/// @details TODO: Document this function in detail, and refactor.
void Swm__drawbar(Monitor *monitor);

/// @brief Draws the bar for all monitors.
void Swm__drawbars(void);

/// @brief Focuses on a client
///
/// @param `client` The client to focus on
void Swm__focus(Client *client);

/// @brief Get the atom property for a given client.
///
/// @details TODO: This should be a Client__ function
/// @param `client` The client to get the atom prop for
Atom Swm__get_atom_prop(Client *client, Atom prop);

/// @brief Gets the pointer coordinates relative to the root window's origin.
///
/// @param `root_x_return` Pointer to the root x coordinate
/// @param `root_y_return` Pointer to the root y coordinate
///
/// @return BadWindow if the window is invalid
int32_t Swm__getrootptr(int *root_x_return, int *root_y_return);

/// @brief Gets the state of a given window and frees it via Xlib.
///
/// @param `wid` The window to get the state of and free
int64_t Swm__getstate(Window w_id);

/// @brief Get the text property for a given window id.
/// @details Takes a window id instead of a Client, because it is used for the root window as well.
///
/// @param `w_id` The window id to get the text prop for
/// @param `atom` The atom to get the text prop for
/// @param `text` Stores the text property name to be used
/// @param `size` The size of the text property name
///
/// @returns 1 if the text property was found, 0 otherwise
bool Swm__get_text_prop(Window w_id, Atom atom, char *text, uint32_t size);

/// @brief Checks to see if any of the supported button masks were pressed
/// on a client window.
///
/// @param `client` The client to check for button presses on
/// @param `focused` Whether or not the client should be focused
void Swm__grab_buttons(Client *client, bool focused);

/// @brief Sets up all the custom key bindings defined in GLOBAL_KEYBINDINGS
///
/// @details This function is called once at startup, and again on mappping notify events.
void Swm__grab_keys(void);

/// @brief Creates a new client window and manages it.
///
/// @param `w` The window to manage
/// @param `wa` The window attributes of the window to manage
void Swm__manage(Window w_id, XWindowAttributes *wa);

/// @brief Transforms coordinates, width and height to the monitor they are on.
///
/// @param `x` The x coordinate
/// @param `y` The y coordinate
/// @param `w` The width
/// @param `h` The height
///
/// @return The monitor the coordinates, width and height are on.
Monitor *Swm__rect_to_monitor(int x, int y, int w, int h);

/// @brief Resizes a client with the given dimensions.
///
/// @param `client` The client to resize
/// @param `x` The x coordinate
/// @param `y` The y coordinate
/// @param `w` The width
/// @param `h` The height
void Swm__resize_client(Client *client, int x, int y, int w, int h);

/// @brief Restacks the client windows on a given monitor according to the layout.
///
/// @details Likely called on a layout change
///
/// @param `monitor` The monitor to restack
void Swm__restack(Monitor *monitor);

/// @brief Runs the main event loop.
///
/// @details This function is called after the window manager has been initialized
/// and is responsible for handling all X events and running the window manager.
void Swm__run(void);

/// @brief Updates the client state property of a client.
///
/// @param `client` The client to update the state of
/// @param `state` The state to update the client to
void Swm__set_client_state(Client *client, int64_t state);

/// @brief Sends an an event to a client if a supported protocol is found.
///
/// @param `client` The client to send the event to
/// @param `proto` The protocol to send to the client
///
/// @returns true if the client supports the protocol and the event was sent, false otherwise.
bool Swm__send_event(Client *client, Atom proto);

/// @brief Sets the focus to a given client.
///
/// @param `client` The client to set the focus to
void Swm__setfocus(Client *client);

/// @brief Sets the fullscreen state of a client.
///
/// @param `client` The client to set the fullscreen state of
/// @param `fullscreen` The fullscreen state to set
void Swm__setfullscreen(Client *client, int32_t fullscreen);

/// @brief Sets the urgent state of a client.
///
/// @param `client` The client to set the urgent state of
/// @param `urgent` The urgent state to set
void Swm__seturgent(Client *client, int urgent);

/// @brief Shows or hides a client.
///
/// @param `client` The client to show or hide
void Swm__showhide(Client *client);

/// @brief Unfocus the client.
///
/// @details The border color is set to the inactive border color.
///
/// @param `client` The client to focus/unfocus
/// @param `setfocus` Whether or not to set the focus
void Swm__unfocus(Client *client, bool setfocus);

/// @brief Unmaps a client from the window manager.
///
/// @param `client` The client to unmap
void Swm__unmanage(Client *client, bool destroyed);

/// @brief Iterates through all monitors and crates the
/// bar x window if it does not exist. No text or color is drawn,
/// this is done in `Swm__drawbar`.
void Swm__updatebars(void);

/// @brief Iterates through all clients on all monitors and updates the window properties.
///
/// @details The following X11 functions are called:
///	- XChangeProperty: https://tronche.com/gui/x/xlib/window-information/XChangeProperty.html
///	- XDeleteProperty: https://tronche.com/gui/x/xlib/window-information/XDeleteProperty.html
void Swm__update_client_list(void);

/// @brief Update the geometry of the screen
///
/// @details For each monitor, update the width and height of the monitor
/// based of the screen width and height. Then update the bar position,
/// and map the current monitor to the root window id.
bool Swm__updategeom(void);

/// @brief Updates the numlock mask
/// TODO: Better documentation
void Swm__update_numlock_mask(void);

/// @brief Update the status text in the bar
void Swm__update_status(void);

/// @brief Update the title of a client, which is displayed in the center of the bar.
void Swm__update_client_title(Client *client);

/// @brief Updates the type of window, is it floating or fullscreen.
///
/// @param `client` The client to update
void Swm__update_window_type(Client *client);

/// @brief Sets the urgency and input hint of a client.
///
/// @param `client` The client to update
void Swm__update_wmhints(Client *client);

/// @brief Maps an X window id to an existing client.
///
/// @param `w_id` The window id to map to a client
///
/// @returns The client that matches the window id, or NULL if no client was found.
Client *Swm__win_to_client(Window w_id);

/// @brief Maps an X window id to an existing monitor.
///
/// @details if the window id is the root window, return the monitor
/// where the x and y coordinates of the mouse pointer are.
/// If the w_id is the bar window, return the monitor where the bar is.
/// If the w_id is a client window, return the monitor where the client is.
/// Else just return the current active monitor.
///
/// @param `w_id` The window id to map to a monitor
Monitor *Swm__wintomon(Window w_id);

//////////////////////////////////////////////////////////////////////////////////////////
/// 						Keybind Modifier Functions
//////////////////////////////////////////////////////////////////////////////////////////

#endif
