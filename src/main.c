/// See LICENSE file for copyright and license details.
///
/// Slacker is designed like any other X client. It is
/// driven through handling X events. In contrast to other X clients, a window
/// manager selects for SubstructureRedirectMask on the root window, to receive
/// events about window (dis-)appearance. Only one X connection at a time is
/// allowed to select for this event mask.
///
/// Each child of the root window is called a client, except windows which have
/// set the override_redirect flag. Clients are organized in a linked client
/// list on each monitor, the focus history is remembered through a stack list
/// on each monitor. Each client contains a bit array to indicate the tags of a
/// client.
///
/// Keys and tagging WINDOW_RULES are organized as arrays and defined in config.c
///
/// To understand everything else, start reading main().

// X11 Libraries
#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>

// Standard Libraries
#include <bits/stdint-intn.h>
#include <bits/stdint-uintn.h>
#include <errno.h>
#include <locale.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Slacker Headers
#include "config.h"
#include "constants.h"
#include "drawable.h"
#include "events.h"
#include "modifiers.h"
#include "monitor.h"
#include "utils.h"
#include "slacker.h"

//////////////////////////////////////////
/// Window Mangager specific macros
//////////////////////////////////////////

#define BUTTONMASK (ButtonPressMask | ButtonReleaseMask)

#define CLEANMASK(mask)                                              \
	(mask & ~(g_slacker.numlockmask | LockMask) &                \
	 (ShiftMask | ControlMask | Mod1Mask | Mod2Mask | Mod3Mask | \
	  Mod4Mask | Mod5Mask))

#define INTERSECT(x, y, w, h, m)                                         \
	(MAX(0, MIN((x) + (w), (m)->wx + (m)->ww) - MAX((x), (m)->wx)) * \
	 MAX(0, MIN((y) + (h), (m)->wy + (m)->wh) - MAX((y), (m)->wy)))

#define ISVISIBLE(Client) \
	((Client->tags & Client->mon->tagset[Client->mon->seltags]))

#define MOUSEMASK (BUTTONMASK | PointerMotionMask)

#define WIDTH(X) ((X)->w + 2 * (X)->bw)

#define HEIGHT(X) ((X)->h + 2 * (X)->bw)

#define TAGMASK ((1 << LENGTH(GLOBAL_TAGS)) - 1)

#define TEXTW(X)                                     \
	(drw_fontset_getwidth(g_slacker.draw, (X)) + \
	 g_slacker.left_right_padding_sum)

//////////////////////
/// Global variables
//////////////////////

/// Client text which displays in the bar when the client is broken
static const char BROKEN[] = "broken";

/// @brief compile-time check if all tags fit into an unsigned int bit array
struct NumTags {
	uint32_t limitexceeded[LENGTH(GLOBAL_TAGS) > 31 ? -1 : 1];
};

/////////////////////////////
/// Function declarations
/////////////////////////////

static void Slacker__applyrules(Client *client);
static int32_t Slacker__applysizehints(Client *client, int *x, int *y, int *w,
				       int *h, int interact);
static void Slacker__arrange_monitors(Monitor *monitor);
static void Monitor__arrange(Monitor *monitor);
static void Client__attach(Client *client);
static void Client__attach_to_stack(Client *client);
static void Slacker__checkotherwm(void);
static void Slacker__destroy(void);
static void Monitor__destroy(Monitor *monitor);
static void Client__configure(Client *client);
static Monitor *Monitor__create(void);
static void Client__detach(Client *client);
static void Client__detach_from_stack(Client *client);
static Monitor *Slacker__dir_to_monitor(int dir);
static void Slacker__drawbar(Monitor *monitor);
static void Slacker__drawbars(void);
static void Slacker__focus(Client *client);
static Atom Slacker__get_atom_prop(Client *client, Atom prop);
static int32_t Slacker__getrootptr(int *x, int *y);
static int64_t Slacker__getstate(Window w_id);
static int32_t Slacker__get_text_prop(Window w_id, Atom atom, char *text,
				      uint32_t size);
static void Slacker__grab_buttons(Client *client, bool focused);
static void Slacker__grab_keys(void);
static void Slacker__manage(Window w_id, XWindowAttributes *wa);
static Client *Client__next_tiled(Client *client);
static void Client__pop(Client *client);
static Monitor *Slacker__rect_to_monitor(int x, int y, int w, int h);
static void Client__resize(Client *client, int x, int y, int w, int h,
			   int interact);
static void Slacker__resize_client(Client *client, int x, int y, int w, int h);
static void Slacker__restack(Monitor *monitor);
static void Slacker__run(void);
static void Slacker__scan(void);

/// @brief Sends an an event to a client if a supported protocol is found.
///
/// @param `client` The client to send the event to
/// @param `proto` The protocol to send to the client
///
/// @returns true if the client supports the protocol and the event was sent, false otherwise.
static bool Slacker__send_event(Client *client, Atom proto);
static void Client__send_to_monitor(Client *client, Monitor *monitor);
static void Slacker__set_client_state(Client *client, int64_t state);
static void Slacker__setfocus(Client *client);
static void Slacker__setfullscreen(Client *client, int fullscreen);
static void Slacker__create(void);
static void Slacker__seturgent(Client *client, int urgent);
static void Slacker__showhide(Client *client);
static void Slacker__unfocus(Client *client, int setfocus);
static void Slacker__unmanage(Client *client, int destroyed);
static void Monitor__updatebarpos(Monitor *monitor);
static void Slacker__updatebars(void);
static void Slacker__update_client_list(void);
static bool Slacker__updategeom(void);
static void Slacker__update_numlock_mask(void);
static void updatesizehints(Client *client);
static void Slacker__update_status(void);
static void Slacker__update_client_title(Client *client);
static void Slacker__update_window_type(Client *client);
static void Slacker__update_wmhints(Client *client);
static Client *Slacker__win_to_client(Window w_id);
static Monitor *Slacker__wintomon(Window w_id);
static int xerror(Display *dpy, XErrorEvent *ee);
static int xerrordummy(Display *dpy, XErrorEvent *ee);
static int xerrorstart(Display *dpy, XErrorEvent *ee);

///////////////////////////////
/// Function implementations
///////////////////////////////

/// @brief Applies X window rules to a client
void Slacker__applyrules(Client *client)
{
	const char *class = NULL;
	const char *instance = NULL;
	const WindowRule *window_rule = NULL;
	Monitor *temp_monitor = NULL;
	XClassHint ch = { NULL, NULL };

	// Rule matching
	client->isfloating = 0;
	client->tags = 0;
	XGetClassHint(g_slacker.display, client->win, &ch);
	class = ch.res_class ? ch.res_class : BROKEN;
	instance = ch.res_name ? ch.res_name : BROKEN;

	for (uint32_t i = 0; i < LENGTH(GLOBAL_WINDOW_RULES); ++i) {
		window_rule = &GLOBAL_WINDOW_RULES[i];
		if ((!window_rule->title ||
		     strstr(client->name, window_rule->title)) &&
		    (!window_rule->window_class ||
		     strstr(class, window_rule->window_class)) &&
		    (!window_rule->instance ||
		     strstr(instance, window_rule->instance))) {
			client->isfloating = window_rule->isfloating;
			client->tags |= window_rule->tags;
			for (temp_monitor = g_slacker.monitor_list;
			     temp_monitor &&
			     temp_monitor->num != window_rule->monitor;
			     temp_monitor = temp_monitor->next) {
				;
			}
			if (temp_monitor) {
				client->mon = temp_monitor;
			}
		}
	}

	if (ch.res_class) {
		XFree(ch.res_class);
	}

	if (ch.res_name) {
		XFree(ch.res_name);
	}

	client->tags = client->tags & TAGMASK ?
			       client->tags & TAGMASK :
			       client->mon->tagset[client->mon->seltags];
}

/// @brief Apply window size hints to a client
///
/// TODO: This function is a mess and needs to be cleaned up.
/// x, y, w, and h should all be in a Rect struct.
/// the only slacker variables that are used are screen_width and height
/// which should be moved into their own struct called Screen.
int Slacker__applysizehints(Client *client, int *x, int *y, int *w, int *h,
			    int interact)
{
	int32_t baseismin = 0;
	Monitor *temp_monitor = client->mon;

	// set minimum possible
	*w = MAX(1, *w);
	*h = MAX(1, *h);

	if (interact) {
		if (*x > g_slacker.screen_width) {
			*x = g_slacker.screen_width - WIDTH(client);
		}
		if (*y > g_slacker.screen_height) {
			*y = g_slacker.screen_height - HEIGHT(client);
		}
		if (*x + *w + 2 * client->bw < 0) {
			*x = 0;
		}
		if (*y + *h + 2 * client->bw < 0) {
			*y = 0;
		}
	} else {
		if (*x >= temp_monitor->wx + temp_monitor->ww) {
			*x = temp_monitor->wx + temp_monitor->ww -
			     WIDTH(client);
		}
		if (*y >= temp_monitor->wy + temp_monitor->wh) {
			*y = temp_monitor->wy + temp_monitor->wh -
			     HEIGHT(client);
		}
		if (*x + *w + 2 * client->bw <= temp_monitor->wx) {
			*x = temp_monitor->wx;
		}
		if (*y + *h + 2 * client->bw <= temp_monitor->wy) {
			*y = temp_monitor->wy;
		}
	}

	if (*h < g_slacker.bar_height) {
		*h = g_slacker.bar_height;
	}
	if (*w < g_slacker.bar_height) {
		*w = g_slacker.bar_height;
	}

	if (GLOBAL_RESIZE_HINTS || client->isfloating ||
	    !client->mon->layouts[client->mon->selected_layout]
		     ->layout_arrange_callback) {
		if (!client->hintsvalid) {
			updatesizehints(client);
		}

		// See last two sentences in ICCCM 4.1.2.3
		baseismin = client->basew == client->minw &&
			    client->baseh == client->minh;

		// Temporarily remove base dimensions
		if (!baseismin) {
			*w -= client->basew;
			*h -= client->baseh;
		}

		// adjust for aspect limits
		if (client->mina > 0 && client->maxa > 0) {
			if (client->maxa < (float)*w / *h) {
				*w = *h * client->maxa + 0.5;
			} else if (client->mina < (float)*h / *w) {
				*h = *w * client->mina + 0.5;
			}
		}
		// The increment caclulation requires this
		if (baseismin) {
			*w -= client->basew;
			*h -= client->baseh;
		}

		// Adjust for increment value
		if (client->incw) {
			*w -= *w % client->incw;
		}

		if (client->inch) {
			*h -= *h % client->inch;
		}

		// Restore base dimensions
		*w = MAX(*w + client->basew, client->minw);
		*h = MAX(*h + client->baseh, client->minh);

		if (client->maxw) {
			*w = MIN(*w, client->maxw);
		}

		if (client->maxh) {
			*h = MIN(*h, client->maxh);
		}
	}
	return *x != client->x || *y != client->y || *w != client->w ||
	       *h != client->h;
}

/// @brief Top level arrange function. Manages n monitors.
///
/// @details If the monitor is NULL, iterate through the linked list of monitors and
/// decided whether to show or hide them.
///
/// @param `monitor` Takes a single monitor instead of a list of monitors, since a Client holds
/// a pointer to a single monitor.
void Slacker__arrange_monitors(Monitor *monitor)
{
	if (monitor) {
		Slacker__showhide(monitor->client_stack);
	} else {
		for (monitor = g_slacker.monitor_list; monitor;
		     monitor = monitor->next) {
			Slacker__showhide(monitor->client_stack);
		}
	}

	if (monitor) {
		Monitor__arrange(monitor);
		Slacker__restack(monitor);
	} else {
		for (monitor = g_slacker.monitor_list; monitor;
		     monitor = monitor->next) {
			Monitor__arrange(monitor);
		}
	}
}

/// @brief Updates the layout symbol, then calls the layout's arrange function
/// for the given monitor.
void Monitor__arrange(Monitor *monitor)
{
	strncpy(monitor->layout_symbol,
		monitor->layouts[monitor->selected_layout]->symbol,
		sizeof(monitor->layout_symbol));

	if (monitor->layouts[monitor->selected_layout]->layout_arrange_callback) {
		monitor->layouts[monitor->selected_layout]
			->layout_arrange_callback(monitor);
	}
}

/// @brief Adds a client to the front of a monitor's client list.
void Client__attach(Client *client)
{
	client->next = client->mon->client_list;
	client->mon->client_list = client;
}

