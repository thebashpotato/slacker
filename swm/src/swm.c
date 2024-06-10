// X11 Libraries
#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>

// Standard libraries
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

// Slacker headers
#include "client.h"
#include "config.h"
#include "common.h"
#include "error.h"
#include "events.h"
#include "swm.h"
#include "utils.h"
#include "modifiers.h"
#include "monitor.h"

Swm g_swm;

/// @brief Allows the ctx to log itself to stdout
///
/// @param `ctx` The client to log: client->log(client)
static void Ctx__log(Ctx *this)
{
#if (DEBUG == true)
	fprintf(stdout, "\nScreen id: %d\n", this->xscreen_id);
	fprintf(stdout, "W: %d\n", this->xscreen_width);
	fprintf(stdout, "H: %d\n", this->xscreen_height);
#endif
}

Ctx Ctx__new(void)
{
	Ctx ctx;

	// Open a connection to the X server, if we can't get a handle we can't run.
	// This will usually fail if the DISPLAY environment variable is not set,
	// or if the X server is not running.
	if (!(ctx.xconn = XOpenDisplay(NULL))) {
		die("swm: cannot open display");
	}

	ctx.xscreen_id = DefaultScreen(ctx.xconn);
	ctx.xscreen_width = DisplayWidth(ctx.xconn, ctx.xscreen_id);
	ctx.xscreen_height = DisplayHeight(ctx.xconn, ctx.xscreen_id);
	ctx.xroot_id = RootWindow(ctx.xconn, ctx.xscreen_id);
	ctx.xewmh_id = XCreateSimpleWindow(ctx.xconn, ctx.xroot_id, 0, 0, 1, 1,
					   0, 0, 0);
	ctx.log = Ctx__log;

	return ctx;
}

void Ctx__delete(Ctx *ctx)
{
	if (ctx) {
		ctx->xscreen_id = 0;
		ctx->xscreen_width = 0;
		ctx->xscreen_height = 0;
		ctx->xroot_id = 0;
		XDestroyWindow(ctx->xconn, ctx->xewmh_id);
		XCloseDisplay(ctx->xconn);
	}
}

/////////////////////////////////////////////////////////////
/// 				Private Functions
/////////////////////////////////////////////////////////////

/// @brief Check if another window manager is running.
///
/// @details Private function, can only be called after a X server connection has been established.
/// Called only once in `Swm__init`
static void Swm__checkotherwm(void)
{
	if (g_swm.ctx.xconn) {
		g_swm.xerror_callback = XSetErrorHandler(xerrorstart);
		// This will cause an X11 error to be triggered if another window manager is running.
		// The error is handlered by the slacker custom xerrorxlib function.
		XSelectInput(g_swm.ctx.xconn,
			     DefaultRootWindow(g_swm.ctx.xconn),
			     SubstructureRedirectMask);

		XSync(g_swm.ctx.xconn, False);
		XSetErrorHandler(Swm__xerror_handler);
		XSync(g_swm.ctx.xconn, False);
	}
}

/// @brief Initializes the X11 context, and sets sane defaults.
///
/// @details Private function, only called once in `Swm__init`
static void Swm__init(void)
{
	g_swm.ctx = Ctx__new();
	g_swm.bar_height = 0;
	g_swm.left_right_padding_sum = 0;
	g_swm.xerror_callback = NULL;
	g_swm.numlockmask = 0;
	g_swm.is_running = false;
	for (uint32_t i = 0; i < SlackerCursorState_Last; ++i) {
		g_swm.cursor[i] = NULL;
	}
	g_swm.scheme = NULL;
	g_swm.draw = NULL;
	g_swm.monitor_list = NULL;
	g_swm.selected_monitor = NULL;
}

/// @brief Remove all child and transient windows from the window manager.
///
/// @details Private Function: Runs before slacker starts, this is to ensure that no windows
/// from previous window managers are still lurking about.
static void Swm__scan(void)
{
	uint32_t number_child_windows = 0;
	Window parent_return;
	Window child_return;
	Window *list_of_windows = NULL;
	XWindowAttributes wa;

	if (XQueryTree(g_swm.ctx.xconn, g_swm.ctx.xroot_id, &parent_return,
		       &child_return, &list_of_windows,
		       &number_child_windows)) {
		for (uint32_t i = 0; i < number_child_windows; ++i) {
			if (!XGetWindowAttributes(g_swm.ctx.xconn,
						  list_of_windows[i], &wa) ||
			    wa.override_redirect ||
			    XGetTransientForHint(g_swm.ctx.xconn,
						 list_of_windows[i],
						 &parent_return)) {
				continue;
			}

			if (wa.map_state == IsViewable ||
			    Swm__getstate(list_of_windows[i]) == IconicState) {
				Swm__manage_client(list_of_windows[i], &wa);
			}
		}
		// Clear transients
		for (uint32_t i = 0; i < number_child_windows; ++i) {
			if (!XGetWindowAttributes(g_swm.ctx.xconn,
						  list_of_windows[i], &wa))
				continue;
			if (XGetTransientForHint(g_swm.ctx.xconn,
						 list_of_windows[i],
						 &parent_return) &&
			    (wa.map_state == IsViewable ||
			     Swm__getstate(list_of_windows[i]) ==
				     IconicState)) {
				Swm__manage_client(list_of_windows[i], &wa);
			}
		}
		if (list_of_windows) {
			XFree(list_of_windows);
		}
	}
}

/// @brief Initializes the draw object, fonts, and bar.
///
/// @details Private function, only called once in `Swm__init`
static void Swm__init_fonts(void)
{
	g_swm.draw = drw_create(g_swm.ctx.xconn, g_swm.ctx.xscreen_id,
				g_swm.ctx.xroot_id, g_swm.ctx.xscreen_width,
				g_swm.ctx.xscreen_height);

	if (!drw_fontset_create(g_swm.draw, G_USER_FONT)) {
		die("no fonts could be loaded.");
	}

	// TODO: This will be moved to the future Bar struct
	g_swm.left_right_padding_sum = g_swm.draw->fonts->h + 5;
	g_swm.bar_height = g_swm.draw->fonts->h + 5;
}

