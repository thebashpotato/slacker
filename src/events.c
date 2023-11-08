// Standard Libraries
#include <stdio.h>
#include <stdint.h>

// Swm headers

#include "common.h"
#include "config.h"
#include "events.h"
#include "swm.h"
#include "utils.h"

void Swm__event_loop(XEvent *event)
{
	switch (event->type) {
	case ButtonPress:
		Swm__event_button_press(event);
		break;
	case ClientMessage:
		Swm__event_client_message(event);
		break;
	case ConfigureRequest:
		Swm__event_configure_request(event);
		break;
	case ConfigureNotify:
		Swm__event_configure_notify(event);
		break;
	case DestroyNotify:
		Swm__event_destroy_notify(event);
		break;
	case EnterNotify:
		Swm__event_enter_notify(event);
		break;
	case Expose:
		Swm__event_expose(event);
		break;
	case FocusIn:
		Swm__event_focusin(event);
		break;
	case KeyPress:
		Swm__event_keypress(event);
		break;
	case MappingNotify:
		Swm__event_mapping_notify(event);
		break;
	case MapRequest:
		Swm__event_map_request(event);
		break;
	case MotionNotify:
		Swm__event_motion_notify(event);
		break;
	case PropertyNotify:
		Swm__event_property_notify(event);
		break;
	case UnmapNotify:
		Swm__event_unmap_notify(event);
		break;
	default:
		// if (DEBUG == 1) {
		// 	fprintf(stdout, "Unhandled event: %d\n", event->type);
		// }
		break;
	}
}

void Swm__event_button_press(XEvent *event)
{
	uint32_t i = 0;
	uint32_t x = 0;
	uint32_t click = SlackerClick_RootWin;
	Arg arg = { 0 };
	Client *temp_client = NULL;
	Monitor *temp_monitor = NULL;
	XButtonPressedEvent *ev = &event->xbutton;

	// Focus monitor if necessary
	if ((temp_monitor = Swm__wintomon(ev->window)) &&
	    temp_monitor != g_swm.selected_monitor) {
		Swm__unfocus(g_swm.selected_monitor->selected_client, 1);
		g_swm.selected_monitor = temp_monitor;
		Swm__focus(NULL);
	}

	// Check if the button press was a click on the bar
	if (ev->window == g_swm.selected_monitor->barwin) {
		do {
			x += TEXTW(G_TAGS[i]);
		} while (ev->x >= x && ++i < LENGTH(G_TAGS));

		if (i < LENGTH(G_TAGS)) {
			click = SlackerClick_TagBar;
			arg.ui = 1 << i;
		} else if (ev->x <
			   x + TEXTW(g_swm.selected_monitor->layout_symbol)) {
			click = SlackerClick_LtSymbol;
		} else if (ev->x > g_swm.selected_monitor->ww -
					   (int)TEXTW(g_swm.status_text)) {
			click = SlackerClick_StatusText;
		} else {
			click = SlackerClick_WinTitle;
		}
	}

	if ((temp_client = Swm__win_to_client(ev->window))) {
		Swm__focus(temp_client);
		Swm__restack(g_swm.selected_monitor);
		XAllowEvents(g_swm.ctx.xconn, ReplayPointer, CurrentTime);
		click = SlackerClick_ClientWin;
	}

	// Check to see if we have a button handler for the click we have registered.
	for (i = 0; i < LENGTH(G_CLICKABLE_BUTTONS); ++i) {
		// If we have a match, the callback is not a null function,
		// and
		if (click == G_CLICKABLE_BUTTONS[i].click &&
		    G_CLICKABLE_BUTTONS[i].handler &&
		    G_CLICKABLE_BUTTONS[i].id == ev->button &&
		    CLEANMASK(G_CLICKABLE_BUTTONS[i].event_mask) ==
			    CLEANMASK(ev->state)) {
			G_CLICKABLE_BUTTONS[i].handler(
				click == SlackerClick_TagBar &&
						G_CLICKABLE_BUTTONS[i].arg.i ==
							0 ?
					&arg :
					&G_CLICKABLE_BUTTONS[i].arg);
		}
	}
}