/// @brief Adds a client to the front of a monitor's stack list.
void Client__attach_to_stack(Client *client)
{
	client->stack_next = client->mon->client_stack;
	client->mon->client_stack = client;
}

/// @brief Button press event handler
///
/// @param `event` The X event context
void event_buttonpress(XEvent *event)
{
	uint32_t i = 0;
	uint32_t x = 0;
	uint32_t click = SlackerClick_RootWin;
	Arg arg = { 0 };
	Client *temp_client = NULL;
	Monitor *temp_monitor = NULL;
	XButtonPressedEvent *ev = &event->xbutton;

	// Focus monitor if necessary
	if ((temp_monitor = Slacker__wintomon(ev->window)) &&
	    temp_monitor != g_slacker.selected_monitor) {
		Slacker__unfocus(g_slacker.selected_monitor->selected_client,
				 1);
		g_slacker.selected_monitor = temp_monitor;
		Slacker__focus(NULL);
	}

	// Check if the button press was a click on the bar
	if (ev->window == g_slacker.selected_monitor->barwin) {
		do {
			x += TEXTW(GLOBAL_TAGS[i]);
		} while (ev->x >= x && ++i < LENGTH(GLOBAL_TAGS));

		if (i < LENGTH(GLOBAL_TAGS)) {
			click = SlackerClick_TagBar;
			arg.ui = 1 << i;
		} else if (ev->x < x + TEXTW(g_slacker.selected_monitor
						     ->layout_symbol)) {
			click = SlackerClick_LtSymbol;
		} else if (ev->x > g_slacker.selected_monitor->ww -
					   (int)TEXTW(g_slacker.status_text)) {
			click = SlackerClick_StatusText;
		} else {
			click = SlackerClick_WinTitle;
		}
	}

	if ((temp_client = Slacker__win_to_client(ev->window))) {
		Slacker__focus(temp_client);
		Slacker__restack(g_slacker.selected_monitor);
		XAllowEvents(g_slacker.display, ReplayPointer, CurrentTime);
		click = SlackerClick_ClientWin;
	}

	// Check to see if we have a button handler for the click we have registered.
	for (i = 0; i < LENGTH(GLOBAL_CLICKABLE_BUTTONS); ++i) {
		// If we have a match, the callback is not a null function,
		// and
		if (click == GLOBAL_CLICKABLE_BUTTONS[i].click &&
		    GLOBAL_CLICKABLE_BUTTONS[i].button_handler_callback &&
		    GLOBAL_CLICKABLE_BUTTONS[i].button == ev->button &&
		    CLEANMASK(GLOBAL_CLICKABLE_BUTTONS[i].mask) ==
			    CLEANMASK(ev->state)) {
			GLOBAL_CLICKABLE_BUTTONS[i].button_handler_callback(
				click == SlackerClick_TagBar &&
						GLOBAL_CLICKABLE_BUTTONS[i]
								.arg.i == 0 ?
					&arg :
					&GLOBAL_CLICKABLE_BUTTONS[i].arg);
		}
	}
}

/// @brief check if another window manager is running.
void Slacker__checkotherwm(void)
{
	g_slacker.xerrorxlib = XSetErrorHandler(xerrorstart);
	// This will cause an X11 error to be triggered if another window manager is running.
	// The error is handlered by the slacker custom xerrorxlib function.
	XSelectInput(g_slacker.display, DefaultRootWindow(g_slacker.display),
		     SubstructureRedirectMask);
	XSync(g_slacker.display, False);
	XSetErrorHandler(xerror);
	XSync(g_slacker.display, False);
}

/// @brief Frees all memory allocated by the window manager.
///
/// @details The following resources are freed:
/// - All monitors
/// - All cursors
/// - All color schemes
/// - The check window
/// - The drawable abstraction
void Slacker__destroy(void)
{
	Arg a = { .ui = ~0 };
	Layout foo = { "", NULL };
	Monitor *temp_monitor = NULL;

	Slacker__view(&a);
	g_slacker.selected_monitor
		->layouts[g_slacker.selected_monitor->selected_layout] = &foo;

	// iterate through all monitors and unmanage all client windows
	for (temp_monitor = g_slacker.monitor_list; temp_monitor;
	     temp_monitor = temp_monitor->next) {
		while (temp_monitor->client_stack) {
			Slacker__unmanage(temp_monitor->client_stack, 0);
		}
	}

	// force ungrabbing of any key presses on the root window
	XUngrabKey(g_slacker.display, AnyKey, AnyModifier, g_slacker.root);

	// free all monitors
	while (g_slacker.monitor_list) {
		Monitor__destroy(g_slacker.monitor_list);
	}

	// free all cursors
	for (size_t i = 0; i < SlackerCursorState_Last; ++i) {
		drw_cur_free(g_slacker.draw, g_slacker.cursor[i]);
	}

	// free all color schemes
	for (size_t i = 0; i < LENGTH(GLOBAL_COLORSCHEMES); ++i) {
		if (g_slacker.scheme[i]) {
			free(g_slacker.scheme[i]);
		}
	}
	if (g_slacker.scheme) {
		free(g_slacker.scheme);
	}

	// Free the check window
	XDestroyWindow(g_slacker.display, g_slacker.wmcheckwin);

	// Free the drawable abstraction
	drw_free(g_slacker.draw);

	XSync(g_slacker.display, False);
	XSetInputFocus(g_slacker.display, PointerRoot, RevertToPointerRoot,
		       CurrentTime);
	XDeleteProperty(g_slacker.display, g_slacker.root,
			g_slacker.netatom[SlackerEWMHAtom_NetActiveWindow]);
}

/// @brief Destroys a monitor and frees all memory allocated to it.
///
/// @details Also unmaps the bar window and destroys it.
void Monitor__destroy(Monitor *monitor)
{
	Monitor *temp_mon;

	if (monitor == g_slacker.monitor_list) {
		g_slacker.monitor_list = g_slacker.monitor_list->next;
	} else {
		for (temp_mon = g_slacker.monitor_list;
		     temp_mon && temp_mon->next != monitor;
		     temp_mon = temp_mon->next) {
			;
		}
		temp_mon->next = monitor->next;
	}
	XUnmapWindow(g_slacker.display, monitor->barwin);
	XDestroyWindow(g_slacker.display, monitor->barwin);
	free(monitor);
}

/// @brief Client message event handler
///
/// @details Handles setting a window to full screen and setting a window to urgent.
void event_clientmessage(XEvent *event)
{
	XClientMessageEvent *cme = &event->xclient;
	Client *temp_client = Slacker__win_to_client(cme->window);

	if (!temp_client) {
		return;
	}

	if (cme->message_type ==
	    g_slacker.netatom[SlackerEWMHAtom_NetWMState]) {
		if (cme->data.l[1] ==
			    g_slacker.netatom[SlackerEWMHAtom_NetWMFullscreen] ||
		    cme->data.l[2] ==
			    g_slacker.netatom[SlackerEWMHAtom_NetWMFullscreen]) {
			Slacker__setfullscreen(
				temp_client,
				(cme->data.l[0] == 1 /* _NET_WM_STATE_ADD    */
				 ||
				 (cme->data.l[0] == 2 /* _NET_WM_STATE_TOGGLE */ &&
				  !temp_client->isfullscreen)));
		}
	} else if (cme->message_type ==
		   g_slacker.netatom[SlackerEWMHAtom_NetActiveWindow]) {
		if (temp_client !=
			    g_slacker.selected_monitor->selected_client &&
		    !temp_client->isurgent) {
			Slacker__seturgent(temp_client, 1);
		}
	}
}

/// @brief Used by the ConfigureRequest event handler to configure a client.
///
/// @details Builds an XConfigureEvent struct and sents it to the client via its window id.
void Client__configure(Client *client)
{
	XConfigureEvent ce;

	ce.type = ConfigureNotify;
	ce.display = g_slacker.display;
	ce.event = client->win;
	ce.window = client->win;
	ce.x = client->x;
	ce.y = client->y;
	ce.width = client->w;
	ce.height = client->h;
	ce.border_width = client->bw;
	ce.above = None;
	ce.override_redirect = False;
	XSendEvent(g_slacker.display, client->win, False, StructureNotifyMask,
		   (XEvent *)&ce);
}

void event_configurenotify(XEvent *event)
{
	Monitor *temp_monitor = NULL;
	Client *temp_client = NULL;
	XConfigureEvent *ev = &event->xconfigure;

	// TODO: updategeom handling sucks, needs to be simplified
	if (ev->window == g_slacker.root) {
		int32_t dirty = (g_slacker.screen_width != ev->width ||
				 g_slacker.screen_height != ev->height);
		g_slacker.screen_width = ev->width;
		g_slacker.screen_height = ev->height;
		if (Slacker__updategeom() || dirty) {
			drw_resize(g_slacker.draw, g_slacker.screen_width,
				   g_slacker.bar_height);
			Slacker__updatebars();
			for (temp_monitor = g_slacker.monitor_list;
			     temp_monitor; temp_monitor = temp_monitor->next) {
				for (temp_client = temp_monitor->client_list;
				     temp_client;
				     temp_client = temp_client->next) {
					if (temp_client->isfullscreen)
						Slacker__resize_client(
							temp_client,
							temp_monitor->mx,
							temp_monitor->my,
							temp_monitor->mw,
							temp_monitor->mh);
				}
				XMoveResizeWindow(
					g_slacker.display, temp_monitor->barwin,
					temp_monitor->wx, temp_monitor->by,
					temp_monitor->ww, g_slacker.bar_height);
			}
			Slacker__focus(NULL);
			Slacker__arrange_monitors(NULL);
		}
	}
}

void event_configurerequest(XEvent *event)
{
	Client *temp_client = NULL;
	Monitor *temp_monitor = NULL;
	XConfigureRequestEvent *ev = &event->xconfigurerequest;
	XWindowChanges wc;

	if ((temp_client = Slacker__win_to_client(ev->window))) {
		if (ev->value_mask & CWBorderWidth) {
			temp_client->bw = ev->border_width;
		} else if (temp_client->isfloating ||
			   !g_slacker.selected_monitor
				    ->layouts[g_slacker.selected_monitor
						      ->selected_layout]
				    ->layout_arrange_callback) {
			temp_monitor = temp_client->mon;

			// Configure window x coordinate
			if (ev->value_mask & CWX) {
				temp_client->oldx = temp_client->x;
				temp_client->x = temp_monitor->mx + ev->x;
			}

			// Configure window y coordinate
			if (ev->value_mask & CWY) {
				temp_client->oldy = temp_client->y;
				temp_client->y = temp_monitor->my + ev->y;
			}

			// Configure window width
			if (ev->value_mask & CWWidth) {
				temp_client->oldw = temp_client->w;
				temp_client->w = ev->width;
			}

			// Configure window height
			if (ev->value_mask & CWHeight) {
				temp_client->oldh = temp_client->h;
				temp_client->h = ev->height;
			}

			// Center in x direction
			if ((temp_client->x + temp_client->w) >
				    temp_monitor->mx + temp_monitor->mw &&
			    temp_client->isfloating) {
				temp_client->x = temp_monitor->mx +
						 (temp_monitor->mw / 2 -
						  WIDTH(temp_client) / 2);
			}

			// Center in y direction
			if ((temp_client->y + temp_client->h) >
				    temp_monitor->my + temp_monitor->mh &&
			    temp_client->isfloating) {
				temp_client->y = temp_monitor->my +
						 (temp_monitor->mh / 2 -
						  HEIGHT(temp_client) / 2);
			}

			if ((ev->value_mask & (CWX | CWY)) &&
			    !(ev->value_mask & (CWWidth | CWHeight))) {
				Client__configure(temp_client);
			}

			if (ISVISIBLE(temp_client)) {
				XMoveResizeWindow(
					g_slacker.display, temp_client->win,
					temp_client->x, temp_client->y,
					temp_client->w, temp_client->h);
			}
		} else {
			Client__configure(temp_client);
		}
	} else {
		wc.x = ev->x;
		wc.y = ev->y;
		wc.width = ev->width;
		wc.height = ev->height;
		wc.border_width = ev->border_width;
		wc.sibling = ev->above;
		wc.stack_mode = ev->detail;
		XConfigureWindow(g_slacker.display, ev->window, ev->value_mask,
				 &wc);
	}
	XSync(g_slacker.display, False);
}

