
// X11 Libraries
#include <X11/Xlib.h>

// Standard Libraries
#include <stdio.h>
#include <string.h>

// Slacker Headers
#include "client.h"
#include "config.h"
#include "monitor.h"
#include "utils.h"
#include "swm.h"

/// @brief Allows a client to log itself to stdout
///
/// @param `client` The client to log: client->log(client)
static void Client__log(Client *this)
{
#if (DEBUG == 1)
	fprintf(stdout, "\nClient: %li\n", this->win);
	fprintf(stdout, "Name: %s\n", this->name);
	fprintf(stdout, "X: %d, Y: %i\n", this->x, this->y);
	fprintf(stdout, "W: %d, H: %i\n", this->w, this->h);
	fprintf(stdout, "Tag(s): %d\n", this->tags);
#endif
}

Client *Client__new(Window w_id, XWindowAttributes *wa, Monitor *monitor)
{
	Client *c = ecalloc(1, sizeof(Client));
	strcpy(c->name, "");
	c->mina = 0.0;
	c->maxa = 0.0;
	c->x = c->oldx = wa->x;
	c->y = c->oldy = wa->y;
	c->w = c->oldw = wa->width;
	c->h = c->oldh = wa->height;
	c->oldbw = wa->border_width;
	c->bw = G_BORDER_PIXEL;
	c->tags = 0;
	c->isfixed = 0;
	c->isfloating = 0;
	c->isurgent = 0;
	c->neverfocus = 0;
	c->oldstate = 0;
	c->isfullscreen = 0;
	c->next = NULL;
	c->stack_next = NULL;
	c->mon = monitor;
	c->win = w_id;
	c->log = Client__log;

	if (c->x + WIDTH(c) > c->mon->wx + c->mon->ww) {
		c->x = c->mon->wx + c->mon->ww - WIDTH(c);
	}

	if (c->y + HEIGHT(c) > (c->mon->wy + c->mon->wh)) {
		c->y = c->mon->wy + c->mon->wh - HEIGHT(c);
	}

	c->x = MAX(c->x, c->mon->wx);
	c->y = MAX(c->y, c->mon->wy);

	return c;
}

void Client__delete(Client *client)
{
	if (client) {
		Client__detach(client);
		Client__detach_from_stack(client);
		free(client);
	}
}

void Client__attach(Client *client)
{
	client->next = client->mon->client_list;
	client->mon->client_list = client;
}

void Client__attach_to_stack(Client *client)
{
	client->stack_next = client->mon->client_stack;
	client->mon->client_stack = client;
}

void Client__configure(Display *xconn, Client *client)
{
	XConfigureEvent ce;

	ce.type = ConfigureNotify;
	ce.display = xconn;
	ce.event = client->win;
	ce.window = client->win;
	ce.x = client->x;
	ce.y = client->y;
	ce.width = client->w;
	ce.height = client->h;
	ce.border_width = client->bw;
	ce.above = None;
	ce.override_redirect = False;
	XSendEvent(xconn, client->win, False, StructureNotifyMask,
		   (XEvent *)&ce);
}

Client *Client__next_tiled(Client *client)
{
	for (; client && (client->isfloating || !ISVISIBLE(client));
	     client = client->next) {
		;
	}
	return client;
}

void Client__detach(Client *client)
{
	Client **tc;

	for (tc = &client->mon->client_list; *tc && *tc != client;
	     tc = &(*tc)->next) {
		;
	}
	*tc = client->next;
}

void Client__detach_from_stack(Client *client)
{
	Client **tc, *t;

	for (tc = &client->mon->client_stack; *tc && *tc != client;
	     tc = &(*tc)->stack_next) {
		;
	}

	*tc = client->stack_next;

	if (client == client->mon->selected_client) {
		for (t = client->mon->client_stack; t && !ISVISIBLE(t);
		     t = t->stack_next) {
			;
		}
		client->mon->selected_client = t;
	}
}

void Client__pop(Client *client)
{
	Client__detach(client);
	Client__attach(client);
	Swm__focus(client);
	Swm__arrange_monitors(client->mon);
}

void Client__resize(Client *client, int x, int y, int w, int h, int interact)
{
	if (Swm__applysizehints(client, &x, &y, &w, &h, interact)) {
		Swm__resize_client(client, x, y, w, h);
	}
	client->log(client);
}

void Client__send_to_monitor(Client *client, Monitor *target_monitor)
{
	// If the clients currently registered monitor is the same as the target monitor, return.
	if (client->mon == target_monitor) {
		return;
	}

	// Unfocus the client, detach it from the current monitor list and monitor stack.
	Swm__unfocus(client, 1);
	Client__detach(client);
	Client__detach_from_stack(client);
	// Update the clients monitor with the new monitor it will be on.
	client->mon = target_monitor;
	// Assign tags of target monitor
	client->tags = target_monitor->tagset[target_monitor->selected_tags];
	// Attach the client to the new monitor list and stack, then focus,
	// and arrange all monitors
	Client__attach(client);
	Client__attach_to_stack(client);
	Swm__focus(NULL);
	Swm__arrange_monitors(NULL);
}

void Client__update_size_hints(Display *display, Client *client)
{
	int64_t msize = 0;
	XSizeHints x_size_hints;

	if (!XGetWMNormalHints(display, client->win, &x_size_hints, &msize)) {
		// Size is uninitialized, ensure that size.flags aren't used
		x_size_hints.flags = PSize;
	}

	if (x_size_hints.flags & PBaseSize) {
		client->basew = x_size_hints.base_width;
		client->baseh = x_size_hints.base_height;
	} else if (x_size_hints.flags & PMinSize) {
		client->basew = x_size_hints.min_width;
		client->baseh = x_size_hints.min_height;
	} else {
		client->basew = client->baseh = 0;
	}

	if (x_size_hints.flags & PResizeInc) {
		client->incw = x_size_hints.width_inc;
		client->inch = x_size_hints.height_inc;
	} else {
		client->incw = client->inch = 0;
	}

	if (x_size_hints.flags & PMaxSize) {
		client->maxw = x_size_hints.max_width;
		client->maxh = x_size_hints.max_height;
	} else {
		client->maxw = client->maxh = 0;
	}

	if (x_size_hints.flags & PMinSize) {
		client->minw = x_size_hints.min_width;
		client->minh = x_size_hints.min_height;
	} else if (x_size_hints.flags & PBaseSize) {
		client->minw = x_size_hints.base_width;
		client->minh = x_size_hints.base_height;
	} else {
		client->minw = client->minh = 0;
	}

	if (x_size_hints.flags & PAspect) {
		client->mina = (float)x_size_hints.min_aspect.y /
			       x_size_hints.min_aspect.x;
		client->maxa = (float)x_size_hints.max_aspect.x /
			       x_size_hints.max_aspect.y;
	} else {
		client->maxa = client->mina = 0.0;
	}

	client->isfixed =
		(client->maxw && client->maxh && client->maxw == client->minw &&
		 client->maxh == client->minh);
	client->hintsvalid = 1;
}