/// @brief Sets up Wm atoms, and net atoms
///
/// @details Private function, only called once in `Swm__init`
///
/// https://tronche.com/gui/x/xlib/window-information/XInternAtom.html
///
/// @return Atom The UTF8_STRING atom
Atom static Swm__init_atoms(void)
{
	Atom utf8string = XInternAtom(g_swm.ctx.xconn, "UTF8_STRING", False);

	g_swm.wmatom[SlackerDefaultAtom_WMProtocols] =
		XInternAtom(g_swm.ctx.xconn, "WM_PROTOCOLS", False);

	g_swm.wmatom[SlackerDefaultAtom_WMDelete] =
		XInternAtom(g_swm.ctx.xconn, "WM_DELETE_WINDOW", False);

	g_swm.wmatom[SlackerDefaultAtom_WMState] =
		XInternAtom(g_swm.ctx.xconn, "WM_STATE", False);

	g_swm.wmatom[SlackerDefaultAtom_WMTakeFocus] =
		XInternAtom(g_swm.ctx.xconn, "WM_TAKE_FOCUS", False);

	g_swm.netatom[SlackerEWMHAtom_NetActiveWindow] =
		XInternAtom(g_swm.ctx.xconn, "_NET_ACTIVE_WINDOW", False);

	g_swm.netatom[SlackerEWMHAtom_NetSupported] =
		XInternAtom(g_swm.ctx.xconn, "_NET_SUPPORTED", False);

	g_swm.netatom[SlackerEWMHAtom_NetWMName] =
		XInternAtom(g_swm.ctx.xconn, "_NET_WM_NAME", False);

	g_swm.netatom[SlackerEWMHAtom_NetWMState] =
		XInternAtom(g_swm.ctx.xconn, "_NET_WM_STATE", False);

	g_swm.netatom[SlackerEWMHAtom_NetWMCheck] =
		XInternAtom(g_swm.ctx.xconn, "_NET_SUPPORTING_WM_CHECK", False);

	g_swm.netatom[SlackerEWMHAtom_NetWMFullscreen] =
		XInternAtom(g_swm.ctx.xconn, "_NET_WM_STATE_FULLSCREEN", False);

	g_swm.netatom[SlackerEWMHAtom_NetWMWindowType] =
		XInternAtom(g_swm.ctx.xconn, "_NET_WM_WINDOW_TYPE", False);

	g_swm.netatom[SlackerEWMHAtom_NetWMWindowTypeDialog] = XInternAtom(
		g_swm.ctx.xconn, "_NET_WM_WINDOW_TYPE_DIALOG", False);

	g_swm.netatom[SlackerEWMHAtom_NetClientList] =
		XInternAtom(g_swm.ctx.xconn, "_NET_CLIENT_LIST", False);

	return utf8string;
}

/// @brief Initializes the supporting window for EWMH and set properties
static void Swm__init_ewmh_support(Atom utf8string)
{
	XChangeProperty(g_swm.ctx.xconn, g_swm.ctx.xewmh_id,
			g_swm.netatom[SlackerEWMHAtom_NetWMCheck], XA_WINDOW,
			32, PropModeReplace,
			(unsigned char *)&g_swm.ctx.xewmh_id, 1);

	XChangeProperty(g_swm.ctx.xconn, g_swm.ctx.xewmh_id,
			g_swm.netatom[SlackerEWMHAtom_NetWMName], utf8string, 8,
			PropModeReplace, (unsigned char *)"swm", 3);

	XChangeProperty(g_swm.ctx.xconn, g_swm.ctx.xroot_id,
			g_swm.netatom[SlackerEWMHAtom_NetWMCheck], XA_WINDOW,
			32, PropModeReplace,
			(unsigned char *)&g_swm.ctx.xewmh_id, 1);

	// EWMH support per view
	XChangeProperty(g_swm.ctx.xconn, g_swm.ctx.xroot_id,
			g_swm.netatom[SlackerEWMHAtom_NetSupported], XA_ATOM,
			32, PropModeReplace, (unsigned char *)g_swm.netatom,
			SlackerEWMHAtom_NetLast);

	XDeleteProperty(g_swm.ctx.xconn, g_swm.ctx.xroot_id,
			g_swm.netatom[SlackerEWMHAtom_NetClientList]);
}

/// @brief Initializes the cursor states which are:
///		- Normal
///		- Resize
///		- Move
///
/// @details Private function, only called once in `Swm__init`
///
/// @param wa The XSetWindowAttributes struct to set the cursor on
static void Swm__init_cursor_states(XSetWindowAttributes *wa)
{
	g_swm.cursor[SlackerCursorState_Normal] =
		drw_cur_create(g_swm.draw, XC_left_ptr);
	g_swm.cursor[SlackerCursorState_Resize] =
		drw_cur_create(g_swm.draw, XC_sizing);
	g_swm.cursor[SlackerCursorState_Move] =
		drw_cur_create(g_swm.draw, XC_fleur);

	wa->cursor = g_swm.cursor[SlackerCursorState_Normal]->cursor;
}

/// @brief Initializes the appearance of the window manager
///
/// @details Private function, only called once in `Swm__init`
static void Swm__init_appearance(void)
{
	g_swm.scheme = ecalloc(LENGTH(G_COLORSCHEMES), sizeof(SlackerColor *));
	for (uint32_t i = 0; i < LENGTH(G_COLORSCHEMES); ++i) {
		g_swm.scheme[i] =
			drw_scm_create(g_swm.draw, G_COLORSCHEMES[i], 3);
	}
}