/// @brief Constructs a single monitor
Monitor *Monitor__create(void)
{
	Monitor *m;

	m = ecalloc(1, sizeof(Monitor));
	m->tagset[0] = m->tagset[1] = 1;
	m->mfact = GLOBAL_MASTER_FACTOR;
	m->nmaster = GLOBAL_MASTER_COUNT;
	m->showbar = GLOBAL_SHOW_BAR;
	m->topbar = GLOBAL_TOP_BAR;
	m->layouts[0] = &GLOBAL_LAYOUTS[0];
	m->layouts[1] = &GLOBAL_LAYOUTS[1 % LENGTH(GLOBAL_LAYOUTS)];
	strncpy(m->layout_symbol, GLOBAL_LAYOUTS[0].symbol,
		sizeof m->layout_symbol);
	return m;
}

void event_destroynotify(XEvent *event)
{
	Client *temp_client;
	XDestroyWindowEvent *ev = &event->xdestroywindow;

	if ((temp_client = Slacker__win_to_client(ev->window))) {
		Slacker__unmanage(temp_client, 1);
	}
}

/// @brief Detaches a client from a monitor's client list.
void Client__detach(Client *client)
{
	Client **tc;

	for (tc = &client->mon->client_list; *tc && *tc != client;
	     tc = &(*tc)->next) {
		;
	}
	*tc = client->next;
}

/// @brief Detaches a client from a monitor's stack list.
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

/// @brief	Not sure what the purpose of this is, Debugging required.
///
/// @param `dir` TODO:
Monitor *Slacker__dir_to_monitor(int dir)
{
	Monitor *temp_monitor = NULL;

	if (dir > 0) {
		if (!(temp_monitor = g_slacker.selected_monitor->next)) {
			temp_monitor = g_slacker.monitor_list;
		}

	} else if (g_slacker.selected_monitor == g_slacker.monitor_list) {
		for (temp_monitor = g_slacker.monitor_list; temp_monitor->next;
		     temp_monitor = temp_monitor->next) {
			;
		}

	} else {
		for (temp_monitor = g_slacker.monitor_list;
		     temp_monitor->next != g_slacker.selected_monitor;
		     temp_monitor = temp_monitor->next) {
			;
		}
	}

	return temp_monitor;
}

/// @brief Draws the bar for a monitor.
///
/// @details TODO: Document this function in detail, and refactor.
void Slacker__drawbar(Monitor *monitor)
{
	int32_t x = 0;
	int32_t w = 0;
	int32_t text_width = 0;
	int32_t boxs = g_slacker.draw->fonts->h / 9;
	int32_t boxw = g_slacker.draw->fonts->h / 6 + 2;
	uint32_t i = 0;
	uint32_t occ = 0;
	uint32_t urgent = 0;
	Client *temp_client;

	if (!monitor->showbar) {
		return;
	}

	// Draw status first so it can be overdrawn by tags later.
	// Status is only drawn on the selected monitor
	if (monitor == g_slacker.selected_monitor) {
		// Calculate the width of the status text and add 2x padding
		text_width = TEXTW(g_slacker.status_text) -
			     (g_slacker.left_right_padding_sum + 2);

		drw_setscheme(g_slacker.draw,
			      g_slacker.scheme[SlackerColorscheme_Norm]);

		drw_text(g_slacker.draw, (monitor->ww - text_width), 0,
			 text_width, g_slacker.bar_height, 0,
			 g_slacker.status_text, 0);
	}

	for (temp_client = monitor->client_list; temp_client;
	     temp_client = temp_client->next) {
		occ |= temp_client->tags;
		if (temp_client->isurgent) {
			urgent |= temp_client->tags;
		}
	}

	// Draw the tags
	for (i = 0; i < LENGTH(GLOBAL_TAGS); ++i) {
		w = TEXTW(GLOBAL_TAGS[i]);
		drw_setscheme(
			g_slacker.draw,
			g_slacker.scheme[monitor->tagset[monitor->seltags] &
							 1 << i ?
						 SlackerColorscheme_Sel :
						 SlackerColorscheme_Norm]);

		drw_text(g_slacker.draw, x, 0, w, g_slacker.bar_height,
			 (g_slacker.left_right_padding_sum / 2), GLOBAL_TAGS[i],
			 urgent & 1 << i);

		if (occ & 1 << i) {
			drw_rect(
				g_slacker.draw, (x + boxs), boxs, boxw, boxw,
				monitor == g_slacker.selected_monitor &&
					g_slacker.selected_monitor
						->selected_client &&
					g_slacker.selected_monitor
							->selected_client->tags &
						1 << i,
				urgent & 1 << i);
		}

		x += w;
	}

	// Draw the layout symbol
	w = TEXTW(monitor->layout_symbol);
	drw_setscheme(g_slacker.draw,
		      g_slacker.scheme[SlackerColorscheme_Norm]);
	x = drw_text(g_slacker.draw, x, 0, w, g_slacker.bar_height,
		     (g_slacker.left_right_padding_sum / 2),
		     monitor->layout_symbol, 0);

	if ((w = monitor->ww - text_width - x) > g_slacker.bar_height) {
		if (monitor->selected_client) {
			enum SlackerColorscheme colorscheme =
				(monitor == g_slacker.selected_monitor ?
					 SlackerColorscheme_Sel :
					 SlackerColorscheme_Norm);

			drw_setscheme(g_slacker.draw,
				      g_slacker.scheme[colorscheme]);

			drw_text(g_slacker.draw, x, 0, w, g_slacker.bar_height,
				 (g_slacker.left_right_padding_sum / 2),
				 monitor->selected_client->name, 0);

			if (monitor->selected_client->isfloating) {
				drw_rect(g_slacker.draw, x + boxs, boxs, boxw,
					 boxw,
					 monitor->selected_client->isfixed, 0);
			}

		} else {
			drw_setscheme(
				g_slacker.draw,
				g_slacker.scheme[SlackerColorscheme_Norm]);

			drw_rect(g_slacker.draw, x, 0, w, g_slacker.bar_height,
				 1, 1);
		}
	}

	drw_map(g_slacker.draw, monitor->barwin, 0, 0, monitor->ww,
		g_slacker.bar_height);
}

/// @brief Draws the bar for all monitors.
void Slacker__drawbars(void)
{
	Monitor *monitor;

	for (monitor = g_slacker.monitor_list; monitor;
	     monitor = monitor->next) {
		Slacker__drawbar(monitor);
	}
}

void event_enternotify(XEvent *event)
{
	Client *temp_client = NULL;
	Monitor *temp_monitor = NULL;
	XCrossingEvent *ev = &event->xcrossing;

	if ((ev->mode != NotifyNormal || ev->detail == NotifyInferior) &&
	    ev->window != g_slacker.root) {
		return;
	}

	temp_client = Slacker__win_to_client(ev->window);
	temp_monitor = temp_client ? temp_client->mon :
				     Slacker__wintomon(ev->window);

	if (temp_monitor != g_slacker.selected_monitor) {
		Slacker__unfocus(g_slacker.selected_monitor->selected_client,
				 1);
		g_slacker.selected_monitor = temp_monitor;
	} else if (!temp_client ||
		   temp_client == g_slacker.selected_monitor->selected_client) {
		return;
	}
	Slacker__focus(temp_client);
}

void event_expose(XEvent *event)
{
	Monitor *monitor;
	XExposeEvent *ev = &event->xexpose;

	if (ev->count == 0 && (monitor = Slacker__wintomon(ev->window))) {
		Slacker__drawbar(monitor);
	}
}

/// @brief Focuses on a client
void Slacker__focus(Client *client)
{
	if (!client || !ISVISIBLE(client))
		for (client = g_slacker.selected_monitor->client_stack;
		     client && !ISVISIBLE(client);
		     client = client->stack_next) {
			;
		}

	if (g_slacker.selected_monitor->selected_client &&
	    g_slacker.selected_monitor->selected_client != client) {
		Slacker__unfocus(g_slacker.selected_monitor->selected_client,
				 0);
	}

	if (client) {
		if (client->mon != g_slacker.selected_monitor) {
			g_slacker.selected_monitor = client->mon;
		}

		if (client->isurgent) {
			Slacker__seturgent(client, 0);
		}

		Client__detach_from_stack(client);
		Client__attach_to_stack(client);
		Slacker__grab_buttons(client, true);

		XSetWindowBorder(
			g_slacker.display, client->win,
			g_slacker.scheme[SlackerColorscheme_Sel][ColBorder]
				.pixel);

		Slacker__setfocus(client);
	} else {
		XSetInputFocus(g_slacker.display, g_slacker.root,
			       RevertToPointerRoot, CurrentTime);

		XDeleteProperty(
			g_slacker.display, g_slacker.root,
			g_slacker.netatom[SlackerEWMHAtom_NetActiveWindow]);
	}

	g_slacker.selected_monitor->selected_client = client;
	Slacker__drawbars();
}

void event_focusin(XEvent *event)
{
	XFocusChangeEvent *ev = &event->xfocus;

	if (g_slacker.selected_monitor->selected_client &&
	    ev->window != g_slacker.selected_monitor->selected_client->win) {
		Slacker__setfocus(g_slacker.selected_monitor->selected_client);
	}
}

/// @brief When a user changed to a different monitor, this function is called.
///
/// @details  TODO:
void Slacker__focus_monitor(const Arg *arg)
{
	Monitor *temp_monitor;

	if (!g_slacker.monitor_list->next) {
		return;
	}

	if ((temp_monitor = Slacker__dir_to_monitor(arg->i)) ==
	    g_slacker.selected_monitor) {
		return;
	}

	Slacker__unfocus(g_slacker.selected_monitor->selected_client, 0);
	g_slacker.selected_monitor = temp_monitor;
	Slacker__focus(NULL);
}

/// @brief Focuses on the next client in the stack.
/// TODO: Refactor, this function should be a Client function
void Slacker__focus_stack(const Arg *arg)
{
	Client *temp_client = NULL;

	// If the selected client is null, or the is fullscreen and the global lock is set, return.
	if (!g_slacker.selected_monitor->selected_client ||
	    (g_slacker.selected_monitor->selected_client->isfullscreen &&
	     GLOBAL_LOCK_FULLSCREEN)) {
		return;
	}

	if (arg->i > 0) {
		for (temp_client =
			     g_slacker.selected_monitor->selected_client->next;
		     temp_client && !ISVISIBLE(temp_client);
		     temp_client = temp_client->next) {
			;
		}

		if (!temp_client) {
			for (temp_client =
				     g_slacker.selected_monitor->client_list;
			     temp_client && !ISVISIBLE(temp_client);
			     temp_client = temp_client->next) {
				;
			}
		}
	} else {
		Client *iter = NULL;
		for (iter = g_slacker.selected_monitor->client_list;
		     iter != g_slacker.selected_monitor->selected_client;
		     iter = iter->next) {
			if (ISVISIBLE(iter)) {
				temp_client = iter;
			}
		}

		if (!temp_client) {
			for (; iter; iter = iter->next) {
				if (ISVISIBLE(iter)) {
					temp_client = iter;
				}
			}
		}
	}
	if (temp_client) {
		Slacker__focus(temp_client);
		Slacker__restack(g_slacker.selected_monitor);
	}
}

/// @brief Get the atom property for a given client.
///
/// @details TODO: This should be a Client__ function
/// @param `client` The client to get the atom prop for
Atom Slacker__get_atom_prop(Client *client, Atom prop)
{
	int32_t di;
	uint64_t dl;
	unsigned char *p = NULL;
	Atom da, atom = None;

	if (XGetWindowProperty(g_slacker.display, client->win, prop, 0L,
			       sizeof(atom), False, XA_ATOM, &da, &di, &dl, &dl,
			       &p) == Success &&
	    p) {
		atom = *(Atom *)p;
		XFree(p);
	}
	return atom;
}

/// @brief Gets the pointer coordinates relative to the root window's origin.
///
/// @param `root_x_return` Pointer to the root x coordinate
/// @param `root_y_return` Pointer to the root y coordinate
///
/// @return BadWindow if the window is invalid
int32_t Slacker__getrootptr(int *root_x_return, int *root_y_return)
{
	// Dummy variables that we don't care about.
	int32_t win_return_dummy;
	uint32_t mask_return_dummy;
	Window root_return_dummy, child_return_dummy;

	return XQueryPointer(g_slacker.display, g_slacker.root,
			     &root_return_dummy, &child_return_dummy,
			     root_x_return, root_y_return, &win_return_dummy,
			     &win_return_dummy, &mask_return_dummy);
}

