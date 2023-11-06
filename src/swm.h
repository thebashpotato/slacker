#ifndef SWM_H
#define SWM_H

// X11 Libraries
#include <X11/Xatom.h>
#include <X11/Xlib.h>

// Standard Libraries
#include <stdbool.h>
#include <stdint.h>

// Slacker Headers
#include "constants.h"
#include "drawable.h"
#include "error.h"
#include "monitor.h"

/// @brief Call back error handler for X11 errors
typedef int32_t (*XErrorHandler)(Display *, XErrorEvent *);

/// @brief This structure defines the window manager.
/// It uses all other structs in the project.
typedef struct Slacker Slacker;
struct Slacker {
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
	XErrorHandler xerror_callback;
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
	/// X display
	Display *display;
	/// Drawable abstraction
	Drw *draw;
	/// Linked list of all connected monitors
	Monitor *monitor_list;
	/// Currently selected monitor
	Monitor *selected_monitor;
	/// Root window
	Window root;
	/// Window manager check window
	Window wmcheckwin;
};

extern Slacker g_slacker;

/// @brief Initialize slacker fields
void Slacker__create(void);

/// @brief Initializses and setsup all X11 functionality
void Slacker__init(void);

/// @brief Frees all memory allocated by the window manager.
///
/// @details The following resources are freed:
/// - All monitors
/// - All cursors
/// - All color schemes
/// - The check window
/// - The drawable abstraction
void Slacker__destroy(void);

/// @brief Applies X window rules to a client
///
/// @param `client` The client to apply rules to
void Slacker__applyrules(Client *client);

/// @brief Apply window size hints to a client
///
/// x, y, w, and h should all be in a Rect struct.
/// the only slacker variables that are used are screen_width and height
/// which should be moved into their own struct called Screen.
int32_t Slacker__applysizehints(Client *client, int *x, int *y, int *w, int *h,
				int interact);

/// @brief Top level arrange function. Manages n monitors.
///
/// @details If the monitor is NULL, iterate through the linked list of monitors and
/// decided whether to show or hide them.
///
/// @param `monitor` Takes a single monitor instead of a list of monitors, since a Client holds
/// a pointer to a single monitor.
void Slacker__arrange_monitors(Monitor *monitor);

/// @brief check if another window manager is running.
void Slacker__checkotherwm(void);

/// @brief	Not sure what the purpose of this is, Debugging required.
///
/// @param `dir` TODO:
Monitor *Slacker__dir_to_monitor(int dir);

/// @brief Draws the bar for a monitor.
///
/// @details TODO: Document this function in detail, and refactor.
void Slacker__drawbar(Monitor *monitor);

/// @brief Draws the bar for all monitors.
void Slacker__drawbars(void);

/// @brief Focuses on a client
///
/// @param `client` The client to focus on
void Slacker__focus(Client *client);

/// @brief Get the atom property for a given client.
///
/// @details TODO: This should be a Client__ function
/// @param `client` The client to get the atom prop for
Atom Slacker__get_atom_prop(Client *client, Atom prop);

/// @brief Gets the pointer coordinates relative to the root window's origin.
///
/// @param `root_x_return` Pointer to the root x coordinate
/// @param `root_y_return` Pointer to the root y coordinate
///
/// @return BadWindow if the window is invalid
int32_t Slacker__getrootptr(int *root_x_return, int *root_y_return);

/// @brief Gets the state of a given window and frees it via Xlib.
///
/// @param `wid` The window to get the state of and free
int64_t Slacker__getstate(Window w_id);

/// @brief Get the text property for a given window id.
/// @details Takes a window id instead of a Client, because it is used for the root window as well.
///
/// @param `w_id` The window id to get the text prop for
/// @param `atom` The atom to get the text prop for
/// @param `text` Stores the text property name to be used
/// @param `size` The size of the text property name
///
/// @returns 1 if the text property was found, 0 otherwise
bool Slacker__get_text_prop(Window w_id, Atom atom, char *text, uint32_t size);

/// @brief Checks to see if any of the supported button masks were pressed
/// on a client window.
///
/// @param `client` The client to check for button presses on
/// @param `focused` Whether or not the client should be focused
void Slacker__grab_buttons(Client *client, bool focused);

/// @brief Sets up all the custom key bindings defined in GLOBAL_KEYBINDINGS
///
/// @details This function is called once at startup, and again on mappping notify events.
void Slacker__grab_keys(void);

