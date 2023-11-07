/// Standard Libraries
#include <signal.h>
#include <unistd.h>

// Slacker Headers
#include "common.h"
#include "events.h"
#include "modifiers.h"
#include "monitor.h"
#include "swm.h"
#include "utils.h"

void Swm__kill_client(const Arg *arg)
{
	if (!g_swm.selected_monitor->selected_client) {
		return;
	}

	if (!Swm__send_event(g_swm.selected_monitor->selected_client,
			     g_swm.wmatom[SlackerDefaultAtom_WMDelete])) {
		XGrabServer(g_swm.xconn);
		XSetErrorHandler(xerrordummy);
		XSetCloseDownMode(g_swm.xconn, DestroyAll);
		XKillClient(g_swm.xconn,
			    g_swm.selected_monitor->selected_client->win);
		XSync(g_swm.xconn, False);
		XSetErrorHandler(Swm__xerror_handler);
		XUngrabServer(g_swm.xconn);
	}
}

void Swm__increment_n_master(const Arg *arg)
{
	Monitor *active = g_swm.selected_monitor;
	active->nmaster = MAX(active->nmaster + arg->i, 0);
	Swm__arrange_monitors(active);
}

void Swm__move_with_mouse(const Arg *arg)
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
	if (!(temp_client = g_swm.selected_monitor->selected_client)) {
		return;
	}

	// No support moving fullscreen windows by mouse
	if (temp_client->isfullscreen) {
		return;
	}

	Swm__restack(g_swm.selected_monitor);
	ocx = temp_client->x;
	ocy = temp_client->y;

	if (XGrabPointer(g_swm.xconn, g_swm.root_wid, False, MOUSEMASK,
			 GrabModeAsync, GrabModeAsync, None,
			 g_swm.cursor[SlackerCursorState_Move]->cursor,
			 CurrentTime) != GrabSuccess) {
		return;
	}

	// If window is invalid, return.
	if (!Swm__getrootptr(&x, &y)) {
		return;
	}

	do {
		XMaskEvent(g_swm.xconn,
			   MOUSEMASK | ExposureMask | SubstructureRedirectMask,
			   &ev);
		switch (ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			Swm__event_loop(&ev);
			break;
		case MotionNotify:
			if ((ev.xmotion.time - lasttime) <= (1000 / 60)) {
				continue;
			}
			lasttime = ev.xmotion.time;

			nx = ocx + (ev.xmotion.x - x);
			ny = ocy + (ev.xmotion.y - y);

			if (abs(g_swm.selected_monitor->wx - nx) <
			    G_SNAP_PIXEL) {
				nx = g_swm.selected_monitor->wx;
			} else if (abs((g_swm.selected_monitor->wx +
					g_swm.selected_monitor->ww) -
				       (nx + WIDTH(temp_client))) <
				   G_SNAP_PIXEL) {
				nx = g_swm.selected_monitor->wx +
				     g_swm.selected_monitor->ww -
				     WIDTH(temp_client);
			}

			if (abs(g_swm.selected_monitor->wy - ny) <
			    G_SNAP_PIXEL) {
				ny = g_swm.selected_monitor->wy;
			} else if (abs((g_swm.selected_monitor->wy +
					g_swm.selected_monitor->wh) -
				       (ny + HEIGHT(temp_client))) <
				   G_SNAP_PIXEL) {
				ny = g_swm.selected_monitor->wy +
				     g_swm.selected_monitor->wh -
				     HEIGHT(temp_client);
			}

			if (!temp_client->isfloating &&
			    g_swm.selected_monitor
				    ->layouts[g_swm.selected_monitor
						      ->selected_layout]
				    ->handler &&
			    (abs(nx - temp_client->x) > G_SNAP_PIXEL ||
			     abs(ny - temp_client->y) > G_SNAP_PIXEL)) {
				Swm__togglefloating(NULL);
			}

			if (!g_swm.selected_monitor
				     ->layouts[g_swm.selected_monitor
						       ->selected_layout]
				     ->handler ||
			    temp_client->isfloating) {
				Client__resize(temp_client, nx, ny,
					       temp_client->w, temp_client->h,
					       1);
			}
			break;
		}
	} while (ev.type != ButtonRelease);

	XUngrabPointer(g_swm.xconn, CurrentTime);

	if ((temp_monitor = Swm__rect_to_monitor(
		     temp_client->x, temp_client->y, temp_client->w,
		     temp_client->h)) != g_swm.selected_monitor) {
		Client__send_to_monitor(temp_client, temp_monitor);
		g_swm.selected_monitor = temp_monitor;
		Swm__focus(NULL);
	}
}