/// @brief Gets the state of a given window and frees it via Xlib.
///
/// @param `wid` The window to get the state of and free
int64_t Slacker__getstate(Window wid)
{
	int32_t format = 0;
	int64_t result = -1;
	unsigned char *p = NULL;
	uint64_t n = 0;
	uint64_t extra = 0;
	Atom real;

	if (XGetWindowProperty(
		    g_slacker.display, wid,
		    g_slacker.wmatom[SlackerDefaultAtom_WMState], 0L, 2L, False,
		    g_slacker.wmatom[SlackerDefaultAtom_WMState], &real,
		    &format, &n, &extra, (unsigned char **)&p) != Success) {
		return -1;
	}
	if (n != 0) {
		result = *p;
	}
	XFree(p);
	return result;
}

/// @brief Get the text property for a given window id.
/// @details Takes a window id instead of a Client, because it is used for the root window as well.
///
/// @param `w_id` The window id to get the text prop for
/// @param `atom` The atom to get the text prop for
/// @param `text` Stores the text property name to be used
/// @param `size` The size of the text property name
///
/// @returns 1 if the text property was found, 0 otherwise
int32_t Slacker__get_text_prop(Window w_id, Atom atom, char *text,
			       uint32_t size)
{
	char **list = NULL;
	int32_t n = 0;
	XTextProperty name;

	if (!text || size == 0) {
		return 0;
	}
	text[0] = '\0';

	if (!XGetTextProperty(g_slacker.display, w_id, &name, atom) ||
	    !name.nitems) {
		return 0;
	}

	if (name.encoding == XA_STRING) {
		strncpy(text, (char *)name.value, size - 1);
	} else if (XmbTextPropertyToTextList(g_slacker.display, &name, &list,
					     &n) >= Success &&
		   n > 0 && *list) {
		strncpy(text, *list, size - 1);
		XFreeStringList(list);
	}

	text[size - 1] = '\0';
	XFree(name.value);
	return 1;
}

/// @brief Checks to see if any of the supported button masks were pressed
/// on a client window.
///
/// @param `client` The client to check for button presses on
/// @param `focused` Whether or not the client should be focused
void Slacker__grab_buttons(Client *client, bool focused)
{
	Slacker__update_numlock_mask();
	{
		// Clear all buttons
		XUngrabButton(g_slacker.display, AnyButton, AnyModifier,
			      client->win);

		if (!focused) {
			XGrabButton(g_slacker.display, AnyButton, AnyModifier,
				    client->win, False, BUTTONMASK,
				    GrabModeSync, GrabModeSync, None, None);
		}

		uint32_t modifiers[] = { 0, LockMask, g_slacker.numlockmask,
					 (g_slacker.numlockmask | LockMask) };

		for (uint32_t i = 0; i < LENGTH(GLOBAL_CLICKABLE_BUTTONS);
		     ++i) {
			// if the button is a client window button, grab it
			if (GLOBAL_CLICKABLE_BUTTONS[i].click ==
			    SlackerClick_ClientWin) {
				for (uint32_t j = 0; j < LENGTH(modifiers); ++j)
					XGrabButton(g_slacker.display,
						    GLOBAL_CLICKABLE_BUTTONS[i]
							    .button,
						    GLOBAL_CLICKABLE_BUTTONS[i]
								    .mask |
							    modifiers[j],
						    client->win, False,
						    BUTTONMASK, GrabModeAsync,
						    GrabModeSync, None, None);
			}
		}
	}
}

/// @brief Sets up all the custom key bindings defined in GLOBAL_KEYBINDINGS
///
/// @details This function is called once at startup, and again on mappping notify events.
void Slacker__grab_keys(void)
{
	Slacker__update_numlock_mask();
	{
		uint32_t modifiers[] = { 0, LockMask, g_slacker.numlockmask,
					 g_slacker.numlockmask | LockMask };

		int32_t start = 0;
		int32_t end = 0;
		int32_t skip = 0;

		XUngrabKey(g_slacker.display, AnyKey, AnyModifier,
			   g_slacker.root);
		XDisplayKeycodes(g_slacker.display, &start, &end);
		KeySym *syms = XGetKeyboardMapping(g_slacker.display, start,
						   (end - (start + 1)), &skip);
		if (!syms) {
			return;
		}

		for (uint32_t k = start; k <= end; ++k) {
			for (uint32_t i = 0; i < LENGTH(GLOBAL_KEYBINDINGS);
			     ++i)
				// Skip modifier codes, we do that ourselves
				if (GLOBAL_KEYBINDINGS[i].keysym ==
				    syms[(k - start) * skip])
					for (uint32_t j = 0;
					     j < LENGTH(modifiers); ++j) {
						XGrabKey(g_slacker.display, k,
							 GLOBAL_KEYBINDINGS[i]
									 .mod |
								 modifiers[j],
							 g_slacker.root, True,
							 GrabModeAsync,
							 GrabModeAsync);
					}
		}
		XFree(syms);
	}
}

/// @brief Increments master window in the stacker area by (+ or 1 n)
void Slacker__increment_n_master(const Arg *arg)
{
	Monitor *active = g_slacker.selected_monitor;
	active->nmaster = MAX(active->nmaster + arg->i, 0);
	Slacker__arrange_monitors(active);
}

void event_keypress(XEvent *event)
{
	XKeyEvent *ev = &event->xkey;
	KeySym keysym =
		XKeycodeToKeysym(g_slacker.display, (KeyCode)ev->keycode, 0);

	for (uint32_t i = 0; i < LENGTH(GLOBAL_KEYBINDINGS); i++) {
		if (keysym == GLOBAL_KEYBINDINGS[i].keysym &&
		    CLEANMASK(GLOBAL_KEYBINDINGS[i].mod) ==
			    CLEANMASK(ev->state) &&
		    GLOBAL_KEYBINDINGS[i].keymap_callback) {
			GLOBAL_KEYBINDINGS[i].keymap_callback(
				&(GLOBAL_KEYBINDINGS[i].arg));
		}
	}
}

/// @brief Kills the currently selected client.
void Slacker__kill_client(const Arg *arg)
{
	if (!g_slacker.selected_monitor->selected_client) {
		return;
	}

	if (!Slacker__send_event(
		    g_slacker.selected_monitor->selected_client,
		    g_slacker.wmatom[SlackerDefaultAtom_WMDelete])) {
		XGrabServer(g_slacker.display);
		XSetErrorHandler(xerrordummy);
		XSetCloseDownMode(g_slacker.display, DestroyAll);
		XKillClient(g_slacker.display,
			    g_slacker.selected_monitor->selected_client->win);
		XSync(g_slacker.display, False);
		XSetErrorHandler(xerror);
		XUngrabServer(g_slacker.display);
	}
}

/// @brief Creates a new client window and manages it.
///
/// @param `w` The window to manage
/// @param `wa` The window attributes of the window to manage
void Slacker__manage(Window w_id, XWindowAttributes *wa)
{
	fprintf(stdout, "%s\n", __PRETTY_FUNCTION__);
	Client *new_client = NULL;
	Client *temp_client = NULL;
	Window trans = None;
	XWindowChanges wc;

	new_client = ecalloc(1, sizeof(Client));
	new_client->win = w_id;
	/* geometry */
	new_client->x = new_client->oldx = wa->x;
	new_client->y = new_client->oldy = wa->y;
	new_client->w = new_client->oldw = wa->width;
	new_client->h = new_client->oldh = wa->height;
	new_client->oldbw = wa->border_width;

	Slacker__update_client_title(new_client);
	if (XGetTransientForHint(g_slacker.display, w_id, &trans) &&
	    (temp_client = Slacker__win_to_client(trans))) {
		new_client->mon = temp_client->mon;
		new_client->tags = temp_client->tags;
	} else {
		new_client->mon = g_slacker.selected_monitor;
		Slacker__applyrules(new_client);
	}

	if (new_client->x + WIDTH(new_client) >
	    new_client->mon->wx + new_client->mon->ww) {
		new_client->x = new_client->mon->wx + new_client->mon->ww -
				WIDTH(new_client);
	}

	if (new_client->y + HEIGHT(new_client) >
	    (new_client->mon->wy + new_client->mon->wh)) {
		new_client->y = new_client->mon->wy + new_client->mon->wh -
				HEIGHT(new_client);
	}

	new_client->x = MAX(new_client->x, new_client->mon->wx);
	new_client->y = MAX(new_client->y, new_client->mon->wy);
	new_client->bw = GLOBAL_BORDER_PIXEL;

	wc.border_width = new_client->bw;
	XConfigureWindow(g_slacker.display, w_id, CWBorderWidth, &wc);
	XSetWindowBorder(
		g_slacker.display, w_id,
		g_slacker.scheme[SlackerColorscheme_Norm][ColBorder].pixel);

	// Propagates border_width, if size doesn't change
	Client__configure(new_client);

	Slacker__update_window_type(new_client);
	updatesizehints(new_client);
	Slacker__update_wmhints(new_client);
	XSelectInput(g_slacker.display, w_id,
		     EnterWindowMask | FocusChangeMask | PropertyChangeMask |
			     StructureNotifyMask);

	Slacker__grab_buttons(new_client, false);

	if (!new_client->isfloating) {
		new_client->isfloating = new_client->oldstate =
			trans != None || new_client->isfixed;
	}

	if (new_client->isfloating) {
		XRaiseWindow(g_slacker.display, new_client->win);
	}

	Client__attach(new_client);
	Client__attach_to_stack(new_client);
	XChangeProperty(g_slacker.display, g_slacker.root,
			g_slacker.netatom[SlackerEWMHAtom_NetClientList],
			XA_WINDOW, 32, PropModeAppend,
			(unsigned char *)&(new_client->win), 1);

	XMoveResizeWindow(g_slacker.display, new_client->win,
			  new_client->x + 2 * g_slacker.screen_width,
			  new_client->y, new_client->w,
			  new_client->h); /* some windows require this */

	Slacker__set_client_state(new_client, NormalState);
	if (new_client->mon == g_slacker.selected_monitor) {
		Slacker__unfocus(g_slacker.selected_monitor->selected_client,
				 0);
	}

	new_client->mon->selected_client = new_client;
	Slacker__arrange_monitors(new_client->mon);
	XMapWindow(g_slacker.display, new_client->win);
	Slacker__focus(NULL);
}

void event_mappingnotify(XEvent *event)
{
	XMappingEvent *ev = &event->xmapping;

	XRefreshKeyboardMapping(ev);
	if (ev->request == MappingKeyboard) {
		Slacker__grab_keys();
	}
}

void event_maprequest(XEvent *event)
{
	static XWindowAttributes wa;
	XMapRequestEvent *ev = &event->xmaprequest;

	if (!XGetWindowAttributes(g_slacker.display, ev->window, &wa) ||
	    wa.override_redirect) {
		return;
	}

	if (!Slacker__win_to_client(ev->window)) {
		Slacker__manage(ev->window, &wa);
	}
}

void Monitor__layout_monocle(Monitor *monitor)
{
	uint32_t number_of_clients = 0;
	Client *temp_client = NULL;

	// collect number of visible clients on this monitor.
	for (temp_client = monitor->client_list; temp_client;
	     temp_client = temp_client->next) {
		if (ISVISIBLE(temp_client)) {
			number_of_clients++;
		}
	}

	// If we have clients, override the layout symbol in the bar.
	if (number_of_clients > 0) {
		snprintf(monitor->layout_symbol, sizeof(monitor->layout_symbol),
			 "[%d]", number_of_clients);
	}

	for (temp_client = Client__next_tiled(monitor->client_list);
	     temp_client; temp_client = Client__next_tiled(temp_client->next)) {
		Client__resize(temp_client, monitor->wx, monitor->wy,
			       monitor->ww - 2 * temp_client->bw,
			       monitor->wh - 2 * temp_client->bw, 0);
	}
}

void event_motionnotify(XEvent *event)
{
	static Monitor *mon = NULL;
	Monitor *m;
	XMotionEvent *ev = &event->xmotion;

	if (ev->window != g_slacker.root) {
		return;
	}
	if ((m = Slacker__rect_to_monitor(ev->x_root, ev->y_root, 1, 1)) !=
		    mon &&
	    mon) {
		Slacker__unfocus(g_slacker.selected_monitor->selected_client,
				 1);
		g_slacker.selected_monitor = m;
		Slacker__focus(NULL);
	}
	mon = m;
}

