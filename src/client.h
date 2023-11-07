#ifndef SWM_CLIENT_H
#define SWM_CLIENT_H

// X11
#include <X11/Xlib.h>

// Standard Libraries
#include <stdint.h>
#include <stdio.h>

// Slacker headers
#include "constants.h"

// forward declarations
typedef struct Monitor Monitor;

////////////////////////////////////////////////////
/// 			Helper client macros
////////////////////////////////////////////////////

/// @brief Check if a client is visible on a monitor
#define ISVISIBLE(Client) \
	((Client->tags & Client->mon->tagset[Client->mon->selected_tags]))

/// @brief Get the Width of a client + 2 * the border width
#define WIDTH(Client) ((Client)->w + 2 * (Client)->bw)

/// @brief Get the Height of a client + 2 * the border width
#define HEIGHT(Client) ((Client)->h + 2 * (Client)->bw)

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

/// @brief TODO: Implement this
void Client__delete(Client *client);

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
