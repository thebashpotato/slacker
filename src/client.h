#ifndef SWM_CLIENT_H
#define SWM_CLIENT_H

// X11
#include <X11/Xlib.h>

// Standard Libraries
#include <stdint.h>

// Slacker headers
#include "constants.h"

// forward declarations
typedef struct Monitor Monitor;

/// @brief Represents an X client window
typedef struct Client Client;
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
};

/// @brief TODO: Implement this
Client *Client__create(Window window, XWindowAttributes *window_attributes);

/// @brief TODO: Implement this
void Client__destroy(Client *client);

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
void Client__configure(Display *display, Client *client);

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