/////////////////////////////////////////////////////////////
/// 				Public Functions
/////////////////////////////////////////////////////////////

/// @brief Initializes the X11, cleans up processes, checks for other running window managers.
void Swm__startup(void)
{
	XSetWindowAttributes wa;
	if (!g_swm.is_running) {
		// Initialize the main fields of slacker with sane defaults.
		Swm__init();

		// Check to see if a different window manager is running.
		Swm__checkotherwm();

		// If the X context was initialized, and there are no other window managers running,
		// we can assume we are good to run the window manager.
		g_swm.is_running = true;

		// Create Draw object, and fonts
		Swm__init_fonts();

		// Creates monitors and sets the current monitor to the first one
		Swm__updategeom();

		// Init Atoms
		Atom utf8string = Swm__init_atoms();

		// Add support for EWMH and NetWM
		Swm__init_ewmh_support(utf8string);

		// Init cursors
		Swm__init_cursor_states(&wa);

		// Init appearance
		Swm__init_appearance();

		// Register the events we plan to support with X
		wa.event_mask = SubstructureRedirectMask |
				SubstructureNotifyMask | ButtonPressMask |
				PointerMotionMask | EnterWindowMask |
				LeaveWindowMask | StructureNotifyMask |
				PropertyChangeMask;

		XChangeWindowAttributes(g_swm.ctx.xconn, g_swm.ctx.xroot_id,
					CWEventMask | CWCursor, &wa);
		XSelectInput(g_swm.ctx.xconn, g_swm.ctx.xroot_id,
			     wa.event_mask);

		// Init bars
		Swm__updatebars();
		Swm__update_status();

		Swm__grab_keys();
		Swm__focus(NULL);
		Swm__scan();
	}
}

void Swm__delete(void)
{
	Arg a = { .ui = ~0 };
	Layout foo = { "", NULL };
	Monitor *temp_monitor = NULL;

	Swm__view(&a);
	g_swm.selected_monitor
		->layouts[g_swm.selected_monitor->selected_layout] = &foo;

	// iterate through all monitors and unmanage all client windows
	for (temp_monitor = g_swm.monitor_list; temp_monitor;
	     temp_monitor = temp_monitor->next) {
		while (temp_monitor->client_stack) {
			Swm__unmanage(temp_monitor->client_stack, 0);
		}
	}

	// force ungrabbing of any key presses on the root window
	XUngrabKey(g_swm.ctx.xconn, AnyKey, AnyModifier, g_swm.ctx.xroot_id);

	// free all monitors
	while (g_swm.monitor_list) {
		Monitor__delete(g_swm.monitor_list);
	}

	// free all cursors
	for (size_t i = 0; i < SlackerCursorState_Last; ++i) {
		drw_cur_free(g_swm.draw, g_swm.cursor[i]);
	}

	// free all color schemes
	for (size_t i = 0; i < LENGTH(G_COLORSCHEMES); ++i) {
		if (g_swm.scheme[i]) {
			free(g_swm.scheme[i]);
		}
	}
	if (g_swm.scheme) {
		free(g_swm.scheme);
	}

	// Free the drawable abstraction
	drw_free(g_swm.draw);

	XSync(g_swm.ctx.xconn, False);
	XSetInputFocus(g_swm.ctx.xconn, PointerRoot, RevertToPointerRoot,
		       CurrentTime);
	XDeleteProperty(g_swm.ctx.xconn, g_swm.ctx.xroot_id,
			g_swm.netatom[SlackerEWMHAtom_NetActiveWindow]);

	// Close the X context
	Ctx__delete(&g_swm.ctx);
}

int32_t Swm__xerror_handler(Display *xconn, XErrorEvent *ee)
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
	    (ee->request_code == X_CopyArea && ee->error_code == BadDrawable)) {
		return 0;
	}
	fprintf(stderr,
		"slacker: fatal error: request code=%d, error code=%d\n",
		ee->request_code, ee->error_code);
	return g_swm.xerror_callback(xconn, ee); /* may call exit */
}