/// @brief Confusing code. I think this function is used to move a window via the mouse.
///
/// @details TODO: Refactor this function, it is confusing and hard to read.
void Slacker__move_with_mouse(const Arg *arg)
{
	int32_t x = 0;
	int32_t y = 0;
	int32_t ocx = 0;
	int32_t ocy = 0;
	int32_t nx = 0;
	int32_t ny = 0;
	Client *temp_client = 0;
	Monitor *m = 0;
	Time lasttime = 0;
	XEvent ev;

	// If there is no selected client, return.
	if (!(temp_client = g_slacker.selected_monitor->selected_client)) {
		return;
	}

	// No support moving fullscreen windows by mouse
	if (temp_client->isfullscreen) {
		return;
	}

	Slacker__restack(g_slacker.selected_monitor);
	ocx = temp_client->x;
	ocy = temp_client->y;

	if (XGrabPointer(g_slacker.display, g_slacker.root, False, MOUSEMASK,
			 GrabModeAsync, GrabModeAsync, None,
			 g_slacker.cursor[SlackerCursorState_Move]->cursor,
			 CurrentTime) != GrabSuccess) {
		return;
	}

	// If window is invalid, return.
	if (!Slacker__getrootptr(&x, &y)) {
		return;
	}

	do {
		XMaskEvent(g_slacker.display,
			   MOUSEMASK | ExposureMask | SubstructureRedirectMask,
			   &ev);
		switch (ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			event_loop(&ev);
			break;
		case MotionNotify:
			if ((ev.xmotion.time - lasttime) <= (1000 / 60)) {
				continue;
			}
			lasttime = ev.xmotion.time;

			nx = ocx + (ev.xmotion.x - x);
			ny = ocy + (ev.xmotion.y - y);

			if (abs(g_slacker.selected_monitor->wx - nx) <
			    GLOBAL_SNAP_PIXEL) {
				nx = g_slacker.selected_monitor->wx;
			} else if (abs((g_slacker.selected_monitor->wx +
					g_slacker.selected_monitor->ww) -
				       (nx + WIDTH(temp_client))) <
				   GLOBAL_SNAP_PIXEL) {
				nx = g_slacker.selected_monitor->wx +
				     g_slacker.selected_monitor->ww -
				     WIDTH(temp_client);
			}

			if (abs(g_slacker.selected_monitor->wy - ny) <
			    GLOBAL_SNAP_PIXEL) {
				ny = g_slacker.selected_monitor->wy;
			} else if (abs((g_slacker.selected_monitor->wy +
					g_slacker.selected_monitor->wh) -
				       (ny + HEIGHT(temp_client))) <
				   GLOBAL_SNAP_PIXEL) {
				ny = g_slacker.selected_monitor->wy +
				     g_slacker.selected_monitor->wh -
				     HEIGHT(temp_client);
			}

			if (!temp_client->isfloating &&
			    g_slacker.selected_monitor
				    ->layouts[g_slacker.selected_monitor
						      ->selected_layout]
				    ->layout_arrange_callback &&
			    (abs(nx - temp_client->x) > GLOBAL_SNAP_PIXEL ||
			     abs(ny - temp_client->y) > GLOBAL_SNAP_PIXEL)) {
				Slacker__togglefloating(NULL);
			}

			if (!g_slacker.selected_monitor
				     ->layouts[g_slacker.selected_monitor
						       ->selected_layout]
				     ->layout_arrange_callback ||
			    temp_client->isfloating) {
				Client__resize(temp_client, nx, ny,
					       temp_client->w, temp_client->h,
					       1);
			}
			break;
		}
	} while (ev.type != ButtonRelease);

	XUngrabPointer(g_slacker.display, CurrentTime);

	if ((m = Slacker__rect_to_monitor(temp_client->x, temp_client->y,
					  temp_client->w, temp_client->h)) !=
	    g_slacker.selected_monitor) {
		Client__send_to_monitor(temp_client, m);
		g_slacker.selected_monitor = m;
		Slacker__focus(NULL);
	}
}

/// @brief Get the next tiled and visible client in the client linked list.
Client *Client__next_tiled(Client *client)
{
	for (; client && (client->isfloating || !ISVISIBLE(client));
	     client = client->next) {
		;
	}
	return client;
}

/// @brief Removes a client from the client list.
void Client__pop(Client *client)
{
	Client__detach(client);
	Client__attach(client);
	Slacker__focus(client);
	Slacker__arrange_monitors(client->mon);
}

void event_propertynotify(XEvent *event)
{
	Client *client = NULL;
	Window trans;
	XPropertyEvent *ev = &event->xproperty;

	if ((ev->window == g_slacker.root) && (ev->atom == XA_WM_NAME)) {
		Slacker__update_status();
	} else if (ev->state == PropertyDelete) {
		// TODO: Move this else if to an if at the top of the file.
		return;
	} else if ((client = Slacker__win_to_client(ev->window))) {
		switch (ev->atom) {
		case XA_WM_TRANSIENT_FOR:
			if (!client->isfloating &&
			    (XGetTransientForHint(g_slacker.display,
						  client->win, &trans)) &&
			    (client->isfloating =
				     (Slacker__win_to_client(trans)) != NULL)) {
				Slacker__arrange_monitors(client->mon);
			}
			break;
		case XA_WM_NORMAL_HINTS:
			client->hintsvalid = 0;
			break;
		case XA_WM_HINTS:
			Slacker__update_wmhints(client);
			Slacker__drawbars();
			break;
		default:
			break;
		}

		if (ev->atom == XA_WM_NAME ||
		    ev->atom == g_slacker.netatom[SlackerEWMHAtom_NetWMName]) {
			Slacker__update_client_title(client);
			if (client == client->mon->selected_client) {
				Slacker__drawbar(client->mon);
			}
		}

		if (ev->atom ==
		    g_slacker.netatom[SlackerEWMHAtom_NetWMWindowType]) {
			Slacker__update_window_type(client);
		}
	}
}

/// @brief Shutdown the window manager
void Slacker__quit(const Arg *arg)
{
	g_slacker.is_running = false;
}

/// @brief Transforms coordinates, width and height to the monitor they are on.
///
///	TODO: This can be refactored to be a Monitor__ function
/// @param `x` The x coordinate
/// @param `y` The y coordinate
/// @param `w` The width
/// @param `h` The height
///
/// @return The monitor the coordinates, width and height are on.
Monitor *Slacker__rect_to_monitor(int x, int y, int w, int h)
{
	Monitor *m, *r = g_slacker.selected_monitor;
	int32_t a, area = 0;

	for (m = g_slacker.monitor_list; m; m = m->next) {
		if ((a = INTERSECT(x, y, w, h, m)) > area) {
			area = a;
			r = m;
		}
	}
	return r;
}

/// @brief Applies size hints to a given client.
void Client__resize(Client *client, int x, int y, int w, int h, int interact)
{
	if (Slacker__applysizehints(client, &x, &y, &w, &h, interact)) {
		Slacker__resize_client(client, x, y, w, h);
	}
}

/// @brief Resizes a client with the given dimensions.
///
/// @param `client` The client to resize
/// @param `x` The x coordinate
/// @param `y` The y coordinate
/// @param `w` The width
/// @param `h` The height
void Slacker__resize_client(Client *client, int x, int y, int w, int h)
{
	XWindowChanges wc;

	client->oldx = client->x;
	client->x = wc.x = x;
	client->oldy = client->y;
	client->y = wc.y = y;
	client->oldw = client->w;
	client->w = wc.width = w;
	client->oldh = client->h;
	client->h = wc.height = h;
	wc.border_width = client->bw;
	XConfigureWindow(g_slacker.display, client->win,
			 CWX | CWY | CWWidth | CWHeight | CWBorderWidth, &wc);
	Client__configure(client);
	XSync(g_slacker.display, False);
}

/// @brief Adds support for resizing a client with the mouse.
///
/// TODO: Refactor, this function
/// @details Does not support resizing fullscreen windows, because this is a tiling window manager.
///
/// @param `arg` The argument to pass to the function
void Slacker__resize_client_with_mouse(const Arg *arg)
{
	int32_t ocx = 0;
	int32_t ocy = 0;
	int32_t nw = 0;
	int32_t nh = 0;
	Client *temp_client = NULL;
	Monitor *temp_monitor = NULL;
	XEvent ev;
	Time lasttime = 0;

	// If there is no selected client on the selected monitor, return.
	if (!(temp_client = g_slacker.selected_monitor->selected_client)) {
		return;
	}

	// No support for resizing fullscreen windows by mouse
	if (temp_client->isfullscreen) {
		return;
	}

	Slacker__restack(g_slacker.selected_monitor);
	ocx = temp_client->x;
	ocy = temp_client->y;

	if (XGrabPointer(g_slacker.display, g_slacker.root, False, MOUSEMASK,
			 GrabModeAsync, GrabModeAsync, None,
			 g_slacker.cursor[SlackerCursorState_Resize]->cursor,
			 CurrentTime) != GrabSuccess) {
		return;
	}

	XWarpPointer(g_slacker.display, None, temp_client->win, 0, 0, 0, 0,
		     temp_client->w + temp_client->bw - 1,
		     temp_client->h + temp_client->bw - 1);
	do {
		XMaskEvent(g_slacker.display,
			   MOUSEMASK | ExposureMask | SubstructureRedirectMask,
			   &ev);

		switch (ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			event_loop(&ev);
			break;
		case MotionNotify:
			if ((ev.xmotion.time - lasttime) <= (1000 / 60)) {
				continue;
			}
			lasttime = ev.xmotion.time;

			nw = MAX((ev.xmotion.x - ocx - 2 * temp_client->bw + 1),
				 1);
			nh = MAX((ev.xmotion.y - ocy - 2 * temp_client->bw + 1),
				 1);

			if (temp_client->mon->wx + nw >=
				    g_slacker.selected_monitor->wx &&
			    temp_client->mon->wx + nw <=
				    g_slacker.selected_monitor->wx +
					    g_slacker.selected_monitor->ww &&
			    temp_client->mon->wy + nh >=
				    g_slacker.selected_monitor->wy &&
			    temp_client->mon->wy + nh <=
				    g_slacker.selected_monitor->wy +
					    g_slacker.selected_monitor->wh) {
				if (!temp_client->isfloating &&
				    g_slacker.selected_monitor
					    ->layouts[g_slacker
							      .selected_monitor
							      ->selected_layout]
					    ->layout_arrange_callback &&
				    (abs(nw - temp_client->w) >
					     GLOBAL_SNAP_PIXEL ||
				     abs(nh - temp_client->h) >
					     GLOBAL_SNAP_PIXEL))
					Slacker__togglefloating(NULL);
			}

			if (!g_slacker.selected_monitor
				     ->layouts[g_slacker.selected_monitor
						       ->selected_layout]
				     ->layout_arrange_callback ||
			    temp_client->isfloating) {
				Client__resize(temp_client, temp_client->x,
					       temp_client->y, nw, nh, 1);
			}
			break;
		}
	} while (ev.type != ButtonRelease);

	XWarpPointer(g_slacker.display, None, temp_client->win, 0, 0, 0, 0,
		     temp_client->w + (temp_client->bw - 1),
		     temp_client->h + (temp_client->bw - 1));

	XUngrabPointer(g_slacker.display, CurrentTime);

	while (XCheckMaskEvent(g_slacker.display, EnterWindowMask, &ev)) {
		;
	}

	if ((temp_monitor = Slacker__rect_to_monitor(
		     temp_client->x, temp_client->y, temp_client->w,
		     temp_client->h)) != g_slacker.selected_monitor) {
		Client__send_to_monitor(temp_client, temp_monitor);
		g_slacker.selected_monitor = temp_monitor;
		Slacker__focus(NULL);
	}
}