void Swm__quit(const Arg *arg)
{
	g_swm.is_running = false;
}

void Swm__resize_client_with_mouse(const Arg *arg)
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
	if (!(temp_client = g_swm.selected_monitor->selected_client)) {
		return;
	}

	// No support for resizing fullscreen windows by mouse
	if (temp_client->isfullscreen) {
		return;
	}

	Swm__restack(g_swm.selected_monitor);
	ocx = temp_client->x;
	ocy = temp_client->y;

	if (XGrabPointer(g_swm.xconn, g_swm.root_wid, False, MOUSEMASK,
			 GrabModeAsync, GrabModeAsync, None,
			 g_swm.cursor[SlackerCursorState_Resize]->cursor,
			 CurrentTime) != GrabSuccess) {
		return;
	}

	XWarpPointer(g_swm.xconn, None, temp_client->win, 0, 0, 0, 0,
		     temp_client->w + temp_client->bw - 1,
		     temp_client->h + temp_client->bw - 1);
	do {
		XMaskEvent(g_swm.xconn,
			   MOUSEMASK | ExposureMask | SubstructureRedirectMask,
			   &ev);

		switch (ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			Swm__event_loop(&ev);
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
				    g_swm.selected_monitor->wx &&
			    temp_client->mon->wx + nw <=
				    g_swm.selected_monitor->wx +
					    g_swm.selected_monitor->ww &&
			    temp_client->mon->wy + nh >=
				    g_swm.selected_monitor->wy &&
			    temp_client->mon->wy + nh <=
				    g_swm.selected_monitor->wy +
					    g_swm.selected_monitor->wh) {
				if (!temp_client->isfloating &&
				    g_swm.selected_monitor
					    ->layouts[g_swm.selected_monitor
							      ->selected_layout]
					    ->handler &&
				    (abs(nw - temp_client->w) > G_SNAP_PIXEL ||
				     abs(nh - temp_client->h) > G_SNAP_PIXEL))
					Swm__togglefloating(NULL);
			}

			if (!g_swm.selected_monitor
				     ->layouts[g_swm.selected_monitor
						       ->selected_layout]
				     ->handler ||
			    temp_client->isfloating) {
				Client__resize(temp_client, temp_client->x,
					       temp_client->y, nw, nh, 1);
			}
			break;
		}
	} while (ev.type != ButtonRelease);

	XWarpPointer(g_swm.xconn, None, temp_client->win, 0, 0, 0, 0,
		     temp_client->w + (temp_client->bw - 1),
		     temp_client->h + (temp_client->bw - 1));

	XUngrabPointer(g_swm.xconn, CurrentTime);

	while (XCheckMaskEvent(g_swm.xconn, EnterWindowMask, &ev)) {
		;
	}

	if ((temp_monitor = Swm__rect_to_monitor(
		     temp_client->x, temp_client->y, temp_client->w,
		     temp_client->h)) != g_swm.selected_monitor) {
		Client__send_to_monitor(temp_client, temp_monitor);
		g_swm.selected_monitor = temp_monitor;
		Swm__focus(NULL);
	}
}

void Swm__setlayout(const Arg *arg)
{
	Monitor *sm = g_swm.selected_monitor;
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
		Swm__arrange_monitors(sm);
	} else {
		Swm__drawbar(sm);
	}
}

void Swm__setmfact(const Arg *arg)
{
	Monitor *sm = g_swm.selected_monitor;

	if (!sm || !arg || !sm->layouts[sm->selected_layout]->handler) {
		return;
	}

	float factor = arg->f < 1.0 ? arg->f + sm->mfact : arg->f - 1.0;

	if (factor < 0.05 || factor > 0.95) {
		return;
	}
	sm->mfact = factor;
	Swm__arrange_monitors(sm);
}

