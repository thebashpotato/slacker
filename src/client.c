
#include "client.h"
#include "monitor.h"
#include "utils.h"
#include "swm.h"

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

void Client__configure(Display *display, Client *client)
{
	XConfigureEvent ce;

	ce.type = ConfigureNotify;
	ce.display = display;
	ce.event = client->win;
	ce.window = client->win;
	ce.x = client->x;
	ce.y = client->y;
	ce.width = client->w;
	ce.height = client->h;
	ce.border_width = client->bw;
	ce.above = None;
	ce.override_redirect = False;
	XSendEvent(display, client->win, False, StructureNotifyMask,
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
	Slacker__focus(client);
	Slacker__arrange_monitors(client->mon);
}

void Client__resize(Client *client, int x, int y, int w, int h, int interact)
{
	if (Slacker__applysizehints(client, &x, &y, &w, &h, interact)) {
		Slacker__resize_client(client, x, y, w, h);
	}
}

void Client__send_to_monitor(Client *client, Monitor *target_monitor)
{
	// If the clients currently registered monitor is the same as the target monitor, return.
	if (client->mon == target_monitor) {
		return;
	}

	// Unfocus the client, detach it from the current monitor list and monitor stack.
	Slacker__unfocus(client, 1);
	Client__detach(client);
	Client__detach_from_stack(client);
	// Update the clients monitor with the new monitor it will be on.
	client->mon = target_monitor;
	// Assign tags of target monitor
	client->tags = target_monitor->tagset[target_monitor->seltags];
	// Attach the client to the new monitor list and stack, then focus,
	// and arrange all monitors
	Client__attach(client);
	Client__attach_to_stack(client);
	Slacker__focus(NULL);
	Slacker__arrange_monitors(NULL);
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
