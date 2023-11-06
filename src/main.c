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
#include "error.h"
#include "events.h"
#include "monitor.h"
#include "utils.h"
#include "swm.h"

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
			x += TEXTW(G_TAGS[i]);
		} while (ev->x >= x && ++i < LENGTH(G_TAGS));

		if (i < LENGTH(G_TAGS)) {
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
	for (i = 0; i < LENGTH(G_CLICKABLE_BUTTONS); ++i) {
		// If we have a match, the callback is not a null function,
		// and
		if (click == G_CLICKABLE_BUTTONS[i].click &&
		    G_CLICKABLE_BUTTONS[i].button_handler_callback &&
		    G_CLICKABLE_BUTTONS[i].button == ev->button &&
		    CLEANMASK(G_CLICKABLE_BUTTONS[i].mask) ==
			    CLEANMASK(ev->state)) {
			G_CLICKABLE_BUTTONS[i].button_handler_callback(
				click == SlackerClick_TagBar &&
						G_CLICKABLE_BUTTONS[i].arg.i ==
							0 ?
					&arg :
					&G_CLICKABLE_BUTTONS[i].arg);
		}
	}
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
				Client__configure(g_slacker.display,
						  temp_client);
			}

			if (ISVISIBLE(temp_client)) {
				XMoveResizeWindow(
					g_slacker.display, temp_client->win,
					temp_client->x, temp_client->y,
					temp_client->w, temp_client->h);
			}
		} else {
			Client__configure(g_slacker.display, temp_client);
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

void event_destroynotify(XEvent *event)
{
	Client *temp_client;
	XDestroyWindowEvent *ev = &event->xdestroywindow;

	if ((temp_client = Slacker__win_to_client(ev->window))) {
		Slacker__unmanage(temp_client, 1);
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

void event_focusin(XEvent *event)
{
	XFocusChangeEvent *ev = &event->xfocus;

	if (g_slacker.selected_monitor->selected_client &&
	    ev->window != g_slacker.selected_monitor->selected_client->win) {
		Slacker__setfocus(g_slacker.selected_monitor->selected_client);
	}
}

void event_keypress(XEvent *event)
{
	XKeyEvent *ev = &event->xkey;
	KeySym keysym =
		XKeycodeToKeysym(g_slacker.display, (KeyCode)ev->keycode, 0);

	for (uint32_t i = 0; i < LENGTH(G_KEYBINDINGS); ++i) {
		if (keysym == G_KEYBINDINGS[i].keysym &&
		    CLEANMASK(G_KEYBINDINGS[i].mod) == CLEANMASK(ev->state) &&
		    G_KEYBINDINGS[i].keymap_callback) {
			G_KEYBINDINGS[i].keymap_callback(
				&(G_KEYBINDINGS[i].arg));
		}
	}
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


int main(int argc, char *argv[])
{
	if (DEBUG == 1) {
		printf("Running in debug mode, attach debugger to pid: '%d'\n", getpid());
		sleep(15);
	}
	if (argc == 2 && !strcmp("-v", argv[1])) {
		fprintf(stdout, "swm-" VERSION "\n");
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
	}

	return EXIT_SUCCESS;
}