void Swm__event_client_message(XEvent *event)
{
	XClientMessageEvent *cme = &event->xclient;
	Client *temp_client = Swm__win_to_client(cme->window);

	if (!temp_client) {
		return;
	}

	if (cme->message_type == g_swm.netatom[SlackerEWMHAtom_NetWMState]) {
		if (cme->data.l[1] ==
			    g_swm.netatom[SlackerEWMHAtom_NetWMFullscreen] ||
		    cme->data.l[2] ==
			    g_swm.netatom[SlackerEWMHAtom_NetWMFullscreen]) {
			Swm__setfullscreen(
				temp_client,
				(cme->data.l[0] == 1 /* _NET_WM_STATE_ADD    */
				 ||
				 (cme->data.l[0] == 2 /* _NET_WM_STATE_TOGGLE */ &&
				  !temp_client->isfullscreen)));
		}
	} else if (cme->message_type ==
		   g_swm.netatom[SlackerEWMHAtom_NetActiveWindow]) {
		if (temp_client != g_swm.selected_monitor->selected_client &&
		    !temp_client->isurgent) {
			Swm__seturgent(temp_client, 1);
		}
	}
}

void Swm__event_configure_request(XEvent *event)
{
	Client *temp_client = NULL;
	Monitor *temp_monitor = NULL;
	XConfigureRequestEvent *ev = &event->xconfigurerequest;
	XWindowChanges wc;

	if ((temp_client = Swm__win_to_client(ev->window))) {
		if (ev->value_mask & CWBorderWidth) {
			temp_client->bw = ev->border_width;
		} else if (temp_client->isfloating ||
			   !g_swm.selected_monitor
				    ->layouts[g_swm.selected_monitor
						      ->selected_layout]
				    ->handler) {
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
				Client__configure(g_swm.ctx.xconn, temp_client);
			}

			if (ISVISIBLE(temp_client)) {
				XMoveResizeWindow(
					g_swm.ctx.xconn, temp_client->win,
					temp_client->x, temp_client->y,
					temp_client->w, temp_client->h);
			}
		} else {
			Client__configure(g_swm.ctx.xconn, temp_client);
		}
	} else {
		wc.x = ev->x;
		wc.y = ev->y;
		wc.width = ev->width;
		wc.height = ev->height;
		wc.border_width = ev->border_width;
		wc.sibling = ev->above;
		wc.stack_mode = ev->detail;
		XConfigureWindow(g_swm.ctx.xconn, ev->window, ev->value_mask,
				 &wc);
	}
	XSync(g_swm.ctx.xconn, False);
}

void Swm__event_configure_notify(XEvent *event)
{
	Monitor *temp_monitor = NULL;
	Client *temp_client = NULL;
	XConfigureEvent *ev = &event->xconfigure;

	if (ev->window == g_swm.ctx.xroot_id) {
		int32_t dirty = (g_swm.ctx.xscreen_width != ev->width ||
				 g_swm.ctx.xscreen_height != ev->height);
		g_swm.ctx.xscreen_width = ev->width;
		g_swm.ctx.xscreen_height = ev->height;
		if (Swm__updategeom() || dirty) {
			drw_resize(g_swm.draw, g_swm.ctx.xscreen_width,
				   g_swm.bar_height);
			Swm__updatebars();
			for (temp_monitor = g_swm.monitor_list; temp_monitor;
			     temp_monitor = temp_monitor->next) {
				for (temp_client = temp_monitor->client_list;
				     temp_client;
				     temp_client = temp_client->next) {
					if (temp_client->isfullscreen)
						Swm__resize_client(
							temp_client,
							temp_monitor->mx,
							temp_monitor->my,
							temp_monitor->mw,
							temp_monitor->mh);
				}
				XMoveResizeWindow(
					g_swm.ctx.xconn, temp_monitor->barwin,
					temp_monitor->wx, temp_monitor->by,
					temp_monitor->ww, g_swm.bar_height);
			}
			Swm__focus(NULL);
			Swm__arrange_monitors(NULL);
		}
	}
}

void Swm__event_destroy_notify(XEvent *event)
{
	Client *temp_client;
	XDestroyWindowEvent *ev = &event->xdestroywindow;

	if ((temp_client = Swm__win_to_client(ev->window))) {
		Swm__unmanage(temp_client, true);
	}
}

void Swm__event_enter_notify(XEvent *event)
{
	Client *temp_client = NULL;
	Monitor *temp_monitor = NULL;
	XCrossingEvent *ev = &event->xcrossing;

	if ((ev->mode != NotifyNormal || ev->detail == NotifyInferior) &&
	    ev->window != g_swm.ctx.xroot_id) {
		return;
	}

	temp_client = Swm__win_to_client(ev->window);
	temp_monitor = temp_client ? temp_client->mon :
				     Swm__wintomon(ev->window);

	if (temp_monitor != g_swm.selected_monitor) {
		Swm__unfocus(g_swm.selected_monitor->selected_client, 1);
		g_swm.selected_monitor = temp_monitor;
	} else if (!temp_client ||
		   temp_client == g_swm.selected_monitor->selected_client) {
		return;
	}
	Swm__focus(temp_client);
}