void Swm__spawn(const Arg *arg)
{
	struct sigaction sa;

	if (arg->v == G_DMENU_COMMAND) {
		G_DMENU_MONITOR[0] = '0' + g_swm.selected_monitor->num;
	}

	if (fork() == 0) {
		if (g_swm.xconn) {
			close(ConnectionNumber(g_swm.xconn));
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

void Swm__tag(const Arg *arg)
{
	Monitor *sm = g_swm.selected_monitor;
	if (sm->selected_client && arg->ui & TAGMASK) {
		sm->selected_client->tags = arg->ui & TAGMASK;
		Swm__focus(NULL);
		Swm__arrange_monitors(sm);
	}
}

void Swm__tagmon(const Arg *arg)
{
	// TODO: Debug this funciton
	// If there are no monitors return
	if (!g_swm.selected_monitor->selected_client ||
	    !g_swm.monitor_list->next) {
		return;
	}

	Client__send_to_monitor(g_swm.selected_monitor->selected_client,
				Swm__dir_to_monitor(arg->i));
}

void Swm__togglebar(const Arg *arg)
{
	Monitor *sm = g_swm.selected_monitor;

	sm->showbar = !sm->showbar;
	Monitor__updatebarpos(sm);
	XMoveResizeWindow(g_swm.xconn, sm->barwin, sm->wx, sm->by, sm->ww,
			  g_swm.bar_height);
	Swm__arrange_monitors(sm);
}

void Swm__togglefloating(const Arg *arg)
{
	Monitor *sm = g_swm.selected_monitor;
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

	Swm__arrange_monitors(sm);
}

void Swm__toggletag(const Arg *arg)
{
	Monitor *sm = g_swm.selected_monitor;

	if (!sm || !sm->selected_client) {
		return;
	}

	uint32_t newtags = sm->selected_client->tags ^ (arg->ui & TAGMASK);
	if (newtags) {
		sm->selected_client->tags = newtags;
		Swm__focus(NULL);
		Swm__arrange_monitors(sm);
	}
}

void Swm__toggleview(const Arg *arg)
{
	Monitor *sm = g_swm.selected_monitor;
	if (!sm) {
		return;
	}

	uint32_t newtagset = sm->tagset[sm->selected_tags] ^
			     (arg->ui & TAGMASK);

	if (newtagset) {
		sm->tagset[sm->selected_tags] = newtagset;
		Swm__focus(NULL);
		Swm__arrange_monitors(sm);
	}
}

void Swm__view(const Arg *arg)
{
	Monitor *sm = g_swm.selected_monitor;
	if (!sm) {
		return;
	}

	if ((arg->ui & TAGMASK) == sm->tagset[sm->selected_tags]) {
		return;
	}

	// Toggle selected tag set for the active monitor.
	sm->selected_tags ^= 1;
	if (arg->ui & TAGMASK) {
		sm->tagset[sm->selected_tags] = arg->ui & TAGMASK;
	}

	Swm__focus(NULL);
	Swm__arrange_monitors(sm);
}

void Swm__zoom(const Arg *arg)
{
	Monitor *sm = g_swm.selected_monitor;
	if (!sm) {
		return;
	}

	Client *sc = g_swm.selected_monitor->selected_client;
	if (!sc) {
		return;
	}

	if (!sm->layouts[sm->selected_layout]->handler || sc->isfloating) {
		return;
	}

	if (sc == Client__next_tiled(sm->client_list) &&
	    !(sc = Client__next_tiled(sc->next))) {
		return;
	}

	Client__pop(sc);
}

void Swm__focus_stack(const Arg *arg)
{
	Monitor *sm = g_swm.selected_monitor;
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
		Swm__focus(temp_client);
		Swm__restack(g_swm.selected_monitor);
	}
}

void Swm__focus_monitor(const Arg *arg)
{
	Monitor *temp_monitor = NULL;

	if (!g_swm.monitor_list->next) {
		return;
	}

	if ((temp_monitor = Swm__dir_to_monitor(arg->i)) ==
	    g_swm.selected_monitor) {
		return;
	}

	Swm__unfocus(g_swm.selected_monitor->selected_client, 0);
	g_swm.selected_monitor = temp_monitor;
	Swm__focus(NULL);
}