/// @brief Restacks the client windows on a given monitor according to the layout.
///
/// @details Likely called on a layout change
///
/// @param `monitor` The monitor to restack
void Slacker__restack(Monitor *monitor)
{
	Client *temp_client = NULL;
	XEvent ev;
	XWindowChanges wc;

	Slacker__drawbar(monitor);
	if (!monitor->selected_client) {
		return;
	}

	// if the selected client is floating, and the arrange callback is not null, raise the window.
	if (monitor->selected_client->isfloating ||
	    !monitor->layouts[monitor->selected_layout]
		     ->layout_arrange_callback) {
		XRaiseWindow(g_slacker.display, monitor->selected_client->win);
	}

	// if the layout out callback function is not null, stack the windows
	if (monitor->layouts[monitor->selected_layout]->layout_arrange_callback) {
		wc.stack_mode = Below;
		wc.sibling = monitor->barwin;
		for (temp_client = monitor->client_stack; temp_client;
		     temp_client = temp_client->stack_next) {
			if (!temp_client->isfloating &&
			    ISVISIBLE(temp_client)) {
				XConfigureWindow(g_slacker.display,
						 temp_client->win,
						 CWSibling | CWStackMode, &wc);
				wc.sibling = temp_client->win;
			}
		}
	}

	XSync(g_slacker.display, false);
	while (XCheckMaskEvent(g_slacker.display, EnterWindowMask, &ev)) {
		;
	}
}

/// @brief Runs the main event loop.
///
/// @details This function is called after the window manager has been initialized
/// and is responsible for handling all X events and running the window manager.
void Slacker__run(void)
{
	XEvent ev;
	XSync(g_slacker.display, False);
	while (g_slacker.is_running && !XNextEvent(g_slacker.display, &ev)) {
		event_loop(&ev);
	}
}

/// @brief Remove all child and transient windows from the window manager.
///
/// @details Runs before slacker starts, this is to ensure that no windows
/// from previous window managers are still lurking about.
void Slacker__scan(void)
{
	uint32_t number_child_windows = 0;
	Window parent_return;
	Window child_return;
	Window *list_of_windows = NULL;
	XWindowAttributes wa;

	if (XQueryTree(g_slacker.display, g_slacker.root, &parent_return,
		       &child_return, &list_of_windows,
		       &number_child_windows)) {
		for (uint32_t i = 0; i < number_child_windows; ++i) {
			if (!XGetWindowAttributes(g_slacker.display,
						  list_of_windows[i], &wa) ||
			    wa.override_redirect ||
			    XGetTransientForHint(g_slacker.display,
						 list_of_windows[i],
						 &parent_return)) {
				continue;
			}

			if (wa.map_state == IsViewable ||
			    Slacker__getstate(list_of_windows[i]) ==
				    IconicState) {
				Slacker__manage(list_of_windows[i], &wa);
			}
		}
		// Clear transients
		for (uint32_t i = 0; i < number_child_windows; ++i) {
			if (!XGetWindowAttributes(g_slacker.display,
						  list_of_windows[i], &wa))
				continue;
			if (XGetTransientForHint(g_slacker.display,
						 list_of_windows[i],
						 &parent_return) &&
			    (wa.map_state == IsViewable ||
			     Slacker__getstate(list_of_windows[i]) ==
				     IconicState)) {
				Slacker__manage(list_of_windows[i], &wa);
			}
		}
		if (list_of_windows) {
			XFree(list_of_windows);
		}
	}
}

/// @brief Sends a client to a differnt monitor.
///
/// @param `client` The client to send to a different monitor
/// @param `target_monitor` The monitor to send the client to
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

/// @brief Updates the client state property of a client.
void Slacker__set_client_state(Client *client, int64_t state)
{
	int64_t data[] = { state, None };

	XChangeProperty(g_slacker.display, client->win,
			g_slacker.wmatom[SlackerDefaultAtom_WMState],
			g_slacker.wmatom[SlackerDefaultAtom_WMState], 32,
			PropModeReplace, (unsigned char *)data, 2);
}

bool Slacker__send_event(Client *client, Atom proto)
{
	// size of protocols list
	int32_t number_of_protocols = 0;
	// list of protocols
	Atom *protocols;
	// Set to true if 1 or more protocols are supported by this client
	bool exists = false;

	// Returns the list of atoms stored in the WM_PROTOCOLS property on the specified window.
	// These atoms describe window manager protocols in which the owner of this window is willing to participate.
	if (XGetWMProtocols(g_slacker.display, client->win, &protocols,
			    &number_of_protocols)) {
		while (!exists && number_of_protocols--) {
			exists = protocols[number_of_protocols] == proto;
		}
		XFree(protocols);
	}

	// We know this client supports atleast on protocol, so send the event.
	if (exists) {
		XEvent ev;
		ev.type = ClientMessage;
		ev.xclient.window = client->win;
		ev.xclient.message_type =
			g_slacker.wmatom[SlackerDefaultAtom_WMProtocols];
		ev.xclient.format = 32;
		ev.xclient.data.l[0] = proto;
		ev.xclient.data.l[1] = CurrentTime;
		// The XSendEvent() function identifies the destination window,
		// determines which clients should receive the specified events,
		// and ignores any active grabs.
		XSendEvent(g_slacker.display, client->win, False, NoEventMask,
			   &ev);
	}
	return exists;
}

/// @brief Sets the focus to a given client.
void Slacker__setfocus(Client *client)
{
	if (!client->neverfocus) {
		XSetInputFocus(g_slacker.display, client->win,
			       RevertToPointerRoot, CurrentTime);
		XChangeProperty(
			g_slacker.display, g_slacker.root,
			g_slacker.netatom[SlackerEWMHAtom_NetActiveWindow],
			XA_WINDOW, 32, PropModeReplace,
			(unsigned char *)&(client->win), 1);
	}
	Slacker__send_event(client,
			    g_slacker.wmatom[SlackerDefaultAtom_WMTakeFocus]);
}

/// @brief Sets the fullscreen state of a client.
///
/// TODO: Refactor, should be a Client__ function
/// @param `client` The client to set the fullscreen state of
/// @param `fullscreen` The fullscreen state to set
void Slacker__setfullscreen(Client *client, int fullscreen)
{
	if (fullscreen && !client->isfullscreen) {
		XChangeProperty(
			g_slacker.display, client->win,
			g_slacker.netatom[SlackerEWMHAtom_NetWMState], XA_ATOM,
			32, PropModeReplace,
			(unsigned char *)&g_slacker
				.netatom[SlackerEWMHAtom_NetWMFullscreen],
			1);
		client->isfullscreen = 1;
		client->oldstate = client->isfloating;
		client->oldbw = client->bw;
		client->bw = 0;
		client->isfloating = 1;
		Slacker__resize_client(client, client->mon->mx, client->mon->my,
				       client->mon->mw, client->mon->mh);
		XRaiseWindow(g_slacker.display, client->win);
	} else if (!fullscreen && client->isfullscreen) {
		XChangeProperty(g_slacker.display, client->win,
				g_slacker.netatom[SlackerEWMHAtom_NetWMState],
				XA_ATOM, 32, PropModeReplace,
				(unsigned char *)0, 0);

		client->isfullscreen = 0;
		client->isfloating = client->oldstate;
		client->bw = client->oldbw;
		client->x = client->oldx;
		client->y = client->oldy;
		client->w = client->oldw;
		client->h = client->oldh;
		Slacker__resize_client(client, client->x, client->y, client->w,
				       client->h);
		Slacker__arrange_monitors(client->mon);
	}
}

/// @brief Sets the layout of a monitor.
///
/// @param `arg` The argument to pass to the function, the void * is a Layout *
void Slacker__setlayout(const Arg *arg)
{
	Monitor *sm = g_slacker.selected_monitor;
	if (!sm) {
		return;
	}

	if (!arg || !arg->v || arg->v != sm->layouts[sm->selected_layout]) {
		sm->selected_layout ^= 1;
	}

	if (arg && arg->v) {
		sm->layouts[sm->selected_layout] = (Layout *)arg->v;
	}

	strncpy(sm->layout_symbol, sm->layouts[sm->selected_layout]->symbol,
		sizeof(sm->layout_symbol));

	if (sm->selected_client) {
		Slacker__arrange_monitors(sm);
	} else {
		Slacker__drawbar(sm);
	}
}

/// @brief arg > 1.0 will set mfact absolutely
///
/// TODO: Refactor: Should be a Monitor__ function
/// @param `arg` The argument to pass to the function
void Slacker__setmfact(const Arg *arg)
{
	Monitor *sm = g_slacker.selected_monitor;

	if (!sm || !arg ||
	    !sm->layouts[sm->selected_layout]->layout_arrange_callback) {
		return;
	}

	float factor = arg->f < 1.0 ? arg->f + sm->mfact : arg->f - 1.0;

	if (factor < 0.05 || factor > 0.95) {
		return;
	}
	sm->mfact = factor;
	Slacker__arrange_monitors(sm);
}

void Slacker__create(void)
{
	int32_t i = 0;
	XSetWindowAttributes wa;
	Atom utf8string;
	struct sigaction sa;

	/* do not transform children into zombies when they terminate */
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_NOCLDSTOP | SA_NOCLDWAIT | SA_RESTART;
	sa.sa_handler = SIG_IGN;
	sigaction(SIGCHLD, &sa, NULL);

	/* clean up any zombies (inherited from .xinitrc etc) immediately */
	while (waitpid(-1, NULL, WNOHANG) > 0) {
		;
	}

	// TODO: Move these to slacker.c
	g_slacker.is_running = true;
	g_slacker.numlockmask = 0;

	// Init screen
	g_slacker.screen = DefaultScreen(g_slacker.display);
	g_slacker.screen_width =
		DisplayWidth(g_slacker.display, g_slacker.screen);
	g_slacker.screen_height =
		DisplayHeight(g_slacker.display, g_slacker.screen);
	g_slacker.root = RootWindow(g_slacker.display, g_slacker.screen);
	g_slacker.draw = drw_create(g_slacker.display, g_slacker.screen,
				    g_slacker.root, g_slacker.screen_width,
				    g_slacker.screen_height);

	if (!drw_fontset_create(g_slacker.draw, GLOBAL_USER_FONT)) {
		die("no fonts could be loaded.");
	}

	g_slacker.left_right_padding_sum = g_slacker.draw->fonts->h;
	g_slacker.bar_height = g_slacker.draw->fonts->h + 2;

	// Creates monitors and sets the current monitor to the first one
	Slacker__updategeom();

	// Init atoms
	utf8string = XInternAtom(g_slacker.display, "UTF8_STRING", False);
	g_slacker.wmatom[SlackerDefaultAtom_WMProtocols] =
		XInternAtom(g_slacker.display, "WM_PROTOCOLS", False);
	g_slacker.wmatom[SlackerDefaultAtom_WMDelete] =
		XInternAtom(g_slacker.display, "WM_DELETE_WINDOW", False);
	g_slacker.wmatom[SlackerDefaultAtom_WMState] =
		XInternAtom(g_slacker.display, "WM_STATE", False);
	g_slacker.wmatom[SlackerDefaultAtom_WMTakeFocus] =
		XInternAtom(g_slacker.display, "WM_TAKE_FOCUS", False);
	g_slacker.netatom[SlackerEWMHAtom_NetActiveWindow] =
		XInternAtom(g_slacker.display, "_NET_ACTIVE_WINDOW", False);
	g_slacker.netatom[SlackerEWMHAtom_NetSupported] =
		XInternAtom(g_slacker.display, "_NET_SUPPORTED", False);
	g_slacker.netatom[SlackerEWMHAtom_NetWMName] =
		XInternAtom(g_slacker.display, "_NET_WM_NAME", False);
	g_slacker.netatom[SlackerEWMHAtom_NetWMState] =
		XInternAtom(g_slacker.display, "_NET_WM_STATE", False);
	g_slacker.netatom[SlackerEWMHAtom_NetWMCheck] = XInternAtom(
		g_slacker.display, "_NET_SUPPORTING_WM_CHECK", False);
	g_slacker.netatom[SlackerEWMHAtom_NetWMFullscreen] = XInternAtom(
		g_slacker.display, "_NET_WM_STATE_FULLSCREEN", False);
	g_slacker.netatom[SlackerEWMHAtom_NetWMWindowType] =
		XInternAtom(g_slacker.display, "_NET_WM_WINDOW_TYPE", False);
	g_slacker.netatom[SlackerEWMHAtom_NetWMWindowTypeDialog] = XInternAtom(
		g_slacker.display, "_NET_WM_WINDOW_TYPE_DIALOG", False);
	g_slacker.netatom[SlackerEWMHAtom_NetClientList] =
		XInternAtom(g_slacker.display, "_NET_CLIENT_LIST", False);

	// Init cursors
	g_slacker.cursor[SlackerCursorState_Normal] =
		drw_cur_create(g_slacker.draw, XC_left_ptr);
	g_slacker.cursor[SlackerCursorState_Resize] =
		drw_cur_create(g_slacker.draw, XC_sizing);
	g_slacker.cursor[SlackerCursorState_Move] =
		drw_cur_create(g_slacker.draw, XC_fleur);

	// Init appearance
	g_slacker.scheme =
		ecalloc(LENGTH(GLOBAL_COLORSCHEMES), sizeof(SlackerColor *));
	for (i = 0; i < LENGTH(GLOBAL_COLORSCHEMES); i++) {
		g_slacker.scheme[i] = drw_scm_create(g_slacker.draw,
						     GLOBAL_COLORSCHEMES[i], 3);
	}

	// Init bars
	Slacker__updatebars();
	Slacker__update_status();

	// Supporting window for NetWMCheck
	g_slacker.wmcheckwin = XCreateSimpleWindow(
		g_slacker.display, g_slacker.root, 0, 0, 1, 1, 0, 0, 0);

	XChangeProperty(g_slacker.display, g_slacker.wmcheckwin,
			g_slacker.netatom[SlackerEWMHAtom_NetWMCheck],
			XA_WINDOW, 32, PropModeReplace,
			(unsigned char *)&g_slacker.wmcheckwin, 1);

	XChangeProperty(g_slacker.display, g_slacker.wmcheckwin,
			g_slacker.netatom[SlackerEWMHAtom_NetWMName],
			utf8string, 8, PropModeReplace,
			(unsigned char *)"slacker", 3);

	XChangeProperty(g_slacker.display, g_slacker.root,
			g_slacker.netatom[SlackerEWMHAtom_NetWMCheck],
			XA_WINDOW, 32, PropModeReplace,
			(unsigned char *)&g_slacker.wmcheckwin, 1);

	// EWMH support per view
	XChangeProperty(g_slacker.display, g_slacker.root,
			g_slacker.netatom[SlackerEWMHAtom_NetSupported],
			XA_ATOM, 32, PropModeReplace,
			(unsigned char *)g_slacker.netatom,
			SlackerEWMHAtom_NetLast);

	XDeleteProperty(g_slacker.display, g_slacker.root,
			g_slacker.netatom[SlackerEWMHAtom_NetClientList]);

	// Register the events we plan to support with X
	wa.cursor = g_slacker.cursor[SlackerCursorState_Normal]->cursor;
	wa.event_mask = SubstructureRedirectMask | SubstructureNotifyMask |
			ButtonPressMask | PointerMotionMask | EnterWindowMask |
			LeaveWindowMask | StructureNotifyMask |
			PropertyChangeMask;

	XChangeWindowAttributes(g_slacker.display, g_slacker.root,
				CWEventMask | CWCursor, &wa);
	XSelectInput(g_slacker.display, g_slacker.root, wa.event_mask);
	Slacker__grab_keys();
	Slacker__focus(NULL);
}