void Swm__event_expose(XEvent *event)
{
	Monitor *monitor;
	XExposeEvent *ev = &event->xexpose;

	if (ev->count == 0 && (monitor = Swm__wintomon(ev->window))) {
		Swm__drawbar(monitor);
	}
}

void Swm__event_focusin(XEvent *event)
{
	XFocusChangeEvent *ev = &event->xfocus;

	if (g_swm.selected_monitor->selected_client &&
	    ev->window != g_swm.selected_monitor->selected_client->win) {
		Swm__setfocus(g_swm.selected_monitor->selected_client);
	}
}

void Swm__event_keypress(XEvent *event)
{
	XKeyEvent *ev = &event->xkey;
	KeySym keysym =
		XKeycodeToKeysym(g_swm.ctx.xconn, (KeyCode)ev->keycode, 0);

	for (uint32_t i = 0; i < LENGTH(G_KEYBINDINGS); ++i) {
		if (keysym == G_KEYBINDINGS[i].keysym &&
		    CLEANMASK(G_KEYBINDINGS[i].mod) == CLEANMASK(ev->state) &&
		    G_KEYBINDINGS[i].handler) {
			G_KEYBINDINGS[i].handler(&(G_KEYBINDINGS[i].arg));
		}
	}
}

void Swm__event_mapping_notify(XEvent *event)
{
	XMappingEvent *ev = &event->xmapping;

	XRefreshKeyboardMapping(ev);
	if (ev->request == MappingKeyboard) {
		Swm__grab_keys();
	}
}

void Swm__event_map_request(XEvent *event)
{
	static XWindowAttributes wa;
	XMapRequestEvent *ev = &event->xmaprequest;

	if (!XGetWindowAttributes(g_swm.ctx.xconn, ev->window, &wa) ||
	    wa.override_redirect) {
		return;
	}
	printf("MapRequest\n");

	if (!Swm__win_to_client(ev->window)) {
		Swm__manage_client(ev->window, &wa);
	}
}

void Swm__event_motion_notify(XEvent *event)
{
	static Monitor *s_mon = NULL;
	Monitor *temp_mon = NULL;
	XMotionEvent *ev = &event->xmotion;

	if (ev->window != g_swm.ctx.xroot_id) {
		return;
	}
	if ((temp_mon = Swm__rect_to_monitor(ev->x_root, ev->y_root, 1, 1)) !=
		    s_mon &&
	    s_mon) {
		Swm__unfocus(g_swm.selected_monitor->selected_client, 1);
		g_swm.selected_monitor = temp_mon;
		Swm__focus(NULL);
	}
	s_mon = temp_mon;
}

void Swm__event_property_notify(XEvent *event)
{
	Client *client = NULL;
	Window trans;
	XPropertyEvent *ev = &event->xproperty;

	if ((ev->window == g_swm.ctx.xroot_id) && (ev->atom == XA_WM_NAME)) {
		Swm__update_status();
	} else if (ev->state == PropertyDelete) {
		// TODO: Move this else if to an if at the top of the file.
		return;
	} else if ((client = Swm__win_to_client(ev->window))) {
		switch (ev->atom) {
		case XA_WM_TRANSIENT_FOR:
			if (!client->isfloating &&
			    (XGetTransientForHint(g_swm.ctx.xconn, client->win,
						  &trans)) &&
			    (client->isfloating = (Swm__win_to_client(trans)) !=
						  NULL)) {
				Swm__arrange_monitors(client->mon);
			}
			break;
		case XA_WM_NORMAL_HINTS:
			client->hintsvalid = 0;
			break;
		case XA_WM_HINTS:
			Swm__update_wmhints(client);
			Swm__drawbars();
			break;
		default:
			break;
		}

		if (ev->atom == XA_WM_NAME ||
		    ev->atom == g_swm.netatom[SlackerEWMHAtom_NetWMName]) {
			Swm__update_client_title(client);
			if (client == client->mon->selected_client) {
				Swm__drawbar(client->mon);
			}
		}

		if (ev->atom ==
		    g_swm.netatom[SlackerEWMHAtom_NetWMWindowType]) {
			Swm__update_window_type(client);
		}
	}
}

void Swm__event_unmap_notify(XEvent *event)
{
	Client *temp_client = NULL;
	XUnmapEvent *ev = &event->xunmap;

	if ((temp_client = Swm__win_to_client(ev->window))) {
		if (ev->send_event) {
			Swm__set_client_state(temp_client, WithdrawnState);
		} else {
			Swm__unmanage(temp_client, 0);
		}
	}
}
