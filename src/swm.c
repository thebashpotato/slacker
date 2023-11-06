// X11 Libraries
#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>

// Standard libraries
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

// Slacker headers
#include "client.h"
#include "config.h"
#include "swm.h"
#include "utils.h"

/// @brief Global instance of the slacker window manager.
/// @details Since slacker is single threaded and synchronous,
/// we can get away with this, and it's really fast.
Slacker g_slacker;

void Slacker__create(void)
{
	// TODO: Refactor: Split this function into a create and init and maybe one more
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

	if (!drw_fontset_create(g_slacker.draw, G_USER_FONT)) {
		die("no fonts could be loaded.");
	}

	g_slacker.left_right_padding_sum = g_slacker.draw->fonts->h;
	g_slacker.bar_height = g_slacker.draw->fonts->h + 5;

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
		ecalloc(LENGTH(G_COLORSCHEMES), sizeof(SlackerColor *));
	for (i = 0; i < LENGTH(G_COLORSCHEMES); i++) {
		g_slacker.scheme[i] =
			drw_scm_create(g_slacker.draw, G_COLORSCHEMES[i], 3);
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
	class = ch.res_class ? ch.res_class : CLIENT_WINDOW_BROKEN;
	instance = ch.res_name ? ch.res_name : CLIENT_WINDOW_BROKEN;

	for (uint32_t i = 0; i < LENGTH(G_WINDOW_RULES); ++i) {
		window_rule = &G_WINDOW_RULES[i];
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

int Slacker__applysizehints(Client *client, int *x, int *y, int *w, int *h,
			    int interact)
{
	// TODO: This function is a mess and needs to be cleaned up.
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

	if (G_RESIZE_HINTS || client->isfloating ||
	    !client->mon->layouts[client->mon->selected_layout]
		     ->layout_arrange_callback) {
		if (!client->hintsvalid) {
			Client__update_size_hints(g_slacker.display, client);
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

void Slacker__checkotherwm(void)
{
	g_slacker.xerror_callback = XSetErrorHandler(xerrorstart);
	// This will cause an X11 error to be triggered if another window manager is running.
	// The error is handlered by the slacker custom xerrorxlib function.
	XSelectInput(g_slacker.display, DefaultRootWindow(g_slacker.display),
		     SubstructureRedirectMask);
	XSync(g_slacker.display, False);
	XSetErrorHandler(Slacker__xerror_handler);
	XSync(g_slacker.display, False);
}

int32_t Slacker__xerror_handler(Display *display, XErrorEvent *ee)
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
	return g_slacker.xerror_callback(display, ee); /* may call exit */
}

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
	for (size_t i = 0; i < LENGTH(G_COLORSCHEMES); ++i) {
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

	XCloseDisplay(g_slacker.display);
}

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
	for (i = 0; i < LENGTH(G_TAGS); ++i) {
		w = TEXTW(G_TAGS[i]);
		drw_setscheme(
			g_slacker.draw,
			g_slacker.scheme[monitor->tagset[monitor->seltags] &
							 1 << i ?
						 SlackerColorscheme_Sel :
						 SlackerColorscheme_Norm]);

		drw_text(g_slacker.draw, x, 0, w, g_slacker.bar_height,
			 (g_slacker.left_right_padding_sum / 2), G_TAGS[i],
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

void Slacker__drawbars(void)
{
	Monitor *monitor;

	for (monitor = g_slacker.monitor_list; monitor;
	     monitor = monitor->next) {
		Slacker__drawbar(monitor);
	}
}

void Slacker__focus(Client *client)
{
	Monitor *sm = g_slacker.selected_monitor;

	if (!sm) {
		return;
	}

	if (!client || !ISVISIBLE(client))
		for (client = sm->client_stack; client && !ISVISIBLE(client);
		     client = client->stack_next) {
			;
		}

	if (sm->selected_client && sm->selected_client != client) {
		Slacker__unfocus(sm->selected_client, 0);
	}

	if (client) {
		if (client->mon != sm) {
			sm = client->mon;
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

	sm->selected_client = client;
	Slacker__drawbars();
}

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

int32_t Slacker__getrootptr(int *root_x_return, int *root_y_return)
{
	// Dummy variables that we don't care about.
	int32_t win_return_dummy = 0;
	uint32_t mask_return_dummy = 0;
	Window root_return_dummy, child_return_dummy;

	return XQueryPointer(g_slacker.display, g_slacker.root,
			     &root_return_dummy, &child_return_dummy,
			     root_x_return, root_y_return, &win_return_dummy,
			     &win_return_dummy, &mask_return_dummy);
}

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
	}
	if (n != 0) {
		result = *p;
	}
	XFree(p);
	return result;
}

bool Slacker__get_text_prop(Window w_id, Atom atom, char *text, uint32_t size)
{
	char **list = NULL;
	int32_t n = 0;
	XTextProperty name;

	if (!text || size == 0) {
		return false;
	}
	text[0] = '\0';

	if (!XGetTextProperty(g_slacker.display, w_id, &name, atom) ||
	    !name.nitems) {
		return false;
	}

	if (name.encoding == XA_STRING) {
		strncpy(text, (char *)name.value, size - 1);
	} else if (XmbTextPropertyToTextList(g_slacker.display, &name, &list,
					     &n) >= Success &&
		   (n > 0 && *list)) {
		strncpy(text, *list, size - 1);
		XFreeStringList(list);
	}

	text[size - 1] = '\0';
	XFree(name.value);
	return true;
}

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

		for (uint32_t i = 0; i < LENGTH(G_CLICKABLE_BUTTONS); ++i) {
			// if the button is a client window button, grab it
			if (G_CLICKABLE_BUTTONS[i].click ==
			    SlackerClick_ClientWin) {
				for (uint32_t j = 0; j < LENGTH(modifiers); ++j)
					XGrabButton(
						g_slacker.display,
						G_CLICKABLE_BUTTONS[i].button,
						G_CLICKABLE_BUTTONS[i].mask |
							modifiers[j],
						client->win, False, BUTTONMASK,
						GrabModeAsync, GrabModeSync,
						None, None);
			}
		}
	}
}

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
			for (uint32_t i = 0; i < LENGTH(G_KEYBINDINGS); ++i) {
				// Skip modifier codes, we do that ourselves
				if (G_KEYBINDINGS[i].keysym ==
				    syms[(k - start) * skip]) {
					for (uint32_t j = 0;
					     j < LENGTH(modifiers); ++j) {
						XGrabKey(g_slacker.display, k,
							 G_KEYBINDINGS[i].mod |
								 modifiers[j],
							 g_slacker.root, True,
							 GrabModeAsync,
							 GrabModeAsync);
					}
				}
			}
		}
		XFree(syms);
	}
}

void Slacker__manage(Window w_id, XWindowAttributes *wa)
{
	// TODO: Refactor: Since a new Client is created here, it should be abstracted
	// out into a Client__create function.
	fprintf(stdout, "%s\n", __PRETTY_FUNCTION__);
	Client *new_client = NULL;
	Client *temp_client = NULL;
	Window trans = None;
	XWindowChanges wc;

	new_client = ecalloc(1, sizeof(Client));
	new_client->win = w_id;

	// Geometry
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
	new_client->bw = G_BORDER_PIXEL;

	wc.border_width = new_client->bw;
	XConfigureWindow(g_slacker.display, w_id, CWBorderWidth, &wc);
	XSetWindowBorder(
		g_slacker.display, w_id,
		g_slacker.scheme[SlackerColorscheme_Norm][ColBorder].pixel);

	// Propagates border_width, if size doesn't change
	Client__configure(g_slacker.display, new_client);

	Slacker__update_window_type(new_client);
	Client__update_size_hints(g_slacker.display, new_client);
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

Monitor *Slacker__rect_to_monitor(int x, int y, int w, int h)
{
	// TODO: Refactor: Could be a Monitor__ function
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
	Client__configure(g_slacker.display, client);
	XSync(g_slacker.display, False);
}

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

void Slacker__run(void)
{
	XEvent ev;
	XSync(g_slacker.display, False);
	while (g_slacker.is_running && !XNextEvent(g_slacker.display, &ev)) {
		Slacker__event_loop(&ev);
	}
}

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
	Atom *protocols = NULL;
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

void Slacker__setfullscreen(Client *client, int32_t fullscreen)
{
	// TODO: Refactor: should be a Client__ function
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

void Slacker__seturgent(Client *client, int urgent)
{
	// TODO: Refactor: Could be a Client__ function by passing Display
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

void Slacker__showhide(Client *client)
{
	// TODO: Refactor: Should be a Client__ function
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

void Slacker__unfocus(Client *client, bool setfocus)
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

void Slacker__unmanage(Client *client, bool destroyed)
{
	// TODO: Refactor: into a Client__ function, or atleast partly.
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
		XSetErrorHandler(Slacker__xerror_handler);
		XUngrabServer(g_slacker.display);
	}
	free(client);
	Slacker__focus(NULL);
	Slacker__update_client_list();
	Slacker__arrange_monitors(temp_monitor);
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

bool Slacker__updategeom(void)
{
	fprintf(stdout, "%s\n", __PRETTY_FUNCTION__);
	bool dirty = false;

	// If it doesn't exist, create it
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

void Slacker__update_status(void)
{
	/// BUG: The version is not currently displaying in the status bar.
	if (!Slacker__get_text_prop(g_slacker.root, XA_WM_NAME,
				    g_slacker.status_text,
				    sizeof(g_slacker.status_text))) {
		strcpy(g_slacker.status_text, "swm-" VERSION);
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
		strcpy(client->name, CLIENT_WINDOW_BROKEN);
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

Client *Slacker__win_to_client(Window w_id)
{
	Client *temp_client = NULL;
	Monitor *temp_monitor = NULL;

	for (temp_monitor = g_slacker.monitor_list; temp_monitor;
	     temp_monitor = temp_monitor->next) {
		for (temp_client = temp_monitor->client_list; temp_client;
		     temp_client = temp_client->next) {
			if (temp_client->win == w_id) {
				return temp_client;
			}
		}
	}

	return NULL;
}

Monitor *Slacker__wintomon(Window w_id)
{
	int32_t x, y = 0;
	Client *temp_client = NULL;
	Monitor *temp_monitor = NULL;

	if (w_id == g_slacker.root && Slacker__getrootptr(&x, &y)) {
		return Slacker__rect_to_monitor(x, y, 1, 1);
	}

	for (temp_monitor = g_slacker.monitor_list; temp_monitor;
	     temp_monitor = temp_monitor->next) {
		if (w_id == temp_monitor->barwin) {
			return temp_monitor;
		}
	}

	if ((temp_client = Slacker__win_to_client(w_id))) {
		return temp_client->mon;
	}

	return g_slacker.selected_monitor;
}

///////////////////////////////
/// Keybind Modifier Functions
///////////////////////////////

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
		XSetErrorHandler(Slacker__xerror_handler);
		XUngrabServer(g_slacker.display);
	}
}

void Slacker__increment_n_master(const Arg *arg)
{
	Monitor *active = g_slacker.selected_monitor;
	active->nmaster = MAX(active->nmaster + arg->i, 0);
	Slacker__arrange_monitors(active);
}

void Slacker__move_with_mouse(const Arg *arg)
{
	int32_t x = 0;
	int32_t y = 0;
	int32_t ocx = 0;
	int32_t ocy = 0;
	int32_t nx = 0;
	int32_t ny = 0;
	Client *temp_client = 0;
	Monitor *temp_monitor = NULL;
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
			Slacker__event_loop(&ev);
			break;
		case MotionNotify:
			if ((ev.xmotion.time - lasttime) <= (1000 / 60)) {
				continue;
			}
			lasttime = ev.xmotion.time;

			nx = ocx + (ev.xmotion.x - x);
			ny = ocy + (ev.xmotion.y - y);

			if (abs(g_slacker.selected_monitor->wx - nx) <
			    G_SNAP_PIXEL) {
				nx = g_slacker.selected_monitor->wx;
			} else if (abs((g_slacker.selected_monitor->wx +
					g_slacker.selected_monitor->ww) -
				       (nx + WIDTH(temp_client))) <
				   G_SNAP_PIXEL) {
				nx = g_slacker.selected_monitor->wx +
				     g_slacker.selected_monitor->ww -
				     WIDTH(temp_client);
			}

			if (abs(g_slacker.selected_monitor->wy - ny) <
			    G_SNAP_PIXEL) {
				ny = g_slacker.selected_monitor->wy;
			} else if (abs((g_slacker.selected_monitor->wy +
					g_slacker.selected_monitor->wh) -
				       (ny + HEIGHT(temp_client))) <
				   G_SNAP_PIXEL) {
				ny = g_slacker.selected_monitor->wy +
				     g_slacker.selected_monitor->wh -
				     HEIGHT(temp_client);
			}

			if (!temp_client->isfloating &&
			    g_slacker.selected_monitor
				    ->layouts[g_slacker.selected_monitor
						      ->selected_layout]
				    ->layout_arrange_callback &&
			    (abs(nx - temp_client->x) > G_SNAP_PIXEL ||
			     abs(ny - temp_client->y) > G_SNAP_PIXEL)) {
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

	if ((temp_monitor = Slacker__rect_to_monitor(
		     temp_client->x, temp_client->y, temp_client->w,
		     temp_client->h)) != g_slacker.selected_monitor) {
		Client__send_to_monitor(temp_client, temp_monitor);
		g_slacker.selected_monitor = temp_monitor;
		Slacker__focus(NULL);
	}
}

void Slacker__quit(const Arg *arg)
{
	g_slacker.is_running = false;
}

void Slacker__resize_client_with_mouse(const Arg *arg)
{
	// TODO: Refactor, this function
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
			Slacker__event_loop(&ev);
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
				    (abs(nw - temp_client->w) > G_SNAP_PIXEL ||
				     abs(nh - temp_client->h) > G_SNAP_PIXEL))
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

void Slacker__spawn(const Arg *arg)
{
	struct sigaction sa;

	if (arg->v == G_DMENU_COMMAND) {
		G_DMENU_MONITOR[0] = '0' + g_slacker.selected_monitor->num;
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

void Slacker__tag(const Arg *arg)
{
	Monitor *sm = g_slacker.selected_monitor;
	if (sm->selected_client && arg->ui & TAGMASK) {
		sm->selected_client->tags = arg->ui & TAGMASK;
		Slacker__focus(NULL);
		Slacker__arrange_monitors(sm);
	}
}

void Slacker__tagmon(const Arg *arg)
{
	// TODO: Debug this funciton
	// If there are no monitors return
	if (!g_slacker.selected_monitor->selected_client ||
	    !g_slacker.monitor_list->next) {
		return;
	}

	Client__send_to_monitor(g_slacker.selected_monitor->selected_client,
				Slacker__dir_to_monitor(arg->i));
}

void Slacker__togglebar(const Arg *arg)
{
	Monitor *sm = g_slacker.selected_monitor;

	sm->showbar = !sm->showbar;
	Monitor__updatebarpos(sm);
	XMoveResizeWindow(g_slacker.display, sm->barwin, sm->wx, sm->by, sm->ww,
			  g_slacker.bar_height);
	Slacker__arrange_monitors(sm);
}

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
	Monitor *sm = g_slacker.selected_monitor;

	if (!sm || !sm->selected_client) {
		return;
	}

	uint32_t newtags = sm->selected_client->tags ^ (arg->ui & TAGMASK);
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

void Slacker__view(const Arg *arg)
{
	Monitor *sm = g_slacker.selected_monitor;
	if (!sm) {
		return;
	}

	if ((arg->ui & TAGMASK) == sm->tagset[sm->seltags]) {
		return;
	}

	// Toggle selected tag set for the active monitor.
	sm->seltags ^= 1;
	if (arg->ui & TAGMASK) {
		sm->tagset[sm->seltags] = arg->ui & TAGMASK;
	}

	Slacker__focus(NULL);
	Slacker__arrange_monitors(sm);
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

void Slacker__focus_stack(const Arg *arg)
{
	Monitor *sm = g_slacker.selected_monitor;
	Client *temp_client = NULL;

	// If selected monitor is null,
	// If the selected client is null,
	// or the is fullscreen and the global lock is set, return.
	if (!sm || !sm->selected_client ||
	    (sm->selected_client->isfullscreen && G_LOCK_FULLSCREEN)) {
		return;
	}

	if (arg->i > 0) {
		for (temp_client = sm->selected_client->next;
		     temp_client && !ISVISIBLE(temp_client);
		     temp_client = temp_client->next) {
			;
		}

		if (!temp_client) {
			for (temp_client = sm->client_list;
			     temp_client && !ISVISIBLE(temp_client);
			     temp_client = temp_client->next) {
				;
			}
		}
	} else {
		Client *iter = NULL;
		for (iter = sm->client_list; iter != sm->selected_client;
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

void Slacker__focus_monitor(const Arg *arg)
{
	Monitor *temp_monitor = NULL;

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