void Swm__applyrules(Client *client)
{
	const char *class = NULL;
	const char *instance = NULL;
	const SlackerWindowRule *window_rule = NULL;
	Monitor *temp_monitor = NULL;
	XClassHint ch = { NULL, NULL };

	// Rule matching
	client->isfloating = 0;
	client->tags = 0;
	XGetClassHint(g_swm.ctx.xconn, client->win, &ch);
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
			for (temp_monitor = g_swm.monitor_list;
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
			       client->mon->tag_set[client->mon->selected_tags];
}

int Swm__applysizehints(Client *client, int *x, int *y, int *w, int *h,
			int interact)
{
	// TODO: This function is a mess and needs to be cleaned up.
	int32_t baseismin = 0;
	Monitor *temp_monitor = client->mon;

	// set minimum possible
	*w = MAX(1, *w);
	*h = MAX(1, *h);

	if (interact) {
		if (*x > g_swm.ctx.xscreen_width) {
			*x = g_swm.ctx.xscreen_width - WIDTH(client);
		}
		if (*y > g_swm.ctx.xscreen_height) {
			*y = g_swm.ctx.xscreen_height - HEIGHT(client);
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

	if (*h < g_swm.bar_height) {
		*h = g_swm.bar_height;
	}
	if (*w < g_swm.bar_height) {
		*w = g_swm.bar_height;
	}

	if (G_RESIZE_HINTS || client->isfloating ||
	    !client->mon->layouts[client->mon->selected_layout]->handler) {
		if (!client->hintsvalid) {
			Client__update_size_hints(g_swm.ctx.xconn, client);
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

void Swm__arrange_monitors(Monitor *monitor)
{
	if (monitor) {
		Swm__showhide(monitor->client_stack);
	} else {
		for (monitor = g_swm.monitor_list; monitor;
		     monitor = monitor->next) {
			Swm__showhide(monitor->client_stack);
		}
	}

	if (monitor) {
		Monitor__arrange(monitor);
		Swm__restack(monitor);
	} else {
		for (monitor = g_swm.monitor_list; monitor;
		     monitor = monitor->next) {
			Monitor__arrange(monitor);
		}
	}
}

Monitor *Swm__dir_to_monitor(int dir)
{
	Monitor *temp_monitor = NULL;

	if (dir > 0) {
		if (!(temp_monitor = g_swm.selected_monitor->next)) {
			temp_monitor = g_swm.monitor_list;
		}

	} else if (g_swm.selected_monitor == g_swm.monitor_list) {
		for (temp_monitor = g_swm.monitor_list; temp_monitor->next;
		     temp_monitor = temp_monitor->next) {
			;
		}

	} else {
		for (temp_monitor = g_swm.monitor_list;
		     temp_monitor->next != g_swm.selected_monitor;
		     temp_monitor = temp_monitor->next) {
			;
		}
	}

	return temp_monitor;
}

void Swm__drawbar(Monitor *monitor)
{
	int32_t x = 0;
	int32_t w = 0;
	int32_t text_width = 0;
	int32_t boxs = g_swm.draw->fonts->h / 9;
	int32_t boxw = g_swm.draw->fonts->h / 6 + 2;
	uint32_t i = 0;
	uint32_t occ = 0;
	uint32_t urgent = 0;
	Client *temp_client;

	if (!monitor->show_bar) {
		return;
	}

	// Draw status first so it can be overdrawn by tags later.
	// Status is only drawn on the selected monitor
	if (monitor == g_swm.selected_monitor) {
		// Calculate the width of the status text and add 2x padding
		text_width = TEXTW(g_swm.status_text) -
			     (g_swm.left_right_padding_sum);

		drw_setscheme(g_swm.draw,
			      g_swm.scheme[SlackerColorscheme_Norm]);

		drw_text(g_swm.draw, (monitor->ww - text_width), 0, text_width,
			 g_swm.bar_height, 0, g_swm.status_text, 0);
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
			g_swm.draw,
			g_swm.scheme[monitor->tag_set[monitor->selected_tags] &
						     1 << i ?
					     SlackerColorscheme_Sel :
					     SlackerColorscheme_Norm]);

		drw_text(g_swm.draw, x, 0, w, g_swm.bar_height,
			 (g_swm.left_right_padding_sum / 2), G_TAGS[i],
			 urgent & 1 << i);

		if (occ & 1 << i) {
			drw_rect(g_swm.draw, (x + boxs), boxs, boxw, boxw,
				 monitor == g_swm.selected_monitor &&
					 g_swm.selected_monitor
						 ->selected_client &&
					 g_swm.selected_monitor->selected_client
							 ->tags &
						 1 << i,
				 urgent & 1 << i);
		}

		x += w;
	}

	// Draw the layout symbol
	w = TEXTW(monitor->layout_symbol);
	drw_setscheme(g_swm.draw, g_swm.scheme[SlackerColorscheme_Norm]);
	x = drw_text(g_swm.draw, x, 0, w, g_swm.bar_height,
		     (g_swm.left_right_padding_sum / 2), monitor->layout_symbol,
		     0);

	if ((w = monitor->ww - text_width - x) > g_swm.bar_height) {
		if (monitor->selected_client) {
			enum SlackerColorscheme colorscheme =
				(monitor == g_swm.selected_monitor ?
					 SlackerColorscheme_Sel :
					 SlackerColorscheme_Norm);

			drw_setscheme(g_swm.draw, g_swm.scheme[colorscheme]);

			drw_text(g_swm.draw, x, 0, w, g_swm.bar_height,
				 (g_swm.left_right_padding_sum / 2),
				 monitor->selected_client->name, 0);

			if (monitor->selected_client->isfloating) {
				drw_rect(g_swm.draw, x + boxs, boxs, boxw, boxw,
					 monitor->selected_client->isfixed, 0);
			}

		} else {
			drw_setscheme(g_swm.draw,
				      g_swm.scheme[SlackerColorscheme_Norm]);

			drw_rect(g_swm.draw, x, 0, w, g_swm.bar_height, 1, 1);
		}
	}

	drw_map(g_swm.draw, monitor->bar_win_id, 0, 0, monitor->ww,
		g_swm.bar_height);
}

void Swm__drawbars(void)
{
	Monitor *monitor;

	for (monitor = g_swm.monitor_list; monitor; monitor = monitor->next) {
		Swm__drawbar(monitor);
	}
}

void Swm__focus(Client *client)
{
	Monitor *sm = g_swm.selected_monitor;

	if (!sm) {
		return;
	}

	if (!client || !ISVISIBLE(client))
		for (client = sm->client_stack; client && !ISVISIBLE(client);
		     client = client->stack_next) {
			;
		}

	if (sm->selected_client && sm->selected_client != client) {
		Swm__unfocus(sm->selected_client, 0);
	}

	if (client) {
		if (client->mon != sm) {
			sm = client->mon;
		}

		if (client->isurgent) {
			Swm__seturgent(client, 0);
		}

		Client__detach_from_stack(client);
		Client__attach_to_stack(client);
		Swm__grab_buttons(client, true);

		XSetWindowBorder(
			g_swm.ctx.xconn, client->win,
			g_swm.scheme[SlackerColorscheme_Sel][ColBorder].pixel);

		Swm__setfocus(client);
	} else {
		XSetInputFocus(g_swm.ctx.xconn, g_swm.ctx.xroot_id,
			       RevertToPointerRoot, CurrentTime);

		XDeleteProperty(g_swm.ctx.xconn, g_swm.ctx.xroot_id,
				g_swm.netatom[SlackerEWMHAtom_NetActiveWindow]);
	}

	sm->selected_client = client;
	Swm__drawbars();
}

Atom Swm__get_atom_prop(Client *client, Atom prop)
{
	int32_t di;
	uint64_t dl;
	unsigned char *p = NULL;
	Atom da, atom = None;

	if (XGetWindowProperty(g_swm.ctx.xconn, client->win, prop, 0L,
			       sizeof(atom), False, XA_ATOM, &da, &di, &dl, &dl,
			       &p) == Success &&
	    p) {
		atom = *(Atom *)p;
		XFree(p);
	}
	return atom;
}

int32_t Swm__getrootptr(int *root_x_return, int *root_y_return)
{
	// Dummy variables that we don't care about.
	int32_t win_return_dummy = 0;
	uint32_t mask_return_dummy = 0;
	Window root_return_dummy, child_return_dummy;

	return XQueryPointer(g_swm.ctx.xconn, g_swm.ctx.xroot_id,
			     &root_return_dummy, &child_return_dummy,
			     root_x_return, root_y_return, &win_return_dummy,
			     &win_return_dummy, &mask_return_dummy);
}

int64_t Swm__getstate(Window wid)
{
	int32_t format = 0;
	int64_t result = -1;
	unsigned char *p = NULL;
	uint64_t n = 0;
	uint64_t extra = 0;
	Atom real;

	if (XGetWindowProperty(g_swm.ctx.xconn, wid,
			       g_swm.wmatom[SlackerDefaultAtom_WMState], 0L, 2L,
			       False, g_swm.wmatom[SlackerDefaultAtom_WMState],
			       &real, &format, &n, &extra,
			       (unsigned char **)&p) != Success) {
	}
	if (n != 0) {
		result = *p;
	}
	XFree(p);
	return result;
}

bool Swm__get_text_prop(Window w_id, Atom atom, char *text, uint32_t size)
{
	char **list = NULL;
	int32_t n = 0;
	XTextProperty name = { NULL, None, 0, 0 };

	if (!text || size == 0) {
		return false;
	}
	text[0] = '\0';

	if (!XGetTextProperty(g_swm.ctx.xconn, w_id, &name, atom) ||
	    !name.nitems) {
		return false;
	}

	if (name.encoding == XA_STRING) {
		strncpy(text, (char *)name.value, size - 1);
	} else if (XmbTextPropertyToTextList(g_swm.ctx.xconn, &name, &list,
					     &n) >= Success &&
		   (n > 0 && *list)) {
		strncpy(text, *list, size - 1);
		XFreeStringList(list);
	}

	text[size - 1] = '\0';
	XFree(name.value);
	return true;
}

void Swm__grab_buttons(Client *client, bool focused)
{
	Swm__update_numlock_mask();
	{
		// Clear all buttons
		XUngrabButton(g_swm.ctx.xconn, AnyButton, AnyModifier,
			      client->win);

		if (!focused) {
			XGrabButton(g_swm.ctx.xconn, AnyButton, AnyModifier,
				    client->win, False, BUTTONMASK,
				    GrabModeSync, GrabModeSync, None, None);
		}

		uint32_t modifiers[] = { 0, LockMask, g_swm.numlockmask,
					 (g_swm.numlockmask | LockMask) };

		for (uint32_t i = 0; i < LENGTH(G_CLICKABLE_BUTTONS); ++i) {
			// if the button is a client window button, grab it
			if (G_CLICKABLE_BUTTONS[i].click ==
			    SlackerClick_ClientWin) {
				for (uint32_t j = 0; j < LENGTH(modifiers); ++j)
					XGrabButton(
						g_swm.ctx.xconn,
						G_CLICKABLE_BUTTONS[i].id,
						G_CLICKABLE_BUTTONS[i]
								.event_mask |
							modifiers[j],
						client->win, False, BUTTONMASK,
						GrabModeAsync, GrabModeSync,
						None, None);
			}
		}
	}
}

void Swm__grab_keys(void)
{
	Swm__update_numlock_mask();
	{
		uint32_t modifiers[] = { 0, LockMask, g_swm.numlockmask,
					 g_swm.numlockmask | LockMask };

		int32_t start = 0;
		int32_t end = 0;
		int32_t skip = 0;

		XUngrabKey(g_swm.ctx.xconn, AnyKey, AnyModifier,
			   g_swm.ctx.xroot_id);
		XDisplayKeycodes(g_swm.ctx.xconn, &start, &end);
		KeySym *syms = XGetKeyboardMapping(g_swm.ctx.xconn, start,
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
						XGrabKey(g_swm.ctx.xconn, k,
							 G_KEYBINDINGS[i].mod |
								 modifiers[j],
							 g_swm.ctx.xroot_id,
							 True, GrabModeAsync,
							 GrabModeAsync);
					}
				}
			}
		}
		XFree(syms);
	}
}

void Swm__manage_client(Window w_id, XWindowAttributes *wa)
{
	Window trans = None;
	XWindowChanges wc;

	Client *new_client = NULL;
	Client *temp_client = NULL;

	if (XGetTransientForHint(g_swm.ctx.xconn, w_id, &trans) &&
	    (temp_client = Swm__win_to_client(trans))) {
		new_client = Client__new(w_id, wa, temp_client->mon);
		new_client->tags = temp_client->tags;
	} else {
		new_client = Client__new(w_id, wa, g_swm.selected_monitor);
		Swm__applyrules(new_client);
	}
	Swm__update_client_title(new_client);

	wc.border_width = new_client->bw;
	XConfigureWindow(g_swm.ctx.xconn, w_id, CWBorderWidth, &wc);
	XSetWindowBorder(
		g_swm.ctx.xconn, w_id,
		g_swm.scheme[SlackerColorscheme_Norm][ColBorder].pixel);

	// Propagates border_width, if size doesn't change
	Client__configure(g_swm.ctx.xconn, new_client);

	Swm__update_window_type(new_client);
	Client__update_size_hints(g_swm.ctx.xconn, new_client);

	Swm__update_wmhints(new_client);
	XSelectInput(g_swm.ctx.xconn, w_id,
		     EnterWindowMask | FocusChangeMask | PropertyChangeMask |
			     StructureNotifyMask);

	Swm__grab_buttons(new_client, false);

	if (!new_client->isfloating) {
		new_client->isfloating = new_client->oldstate =
			trans != None || new_client->isfixed;
	}

	if (new_client->isfloating) {
		XRaiseWindow(g_swm.ctx.xconn, new_client->win);
	}

	Client__attach(new_client);
	Client__attach_to_stack(new_client);
	XChangeProperty(g_swm.ctx.xconn, g_swm.ctx.xroot_id,
			g_swm.netatom[SlackerEWMHAtom_NetClientList], XA_WINDOW,
			32, PropModeAppend, (unsigned char *)&(new_client->win),
			1);

	XMoveResizeWindow(g_swm.ctx.xconn, new_client->win,
			  new_client->x + 2 * g_swm.ctx.xscreen_width,
			  new_client->y, new_client->w,
			  new_client->h); /* some windows require this */

	Swm__set_client_state(new_client, NormalState);
	if (new_client->mon == g_swm.selected_monitor) {
		Swm__unfocus(g_swm.selected_monitor->selected_client, 0);
	}

	new_client->mon->selected_client = new_client;
	Swm__arrange_monitors(new_client->mon);
	XMapWindow(g_swm.ctx.xconn, new_client->win);
	Swm__focus(NULL);
}

Monitor *Swm__rect_to_monitor(int x, int y, int w, int h)
{
	// TODO: Refactor: Could be a Monitor__ function
	Monitor *temp_mon = NULL;
	Monitor *ret = g_swm.selected_monitor;
	int32_t a = 0;
	int32_t area = 0;

	for (temp_mon = g_swm.monitor_list; temp_mon;
	     temp_mon = temp_mon->next) {
		if ((a = INTERSECT(x, y, w, h, temp_mon)) > area) {
			area = a;
			ret = temp_mon;
		}
	}
	return ret;
}

void Swm__resize_client(Client *client, int x, int y, int w, int h)
{
	XWindowChanges wc = Client__update_dimensions(client, x, y, w, h);

	XConfigureWindow(g_swm.ctx.xconn, client->win,
			 CWX | CWY | CWWidth | CWHeight | CWBorderWidth, &wc);

	Client__configure(g_swm.ctx.xconn, client);

	XSync(g_swm.ctx.xconn, False);
}

void Swm__restack(Monitor *monitor)
{
	Client *temp_client = NULL;
	XEvent ev;
	XWindowChanges wc;

	Swm__drawbar(monitor);
	if (!monitor->selected_client) {
		return;
	}

	// if the selected client is floating, and the arrange callback is not null, raise the window.
	if (monitor->selected_client->isfloating ||
	    !monitor->layouts[monitor->selected_layout]->handler) {
		XRaiseWindow(g_swm.ctx.xconn, monitor->selected_client->win);
	}

	// if the layout out callback function is not null, stack the windows
	if (monitor->layouts[monitor->selected_layout]->handler) {
		wc.stack_mode = Below;
		wc.sibling = monitor->bar_win_id;
		for (temp_client = monitor->client_stack; temp_client;
		     temp_client = temp_client->stack_next) {
			if (!temp_client->isfloating &&
			    ISVISIBLE(temp_client)) {
				XConfigureWindow(g_swm.ctx.xconn,
						 temp_client->win,
						 CWSibling | CWStackMode, &wc);
				wc.sibling = temp_client->win;
			}
		}
	}

	XSync(g_swm.ctx.xconn, false);
	while (XCheckMaskEvent(g_swm.ctx.xconn, EnterWindowMask, &ev)) {
		;
	}
}

void Swm__run(void)
{
	XEvent ev;
	XSync(g_swm.ctx.xconn, False);
	while (g_swm.is_running && !XNextEvent(g_swm.ctx.xconn, &ev)) {
		Swm__event_loop(&ev);
	}
}

void Swm__set_client_state(Client *client, int64_t state)
{
	int64_t data[] = { state, None };

	XChangeProperty(g_swm.ctx.xconn, client->win,
			g_swm.wmatom[SlackerDefaultAtom_WMState],
			g_swm.wmatom[SlackerDefaultAtom_WMState], 32,
			PropModeReplace, (unsigned char *)data, 2);
}

bool Swm__send_event(Client *client, Atom proto)
{
	// size of protocols list
	int32_t number_of_protocols = 0;
	// list of protocols
	Atom *protocols = NULL;
	// Set to true if 1 or more protocols are supported by this client
	bool exists = false;

	// Returns the list of atoms stored in the WM_PROTOCOLS property on the specified window.
	// These atoms describe window manager protocols in which the owner of this window is willing to participate.
	if (XGetWMProtocols(g_swm.ctx.xconn, client->win, &protocols,
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
			g_swm.wmatom[SlackerDefaultAtom_WMProtocols];
		ev.xclient.format = 32;
		ev.xclient.data.l[0] = proto;
		ev.xclient.data.l[1] = CurrentTime;
		// The XSendEvent() function identifies the destination window,
		// determines which clients should receive the specified events,
		// and ignores any active grabs.
		XSendEvent(g_swm.ctx.xconn, client->win, False, NoEventMask,
			   &ev);
	}
	return exists;
}

void Swm__setfocus(Client *client)
{
	if (!client->neverfocus) {
		XSetInputFocus(g_swm.ctx.xconn, client->win,
			       RevertToPointerRoot, CurrentTime);
		XChangeProperty(g_swm.ctx.xconn, g_swm.ctx.xroot_id,
				g_swm.netatom[SlackerEWMHAtom_NetActiveWindow],
				XA_WINDOW, 32, PropModeReplace,
				(unsigned char *)&(client->win), 1);
	}
	Swm__send_event(client, g_swm.wmatom[SlackerDefaultAtom_WMTakeFocus]);
}

void Swm__setfullscreen(Client *client, int32_t fullscreen)
{
	// TODO: Refactor: should be a Client__ function
	if (fullscreen && !client->isfullscreen) {
		XChangeProperty(
			g_swm.ctx.xconn, client->win,
			g_swm.netatom[SlackerEWMHAtom_NetWMState], XA_ATOM, 32,
			PropModeReplace,
			(unsigned char *)&g_swm
				.netatom[SlackerEWMHAtom_NetWMFullscreen],
			1);
		client->isfullscreen = 1;
		client->oldstate = client->isfloating;
		client->oldbw = client->bw;
		client->bw = 0;
		client->isfloating = 1;
		Swm__resize_client(client, client->mon->mx, client->mon->my,
				   client->mon->mw, client->mon->mh);
		XRaiseWindow(g_swm.ctx.xconn, client->win);
	} else if (!fullscreen && client->isfullscreen) {
		XChangeProperty(g_swm.ctx.xconn, client->win,
				g_swm.netatom[SlackerEWMHAtom_NetWMState],
				XA_ATOM, 32, PropModeReplace,
				(unsigned char *)0, 0);

		client->isfullscreen = 0;
		client->isfloating = client->oldstate;
		client->bw = client->oldbw;
		client->x = client->oldx;
		client->y = client->oldy;
		client->w = client->oldw;
		client->h = client->oldh;
		Swm__resize_client(client, client->x, client->y, client->w,
				   client->h);
		Swm__arrange_monitors(client->mon);
	}
}

void Swm__seturgent(Client *client, int urgent)
{
	// TODO: Refactor: Could be a Client__ function by passing Display
	XWMHints *wmh = NULL;

	client->isurgent = urgent;
	if (!(wmh = XGetWMHints(g_swm.ctx.xconn, client->win))) {
		return;
	}
	wmh->flags = urgent ? (wmh->flags | XUrgencyHint) :
			      (wmh->flags & ~XUrgencyHint);
	XSetWMHints(g_swm.ctx.xconn, client->win, wmh);
	XFree(wmh);
}

void Swm__showhide(Client *client)
{
	// TODO: Refactor: Should be a Client__ function
	if (!client) {
		return;
	}
	if (ISVISIBLE(client)) {
		// Show clients top down
		XMoveWindow(g_swm.ctx.xconn, client->win, client->x, client->y);
		if ((!client->mon->layouts[client->mon->selected_layout]
			      ->handler ||
		     client->isfloating) &&
		    !client->isfullscreen)
			Client__resize(client, client->x, client->y, client->w,
				       client->h, 0);
		Swm__showhide(client->stack_next);
	} else {
		// Hide clients bottom up
		Swm__showhide(client->stack_next);
		XMoveWindow(g_swm.ctx.xconn, client->win, WIDTH(client) * -2,
			    client->y);
	}
}

void Swm__unfocus(Client *client, bool setfocus)
{
	if (!client) {
		return;
	}

	Swm__grab_buttons(client, false);
	XSetWindowBorder(
		g_swm.ctx.xconn, client->win,
		g_swm.scheme[SlackerColorscheme_Norm][ColBorder].pixel);

	if (setfocus) {
		XSetInputFocus(g_swm.ctx.xconn, g_swm.ctx.xroot_id,
			       RevertToPointerRoot, CurrentTime);
		XDeleteProperty(g_swm.ctx.xconn, g_swm.ctx.xroot_id,
				g_swm.netatom[SlackerEWMHAtom_NetActiveWindow]);
	}
}

void Swm__unmanage(Client *client, bool destroyed)
{
	Monitor *temp_monitor = client->mon;
	XWindowChanges wc;

	if (!destroyed) {
		wc.border_width = client->oldbw;
		// Avoid race conditions
		XGrabServer(g_swm.ctx.xconn);
		// Set a dummy error handler function
		XSetErrorHandler(xerrordummy);
		// Pass the client window id for input selection
		XSelectInput(g_swm.ctx.xconn, client->win, NoEventMask);
		// Restore the border
		XConfigureWindow(g_swm.ctx.xconn, client->win, CWBorderWidth,
				 &wc);
		// Ungrab all buttons
		XUngrabButton(g_swm.ctx.xconn, AnyButton, AnyModifier,
			      client->win);
		Swm__set_client_state(client, WithdrawnState);
		XSync(g_swm.ctx.xconn, False);
		XSetErrorHandler(Swm__xerror_handler);
		XUngrabServer(g_swm.ctx.xconn);
	}

	Client__delete(client);
	Swm__focus(NULL);
	Swm__update_client_list();
	Swm__arrange_monitors(temp_monitor);
}

void Swm__updatebars(void)
{
	Monitor *temp_monitor = NULL;
	XSetWindowAttributes wa = { .override_redirect = True,
				    .background_pixmap = ParentRelative,
				    .event_mask = ButtonPressMask |
						  ExposureMask };

	XClassHint ch = { "slacker", "slacker" };
	for (temp_monitor = g_swm.monitor_list; temp_monitor;
	     temp_monitor = temp_monitor->next) {
		// If the bar exists for this monitor, continue
		if (temp_monitor->bar_win_id) {
			continue;
		}
		// The bar does not exist, create it
		temp_monitor->bar_win_id = XCreateWindow(
			g_swm.ctx.xconn, g_swm.ctx.xroot_id, temp_monitor->wx,
			temp_monitor->bar_y, temp_monitor->ww, g_swm.bar_height,
			0, DefaultDepth(g_swm.ctx.xconn, g_swm.ctx.xscreen_id),
			CopyFromParent,
			DefaultVisual(g_swm.ctx.xconn, g_swm.ctx.xscreen_id),
			CWOverrideRedirect | CWBackPixmap | CWEventMask, &wa);

		XDefineCursor(g_swm.ctx.xconn, temp_monitor->bar_win_id,
			      g_swm.cursor[SlackerCursorState_Normal]->cursor);
		XMapRaised(g_swm.ctx.xconn, temp_monitor->bar_win_id);
		XSetClassHint(g_swm.ctx.xconn, temp_monitor->bar_win_id, &ch);
	}
}

void Swm__update_client_list(void)
{
	Client *temp_client = NULL;
	Monitor *temp_monitor = NULL;

	XDeleteProperty(g_swm.ctx.xconn, g_swm.ctx.xroot_id,
			g_swm.netatom[SlackerEWMHAtom_NetClientList]);

	// For all monitors
	for (temp_monitor = g_swm.monitor_list; temp_monitor;
	     temp_monitor = temp_monitor->next) {
		// for each client on the monitor
		for (temp_client = temp_monitor->client_list; temp_client;
		     temp_client = temp_client->next) {
			XChangeProperty(
				g_swm.ctx.xconn, g_swm.ctx.xroot_id,
				g_swm.netatom[SlackerEWMHAtom_NetClientList],
				XA_WINDOW, 32, PropModeAppend,
				(unsigned char *)&(temp_client->win), 1);
		}
	}
}

bool Swm__updategeom(void)
{
	bool dirty = false;

	// If it doesn't exist, create it
	if (!g_swm.monitor_list) {
		g_swm.monitor_list = Monitor__new();
	}

	if (g_swm.monitor_list->mw != g_swm.ctx.xscreen_width ||
	    g_swm.monitor_list->mh != g_swm.ctx.xscreen_height) {
		dirty = true;

		g_swm.monitor_list->mw = g_swm.monitor_list->ww =
			g_swm.ctx.xscreen_width;

		g_swm.monitor_list->mh = g_swm.monitor_list->wh =
			g_swm.ctx.xscreen_height;

		Monitor__updatebarpos(g_swm.monitor_list);
	}

	if (dirty) {
		g_swm.selected_monitor = g_swm.monitor_list;
		g_swm.selected_monitor = Swm__wintomon(g_swm.ctx.xroot_id);
	}

	g_swm.ctx.log(&g_swm.ctx);
	return dirty;
}

void Swm__update_numlock_mask(void)
{
	g_swm.numlockmask = 0;

	XModifierKeymap *modmap = XGetModifierMapping(g_swm.ctx.xconn);
	for (uint32_t i = 0; i < 8; ++i) {
		for (uint32_t j = 0; j < modmap->max_keypermod; ++j)
			if (modmap->modifiermap[i * modmap->max_keypermod + j] ==
			    XKeysymToKeycode(g_swm.ctx.xconn, XK_Num_Lock)) {
				g_swm.numlockmask = (1 << i);
			}
	}

	XFreeModifiermap(modmap);
}

void Swm__update_status(void)
{
	if (!Swm__get_text_prop(g_swm.ctx.xroot_id, XA_WM_NAME,
				g_swm.status_text, sizeof(g_swm.status_text))) {
		strcpy(g_swm.status_text, "swm-" VERSION);
	}
	Swm__drawbar(g_swm.selected_monitor);
}

void Swm__update_client_title(Client *client)
{
	if (!Swm__get_text_prop(client->win,
				g_swm.netatom[SlackerEWMHAtom_NetWMName],
				client->name, sizeof client->name)) {
		Swm__get_text_prop(client->win, XA_WM_NAME, client->name,
				   sizeof client->name);
	}

	// hack to mark broken clients
	if (client->name[0] == '\0') {
		strcpy(client->name, CLIENT_WINDOW_BROKEN);
	}
}

void Swm__update_window_type(Client *client)
{
	Atom state = Swm__get_atom_prop(
		client, g_swm.netatom[SlackerEWMHAtom_NetWMState]);

	Atom wtype = Swm__get_atom_prop(
		client, g_swm.netatom[SlackerEWMHAtom_NetWMWindowType]);

	if (state == g_swm.netatom[SlackerEWMHAtom_NetWMFullscreen]) {
		Swm__setfullscreen(client, 1);
	}

	if (wtype == g_swm.netatom[SlackerEWMHAtom_NetWMWindowTypeDialog]) {
		client->isfloating = 1;
	}
}

void Swm__update_wmhints(Client *client)
{
	XWMHints *wmh;

	if ((wmh = XGetWMHints(g_swm.ctx.xconn, client->win))) {
		if (client == g_swm.selected_monitor->selected_client &&
		    wmh->flags & XUrgencyHint) {
			wmh->flags &= ~XUrgencyHint;
			XSetWMHints(g_swm.ctx.xconn, client->win, wmh);
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

Client *Swm__win_to_client(Window w_id)
{
	Client *temp_client = NULL;
	Monitor *temp_monitor = NULL;

	for (temp_monitor = g_swm.monitor_list; temp_monitor;
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

Monitor *Swm__wintomon(Window w_id)
{
	int32_t x, y = 0;
	Client *temp_client = NULL;
	Monitor *temp_monitor = NULL;

	if (w_id == g_swm.ctx.xroot_id && Swm__getrootptr(&x, &y)) {
		return Swm__rect_to_monitor(x, y, 1, 1);
	}

	for (temp_monitor = g_swm.monitor_list; temp_monitor;
	     temp_monitor = temp_monitor->next) {
		if (w_id == temp_monitor->bar_win_id) {
			return temp_monitor;
		}
	}

	if ((temp_client = Swm__win_to_client(w_id))) {
		return temp_client->mon;
	}

	return g_swm.selected_monitor;
}