/// @brief Creates a new client window and manages it.
///
/// @param `w` The window to manage
/// @param `wa` The window attributes of the window to manage
void Slacker__manage(Window w_id, XWindowAttributes *wa);

/// @brief Transforms coordinates, width and height to the monitor they are on.
///
/// @param `x` The x coordinate
/// @param `y` The y coordinate
/// @param `w` The width
/// @param `h` The height
///
/// @return The monitor the coordinates, width and height are on.
Monitor *Slacker__rect_to_monitor(int x, int y, int w, int h);

/// @brief Resizes a client with the given dimensions.
///
/// @param `client` The client to resize
/// @param `x` The x coordinate
/// @param `y` The y coordinate
/// @param `w` The width
/// @param `h` The height
void Slacker__resize_client(Client *client, int x, int y, int w, int h);

/// @brief Restacks the client windows on a given monitor according to the layout.
///
/// @details Likely called on a layout change
///
/// @param `monitor` The monitor to restack
void Slacker__restack(Monitor *monitor);

/// @brief Runs the main event loop.
///
/// @details This function is called after the window manager has been initialized
/// and is responsible for handling all X events and running the window manager.
void Slacker__run(void);

/// @brief Remove all child and transient windows from the window manager.
///
/// @details Runs before slacker starts, this is to ensure that no windows
/// from previous window managers are still lurking about.
void Slacker__scan(void);

/// @brief Updates the client state property of a client.
///
/// @param `client` The client to update the state of
/// @param `state` The state to update the client to
void Slacker__set_client_state(Client *client, int64_t state);

/// @brief Sends an an event to a client if a supported protocol is found.
///
/// @param `client` The client to send the event to
/// @param `proto` The protocol to send to the client
///
/// @returns true if the client supports the protocol and the event was sent, false otherwise.
bool Slacker__send_event(Client *client, Atom proto);

/// @brief Sets the focus to a given client.
///
/// @param `client` The client to set the focus to
void Slacker__setfocus(Client *client);

/// @brief Sets the fullscreen state of a client.
///
/// @param `client` The client to set the fullscreen state of
/// @param `fullscreen` The fullscreen state to set
void Slacker__setfullscreen(Client *client, int32_t fullscreen);

/// @brief Sets the urgent state of a client.
///
/// @param `client` The client to set the urgent state of
/// @param `urgent` The urgent state to set
void Slacker__seturgent(Client *client, int urgent);

/// @brief Shows or hides a client.
///
/// @param `client` The client to show or hide
void Slacker__showhide(Client *client);

/// @brief Unfocus the client.
///
/// @details The border color is set to the inactive border color.
///
/// @param `client` The client to focus/unfocus
/// @param `setfocus` Whether or not to set the focus
void Slacker__unfocus(Client *client, bool setfocus);

/// @brief Unmaps a client from the window manager.
///
/// @param `client` The client to unmap
void Slacker__unmanage(Client *client, bool destroyed);

/// @brief Iterates through all monitors and crates the
/// bar x window if it does not exist. No text or color is drawn,
/// this is done in `Slacker__drawbar`.
void Slacker__updatebars(void);

/// @brief Iterates through all clients on all monitors and updates the window properties.
///
/// @details The following X11 functions are called:
///	- XChangeProperty: https://tronche.com/gui/x/xlib/window-information/XChangeProperty.html
///	- XDeleteProperty: https://tronche.com/gui/x/xlib/window-information/XDeleteProperty.html
void Slacker__update_client_list(void);

/// @brief Update the geometry of the screen
///
/// @details For each monitor, update the width and height of the monitor
/// based of the screen width and height. Then update the bar position,
/// and map the current monitor to the root window id.
bool Slacker__updategeom(void);

/// @brief Updates the numlock mask
/// TODO: Better documentation
void Slacker__update_numlock_mask(void);

/// @brief Update the status text in the bar
void Slacker__update_status(void);

/// @brief Update the title of a client, which is displayed in the center of the bar.
void Slacker__update_client_title(Client *client);

/// @brief Updates the type of window, is it floating or fullscreen.
///
/// @param `client` The client to update
void Slacker__update_window_type(Client *client);

/// @brief Sets the urgency and input hint of a client.
///
/// @param `client` The client to update
void Slacker__update_wmhints(Client *client);