/// @brief Sets the urgent state of a client.
///
/// TODO: Refactor: Could be a Client__ function by passing Display
/// @param `client` The client to set the urgent state of
/// @param `urgent` The urgent state to set
void Slacker__seturgent(Client *client, int urgent)
{
	XWMHints *wmh = NULL;

	client->isurgent = urgent;
	if (!(wmh = XGetWMHints(g_slacker.display, client->win))) {
		return;
	}
	wmh->flags = urgent ? (wmh->flags | XUrgencyHint) :
			      (wmh->flags & ~XUrgencyHint);
	XSetWMHints(g_slacker.display, client->win, wmh);
	XFree(wmh);
}

/// @brief Shows or hides a client.
///
/// TODO: Refactor: Should be a Client__ function
/// @param `client` The client to show or hide
void Slacker__showhide(Client *client)
{
	if (!client) {
		return;
	}
	if (ISVISIBLE(client)) {
		// Show clients top down
		XMoveWindow(g_slacker.display, client->win, client->x,
			    client->y);
		if ((!client->mon->layouts[client->mon->selected_layout]
			      ->layout_arrange_callback ||
		     client->isfloating) &&
		    !client->isfullscreen)
			Client__resize(client, client->x, client->y, client->w,
				       client->h, 0);
		Slacker__showhide(client->stack_next);
	} else {
		// Hide clients bottom up
		Slacker__showhide(client->stack_next);
		XMoveWindow(g_slacker.display, client->win, WIDTH(client) * -2,
			    client->y);
	}
}

/// @brief Helper function to spawn shell commands
///
/// @param `arg` The argument Union to pass to the function, the void * is a char *
void Slacker__spawn(const Arg *arg)
{
	struct sigaction sa;

	if (arg->v == GLOBAL_DMENU_COMMAND) {
		GLOBAL_DMENU_MONITOR[0] = '0' + g_slacker.selected_monitor->num;
	}

	if (fork() == 0) {
		if (g_slacker.display) {
			close(ConnectionNumber(g_slacker.display));
		}
		setsid();

		sigemptyset(&sa.sa_mask);
		sa.sa_flags = 0;
		sa.sa_handler = SIG_DFL;
		sigaction(SIGCHLD, &sa, NULL);

		execvp(((char **)arg->v)[0], (char **)arg->v);
		die("slacker: execvp '%s' failed:", ((char **)arg->v)[0]);
	}
}

/// @brief Uses the arg.ui field as a bitmask to toggle the tag of a client.
void Slacker__tag(const Arg *arg)
{
	Monitor *sm = g_slacker.selected_monitor;
	if (sm->selected_client && arg->ui & TAGMASK) {
		sm->selected_client->tags = arg->ui & TAGMASK;
		Slacker__focus(NULL);
		Slacker__arrange_monitors(sm);
	}
}

/// @brief Not sure what this does, debug
void Slacker__tagmon(const Arg *arg)
{
	// If there are no monitors return
	if (!g_slacker.selected_monitor->selected_client ||
	    !g_slacker.monitor_list->next) {
		return;
	}

	Client__send_to_monitor(g_slacker.selected_monitor->selected_client,
				Slacker__dir_to_monitor(arg->i));
}

void Monitor__layout_master_stack(Monitor *monitor)
{
	uint32_t i = 0;
	uint32_t n = 0;
	uint32_t h = 0;
	uint32_t mw = 0;
	uint32_t my = 0;
	uint32_t ty = 0;
	Client *temp_client = NULL;

	// Check to see if we have any tiled clients on this monitor.
	for (n = 0, temp_client = Client__next_tiled(monitor->client_list);
	     temp_client;
	     temp_client = Client__next_tiled(temp_client->next), ++n) {
		;
	}
	// If we have no tiled clients, we have nothing to do.
	if (n == 0) {
		return;
	}

	if (n > monitor->nmaster) {
		mw = monitor->nmaster ? monitor->ww * monitor->mfact : 0;
	} else {
		mw = monitor->ww;
	}

	// TODO:  Refactor: Unreadable
	for (i = my = ty = 0,
	    temp_client = Client__next_tiled(monitor->client_list);
	     temp_client;
	     temp_client = Client__next_tiled(temp_client->next), ++i) {
		if (i < monitor->nmaster) {
			h = (monitor->wh - my) / (MIN(n, monitor->nmaster) - i);
			Client__resize(temp_client, monitor->wx,
				       monitor->wy + my,
				       mw - (2 * temp_client->bw),
				       h - (2 * temp_client->bw), 0);

			if (my + HEIGHT(temp_client) < monitor->wh) {
				my += HEIGHT(temp_client);
			}
		} else {
			h = (monitor->wh - ty) / (n - i);
			Client__resize(temp_client, monitor->wx + mw,
				       monitor->wy + ty,
				       monitor->ww - mw - (2 * temp_client->bw),
				       h - (2 * temp_client->bw), 0);
			if (ty + HEIGHT(temp_client) < monitor->wh) {
				ty += HEIGHT(temp_client);
			}
		}
	}
}

/// @brief Toggle the bar on the selected monitor.
///
/// @param `arg` Unused argument
void Slacker__togglebar(const Arg *arg)
{
	Monitor *sm = g_slacker.selected_monitor;

	sm->showbar = !sm->showbar;
	Monitor__updatebarpos(sm);
	XMoveResizeWindow(g_slacker.display, sm->barwin, sm->wx, sm->by, sm->ww,
			  g_slacker.bar_height);
	Slacker__arrange_monitors(sm);
}

/// @brief Toggle the floating state of a client.
void Slacker__togglefloating(const Arg *arg)
{
	Monitor *sm = g_slacker.selected_monitor;
	if (!sm || !sm->selected_client) {
		return;
	}

	// no support for fullscreen windows
	if (sm->selected_client->isfullscreen) {
		return;
	}

	sm->selected_client->isfloating = !sm->selected_client->isfloating ||
					  sm->selected_client->isfixed;

	if (sm->selected_client->isfloating) {
		Client__resize(sm->selected_client, sm->selected_client->x,
			       sm->selected_client->y, sm->selected_client->w,
			       sm->selected_client->h, 0);
	}

	Slacker__arrange_monitors(sm);
}

void Slacker__toggletag(const Arg *arg)
{
	uint32_t newtags = 0;
	Monitor *sm = g_slacker.selected_monitor;

	if (!sm || !sm->selected_client) {
		return;
	}

	newtags = sm->selected_client->tags ^ (arg->ui & TAGMASK);
	if (newtags) {
		sm->selected_client->tags = newtags;
		Slacker__focus(NULL);
		Slacker__arrange_monitors(sm);
	}
}

void Slacker__toggleview(const Arg *arg)
{
	Monitor *sm = g_slacker.selected_monitor;
	if (!sm) {
		return;
	}

	uint32_t newtagset = sm->tagset[sm->seltags] ^ (arg->ui & TAGMASK);

	if (newtagset) {
		sm->tagset[sm->seltags] = newtagset;
		Slacker__focus(NULL);
		Slacker__arrange_monitors(sm);
	}
}

void Slacker__unfocus(Client *client, int setfocus)
{
	if (!client) {
		return;
	}

	Slacker__grab_buttons(client, false);
	XSetWindowBorder(
		g_slacker.display, client->win,
		g_slacker.scheme[SlackerColorscheme_Norm][ColBorder].pixel);

	if (setfocus) {
		XSetInputFocus(g_slacker.display, g_slacker.root,
			       RevertToPointerRoot, CurrentTime);
		XDeleteProperty(
			g_slacker.display, g_slacker.root,
			g_slacker.netatom[SlackerEWMHAtom_NetActiveWindow]);
	}
}

/// @brief Unmaps a client from the window manager.
///
/// @details TODO: This could be refactory into a Client__ function
void Slacker__unmanage(Client *client, int destroyed)
{
	Monitor *temp_monitor = client->mon;
	XWindowChanges wc;

	Client__detach(client);
	Client__detach_from_stack(client);
	if (!destroyed) {
		wc.border_width = client->oldbw;
		// Avoid race conditions
		XGrabServer(g_slacker.display);
		// Set a dummy error handler function
		XSetErrorHandler(xerrordummy);
		// Pass the client window id for input selection
		XSelectInput(g_slacker.display, client->win, NoEventMask);
		// Restore the border
		XConfigureWindow(g_slacker.display, client->win, CWBorderWidth,
				 &wc);
		// Ungrab all buttons
		XUngrabButton(g_slacker.display, AnyButton, AnyModifier,
			      client->win);
		Slacker__set_client_state(client, WithdrawnState);
		XSync(g_slacker.display, False);
		XSetErrorHandler(xerror);
		XUngrabServer(g_slacker.display);
	}
	free(client);
	Slacker__focus(NULL);
	Slacker__update_client_list();
	Slacker__arrange_monitors(temp_monitor);
}

void event_unmapnotify(XEvent *event)
{
	Client *temp_client = NULL;
	XUnmapEvent *ev = &event->xunmap;

	if ((temp_client = Slacker__win_to_client(ev->window))) {
		if (ev->send_event) {
			Slacker__set_client_state(temp_client, WithdrawnState);
		} else {
			Slacker__unmanage(temp_client, 0);
		}
	}
}

