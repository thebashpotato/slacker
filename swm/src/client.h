#ifndef SWM_CLIENT_H
#define SWM_CLIENT_H

// X11
#include <X11/Xlib.h>
#include <X11/X.h>

// Standard Libraries
#include <bits/stdint-intn.h>
#include <bits/stdint-uintn.h>
#include <stdbool.h>

// Slacker headers
#include "constants.h"

// NOTE: This is a forward declaration of the Monitor struct, as including
// the monitor header here would cause a circular dependency.
typedef struct Monitor Monitor;

/// @brief Represents an X client window
typedef struct Client Client;

/// @brief Callback function type for Client debugging information
typedef void (*ClientLogFunction)(Client *);

struct Client {
	char name[MAX_CLIENT_NAME_LEN];
	float mina, maxa;
	int32_t x, y, w, h;
	int32_t oldx, oldy, oldw, oldh;
	int32_t basew, baseh, incw, inch, maxw, maxh, minw, minh, hintsvalid;
	/// border width, old border width
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
	ClientLogFunction log;
};

/// @brief Builds a new client
///
/// @param `w_id` The window id to inialize the client with
/// @param `wa` The XWindowAttributes struct gives us the initial window geometry
/// @param `monitor` The monitor the client is being created on
Client *Client__new(Window w_id, XWindowAttributes *wa, Monitor *monitor);

/// @brief Detatches a client from the monitor stack list and the monitor linked list.
///
/// @param `client` The client to add to the monitor's client list.
void Client__delete(Client *client);

/// @brief Updates a single clients x, y, w, and h dimensions.
///
/// @details Used to apply window gaps
XWindowChanges Client__update_dimensions(Client *client, int32_t x, int32_t y,
					 int32_t w, int32_t h);

/// @brief Checks to see if this client is safe to modify or manipulate
///
/// @details This means to check if the client is not floating, and if the layout handler function
/// for this clients monitor is not null. Currently used when resizing a client to apply gaps and window changes.
bool Client__safe_to_modify(Client *client);

/// @brief Adds a client to the front of a monitor's client list.
///
/// @param `client` The client to add to the monitor's client list.
void Client__attach(Client *client);

/// @brief Adds a client to the front of a monitor's stack list.
///
/// @param `client` The client to add to the monitor's stack list.
void Client__attach_to_stack(Client *client);

/// @brief Used by the ConfigureRequest event handler to configure a client.
///
/// @details Builds an XConfigureEvent struct and sents it to the client via its window id.
void Client__configure(Display *xconn, Client *client);

/// @brief Get the next tiled and visible client in the client linked list.
Client *Client__next_tiled(Client *client);

/// @brief Detaches a client from a monitor's client list.
void Client__detach(Client *client);

/// @brief Detaches a client from a monitor's stack list.
void Client__detach_from_stack(Client *client);

/// @brief Removes a client from the client list.
void Client__pop(Client *client);

/// @brief Applies size hints to a given client.
void Client__resize(Client *client, int x, int y, int w, int h, int interact);

/// @brief Sends a client to a differnt monitor.
///
/// @param `client` The client to send to a different monitor
/// @param `target_monitor` The monitor to send the client to
void Client__send_to_monitor(Client *client, Monitor *monitor);

void Client__update_size_hints(Display *display, Client *client);

#endif