/// @brief X11 error handler
///
/// @details There's no way to check accesses to destroyed windows, thus those cases are
/// ignored (especially on UnmapNotify's). Other types of errors call Xlibs
/// default error handler, which may call exit.
int32_t Slacker__xerror_handler(Display *display, XErrorEvent *ee);

/// @brief Maps an X window id to an existing client.
///
/// @param `w_id` The window id to map to a client
///
/// @returns The client that matches the window id, or NULL if no client was found.
Client *Slacker__win_to_client(Window w_id);

/// @brief Maps an X window id to an existing monitor.
///
/// @details if the window id is the root window, return the monitor
/// where the x and y coordinates of the mouse pointer are.
/// If the w_id is the bar window, return the monitor where the bar is.
/// If the w_id is a client window, return the monitor where the client is.
/// Else just return the current active monitor.
///
/// @param `w_id` The window id to map to a monitor
Monitor *Slacker__wintomon(Window w_id);

///////////////////////////////////////////////////////////////////////////////////////
/// 						Event Handler Functions
///////////////////////////////////////////////////////////////////////////////////////

/// @brief Handles matching on and dispatching X11 events
///
/// @details Slacker supports the following X11 events:
/// - ButtonPress
/// - ClientMessage
/// - ConfigureRequest
/// - ConfigureNotify
/// - DestroyNotify
/// - EnterNotify
/// - Expose
/// - FocusIn
/// - KeyPress
/// - MappingNotify
/// - MapRequest
/// - MotionNotify
/// - PropertyNotify
/// - UnmapNotify
///
/// @param `event` The X11 event to handle
void Slacker__event_loop(XEvent *event);

//////////////////////////////////////////////////////////////////////////////////////////
/// 						Keybind Modifier Functions
//////////////////////////////////////////////////////////////////////////////////////////

/// @brief Kills the currently selected client.
void Slacker__kill_client(const Arg *arg);

/// @brief Increments master window in the stacker area by (+ or 1 n)
void Slacker__increment_n_master(const Arg *arg);

/// @brief Confusing code. I think this function is used to move a window via the mouse.
///
/// @details TODO: Refactor this function, it is confusing and hard to read.
void Slacker__move_with_mouse(const Arg *arg);

/// @brief Shutdown the window manager
void Slacker__quit(const Arg *arg);

/// @brief Adds support for resizing a client with the mouse.
///
/// @details Does not support resizing fullscreen windows, because this is a tiling window manager.
///
/// @param `arg` The argument to pass to the function
void Slacker__resize_client_with_mouse(const Arg *arg);

/// @brief Sets the layout of a monitor.
///
/// @param `arg` The argument to pass to the function, the void * is a Layout *
void Slacker__setlayout(const Arg *arg);

/// @brief arg > 1.0 will set mfact absolutely
///
/// @param `arg` The argument to pass to the function
void Slacker__setmfact(const Arg *arg);

/// @brief Helper function to spawn shell commands
///
/// @param `arg` The argument Union to pass to the function, the void * is a char *
void Slacker__spawn(const Arg *arg);

/// @brief Uses the arg.ui field as a bitmask to toggle the tag of a client.
///
/// @param `arg` The argument Union to pass to the function, uses the ui field as a bitmask
void Slacker__tag(const Arg *arg);

/// @brief Not sure what this does, debug
void Slacker__tagmon(const Arg *arg);

/// @brief Toggle the bar on the selected monitor.
///
/// @param `arg` Unused argument
void Slacker__togglebar(const Arg *arg);

/// @brief Toggle the floating state of a client.
///
/// @param `arg` Unused
void Slacker__togglefloating(const Arg *arg);

/// @brief Called when a tag is clicked, or a keybinding is pressed to change to that tag view.
///
/// @param `arg` Unused
void Slacker__toggletag(const Arg *arg);

/// @brief Change the view to a new tag, or monitor.
///
/// @param `arg` Unused
void Slacker__toggleview(const Arg *arg);

/// @brief Changes view to a new tag/monitor
///
/// @param `arg` Uses the ui field as a bitmask to change to the proper tag.
void Slacker__view(const Arg *arg);

/// @brief TODO: Document
void Slacker__zoom(const Arg *arg);

/// @brief Focuses on the next client in the stack.
void Slacker__focus_stack(const Arg *arg);

/// @brief When a user changed to a different monitor, this function is called.
void Slacker__focus_monitor(const Arg *arg);

#endif