void Slacker__updatebars(void)
{
	Monitor *temp_monitor = NULL;
	XSetWindowAttributes wa = { .override_redirect = True,
				    .background_pixmap = ParentRelative,
				    .event_mask = ButtonPressMask |
						  ExposureMask };

	XClassHint ch = { "slacker", "slacker" };
	for (temp_monitor = g_slacker.monitor_list; temp_monitor;
	     temp_monitor = temp_monitor->next) {
		// If the bar exists for this monitor, continue
		if (temp_monitor->barwin) {
			continue;
		}
		// The bar does not exist, create it
		temp_monitor->barwin = XCreateWindow(
			g_slacker.display, g_slacker.root, temp_monitor->wx,
			temp_monitor->by, temp_monitor->ww,
			g_slacker.bar_height, 0,
			DefaultDepth(g_slacker.display, g_slacker.screen),
			CopyFromParent,
			DefaultVisual(g_slacker.display, g_slacker.screen),
			CWOverrideRedirect | CWBackPixmap | CWEventMask, &wa);

		XDefineCursor(
			g_slacker.display, temp_monitor->barwin,
			g_slacker.cursor[SlackerCursorState_Normal]->cursor);
		XMapRaised(g_slacker.display, temp_monitor->barwin);
		XSetClassHint(g_slacker.display, temp_monitor->barwin, &ch);
	}
}

/// @brief Update the status bar position for one monitor
///
/// TODO: Could be moved into a new Bar stucture
void Monitor__updatebarpos(Monitor *monitor)
{
	monitor->wy = monitor->my;
	monitor->wh = monitor->mh;
	if (monitor->showbar) {
		monitor->wh -= g_slacker.bar_height;
		monitor->by = monitor->topbar ? monitor->wy :
						monitor->wy + monitor->wh;
		monitor->wy = monitor->topbar ?
				      monitor->wy + g_slacker.bar_height :
				      monitor->wy;
	} else {
		monitor->by = -g_slacker.bar_height;
	}
}

void Slacker__update_client_list(void)
{
	Client *temp_client = NULL;
	Monitor *temp_monitor = NULL;

	XDeleteProperty(g_slacker.display, g_slacker.root,
			g_slacker.netatom[SlackerEWMHAtom_NetClientList]);

	// For all monitors
	for (temp_monitor = g_slacker.monitor_list; temp_monitor;
	     temp_monitor = temp_monitor->next) {
		// for each client on the monitor
		for (temp_client = temp_monitor->client_list; temp_client;
		     temp_client = temp_client->next) {
			XChangeProperty(
				g_slacker.display, g_slacker.root,
				g_slacker.netatom[SlackerEWMHAtom_NetClientList],
				XA_WINDOW, 32, PropModeAppend,
				(unsigned char *)&(temp_client->win), 1);
		}
	}
}

/// @brief Update the geometry of the screen
///
/// @details For each monitor, update the width and height of the monitor
/// based of the screen width and height. Then update the bar position,
/// and map the current monitor to the root window id.
bool Slacker__updategeom(void)
{
	fprintf(stdout, "%s\n", __PRETTY_FUNCTION__);
	bool dirty = false;

	// Default monitor setup
	if (!g_slacker.monitor_list) {
		g_slacker.monitor_list = Monitor__create();
	}

	if (g_slacker.monitor_list->mw != g_slacker.screen_width ||
	    g_slacker.monitor_list->mh != g_slacker.screen_height) {
		dirty = true;
		g_slacker.monitor_list->mw = g_slacker.monitor_list->ww =
			g_slacker.screen_width;
		g_slacker.monitor_list->mh = g_slacker.monitor_list->wh =
			g_slacker.screen_height;
		Monitor__updatebarpos(g_slacker.monitor_list);
	}

	if (dirty) {
		g_slacker.selected_monitor = g_slacker.monitor_list;
		g_slacker.selected_monitor = Slacker__wintomon(g_slacker.root);
	}
	return dirty;
}

void Slacker__update_numlock_mask(void)
{
	g_slacker.numlockmask = 0;

	XModifierKeymap *modmap = XGetModifierMapping(g_slacker.display);
	for (uint32_t i = 0; i < 8; ++i) {
		for (uint32_t j = 0; j < modmap->max_keypermod; ++j)
			if (modmap->modifiermap[i * modmap->max_keypermod + j] ==
			    XKeysymToKeycode(g_slacker.display, XK_Num_Lock)) {
				g_slacker.numlockmask = (1 << i);
			}
	}

	XFreeModifiermap(modmap);
}

void updatesizehints(Client *c)
{
	long msize;
	XSizeHints size;

	if (!XGetWMNormalHints(g_slacker.display, c->win, &size, &msize))
		/* size is uninitialized, ensure that size.flags aren't used */
		size.flags = PSize;
	if (size.flags & PBaseSize) {
		c->basew = size.base_width;
		c->baseh = size.base_height;
	} else if (size.flags & PMinSize) {
		c->basew = size.min_width;
		c->baseh = size.min_height;
	} else
		c->basew = c->baseh = 0;
	if (size.flags & PResizeInc) {
		c->incw = size.width_inc;
		c->inch = size.height_inc;
	} else
		c->incw = c->inch = 0;
	if (size.flags & PMaxSize) {
		c->maxw = size.max_width;
		c->maxh = size.max_height;
	} else
		c->maxw = c->maxh = 0;
	if (size.flags & PMinSize) {
		c->minw = size.min_width;
		c->minh = size.min_height;
	} else if (size.flags & PBaseSize) {
		c->minw = size.base_width;
		c->minh = size.base_height;
	} else
		c->minw = c->minh = 0;
	if (size.flags & PAspect) {
		c->mina = (float)size.min_aspect.y / size.min_aspect.x;
		c->maxa = (float)size.max_aspect.x / size.max_aspect.y;
	} else
		c->maxa = c->mina = 0.0;
	c->isfixed = (c->maxw && c->maxh && c->maxw == c->minw &&
		      c->maxh == c->minh);
	c->hintsvalid = 1;
}

/// @brief Update the status text in the bar
void Slacker__update_status(void)
{
	if (!Slacker__get_text_prop(g_slacker.root, XA_WM_NAME,
				    g_slacker.status_text,
				    sizeof(g_slacker.status_text))) {
		strcpy(g_slacker.status_text, "slacker-" VERSION);
	}
	Slacker__drawbar(g_slacker.selected_monitor);
}

void Slacker__update_client_title(Client *client)
{
	if (!Slacker__get_text_prop(
		    client->win, g_slacker.netatom[SlackerEWMHAtom_NetWMName],
		    client->name, sizeof client->name)) {
		Slacker__get_text_prop(client->win, XA_WM_NAME, client->name,
				       sizeof client->name);
	}

	// hack to mark broken clients
	if (client->name[0] == '\0') {
		strcpy(client->name, BROKEN);
	}
}

void Slacker__update_window_type(Client *client)
{
	Atom state = Slacker__get_atom_prop(
		client, g_slacker.netatom[SlackerEWMHAtom_NetWMState]);

	Atom wtype = Slacker__get_atom_prop(
		client, g_slacker.netatom[SlackerEWMHAtom_NetWMWindowType]);

	if (state == g_slacker.netatom[SlackerEWMHAtom_NetWMFullscreen]) {
		Slacker__setfullscreen(client, 1);
	}

	if (wtype == g_slacker.netatom[SlackerEWMHAtom_NetWMWindowTypeDialog]) {
		client->isfloating = 1;
	}
}

void Slacker__update_wmhints(Client *client)
{
	XWMHints *wmh;

	if ((wmh = XGetWMHints(g_slacker.display, client->win))) {
		if (client == g_slacker.selected_monitor->selected_client &&
		    wmh->flags & XUrgencyHint) {
			wmh->flags &= ~XUrgencyHint;
			XSetWMHints(g_slacker.display, client->win, wmh);
		} else {
			client->isurgent = (wmh->flags & XUrgencyHint) ? 1 : 0;
		}

		if (wmh->flags & InputHint) {
			client->neverfocus = !wmh->input;
		} else {
			client->neverfocus = 0;
		}

		XFree(wmh);
	}
}

void Slacker__view(const Arg *arg)
{
	Monitor *sm = g_slacker.selected_monitor;
	if (!sm) {
		return;
	}

	if ((arg->ui & TAGMASK) == sm->tagset[sm->seltags]) {
		return;
	}

	// Toggle selected tag set
	sm->seltags ^= 1;
	if (arg->ui & TAGMASK) {
		sm->tagset[sm->seltags] = arg->ui & TAGMASK;
	}

	Slacker__focus(NULL);
	Slacker__arrange_monitors(sm);
}

/// @brief Maps a window id to an existing client.
/// if no client is found, returns NULL
Client *Slacker__win_to_client(Window wid)
{
	Client *temp_client = NULL;
	Monitor *temp_monitor = NULL;

	for (temp_monitor = g_slacker.monitor_list; temp_monitor;
	     temp_monitor = temp_monitor->next) {
		for (temp_client = temp_monitor->client_list; temp_client;
		     temp_client = temp_client->next) {
			if (temp_client->win == wid) {
				return temp_client;
			}
		}
	}

	return NULL;
}

Monitor *Slacker__wintomon(Window wid)
{
	int32_t x, y = 0;
	Client *temp_client = NULL;
	Monitor *temp_monitor = NULL;

	if (wid == g_slacker.root && Slacker__getrootptr(&x, &y)) {
		return Slacker__rect_to_monitor(x, y, 1, 1);
	}

	for (temp_monitor = g_slacker.monitor_list; temp_monitor;
	     temp_monitor = temp_monitor->next) {
		if (wid == temp_monitor->barwin) {
			return temp_monitor;
		}
	}

	if ((temp_client = Slacker__win_to_client(wid))) {
		return temp_client->mon;
	}

	return g_slacker.selected_monitor;
}

/// @brief X11 error handler
///
/// @details There's no way to check accesses to destroyed windows, thus those cases are
/// ignored (especially on UnmapNotify's). Other types of errors call Xlibs
/// default error handler, which may call exit.
int xerror(Display *dpy, XErrorEvent *ee)
{
	if (ee->error_code == BadWindow ||
	    (ee->request_code == X_SetInputFocus &&
	     ee->error_code == BadMatch) ||
	    (ee->request_code == X_PolyText8 &&
	     ee->error_code == BadDrawable) ||
	    (ee->request_code == X_PolyFillRectangle &&
	     ee->error_code == BadDrawable) ||
	    (ee->request_code == X_PolySegment &&
	     ee->error_code == BadDrawable) ||
	    (ee->request_code == X_ConfigureWindow &&
	     ee->error_code == BadMatch) ||
	    (ee->request_code == X_GrabButton && ee->error_code == BadAccess) ||
	    (ee->request_code == X_GrabKey && ee->error_code == BadAccess) ||
	    (ee->request_code == X_CopyArea && ee->error_code == BadDrawable))
		return 0;
	fprintf(stderr,
		"slacker: fatal error: request code=%d, error code=%d\n",
		ee->request_code, ee->error_code);
	return g_slacker.xerrorxlib(dpy, ee); /* may call exit */
}

int xerrordummy(Display *dpy, XErrorEvent *ee)
{
	return 0;
}

/// @brief Startup Error handler to check if another window manager
/// is already running
int xerrorstart(Display *dpy, XErrorEvent *ee)
{
	die("slacker: another window manager is already running");
	return -1;
}

void Slacker__zoom(const Arg *arg)
{
	Monitor *sm = g_slacker.selected_monitor;
	if (!sm) {
		return;
	}

	Client *sc = g_slacker.selected_monitor->selected_client;
	if (!sc) {
		return;
	}

	if (!sm->layouts[sm->selected_layout]->layout_arrange_callback ||
	    sc->isfloating) {
		return;
	}

	if (sc == Client__next_tiled(sm->client_list) &&
	    !(sc = Client__next_tiled(sc->next))) {
		return;
	}

	Client__pop(sc);
}

int main(int argc, char *argv[])
{
	if (DEBUG == 1) {
		printf("Running in debug mode: slacker: pid: %d\n", getpid());
		sleep(15);
	}
	if (argc == 2 && !strcmp("-v", argv[1])) {
		fprintf(stdout, "slacker-" VERSION "\n");
		return EXIT_SUCCESS;
	} else if (argc != 1) {
		die("usage: slacker [-v]");
	} else {
		if (!setlocale(LC_CTYPE, "") || !XSupportsLocale()) {
			fputs("warning: no locale support\n", stderr);
		}

		if (!(g_slacker.display = XOpenDisplay(NULL))) {
			die("slacker: cannot open display");
		}

		Slacker__checkotherwm();
		Slacker__create();

		Slacker__scan();
		Slacker__run();
		Slacker__destroy();
		XCloseDisplay(g_slacker.display);
	}

	return EXIT_SUCCESS;
}
