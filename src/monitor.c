#include "config.h"
#include "monitor.h"
#include "utils.h"

Monitor *Monitor__new(void)
{
	Monitor *m;

	m = ecalloc(1, sizeof(Monitor));
	m->tagset[0] = m->tagset[1] = 1;
	m->mfact = G_MASTER_FACTOR;
	m->nmaster = G_MASTER_COUNT;
	m->showbar = G_SHOW_BAR;
	m->topbar = G_TOP_BAR;
	m->layouts[0] = &G_LAYOUTS[0];
	m->layouts[1] = &G_LAYOUTS[1 % LENGTH(G_LAYOUTS)];
	strncpy(m->layout_symbol, G_LAYOUTS[0].symbol, sizeof m->layout_symbol);
	return m;
}

void Monitor__delete(Monitor *monitor)
{
	Monitor *temp_mon = NULL;

	if (monitor == g_swm.monitor_list) {
		g_swm.monitor_list = g_swm.monitor_list->next;
	} else {
		for (temp_mon = g_swm.monitor_list;
		     temp_mon && (temp_mon->next != monitor);
		     temp_mon = temp_mon->next) {
			;
		}
		temp_mon->next = monitor->next;
	}
	XUnmapWindow(g_swm.xconn, monitor->barwin);
	XDestroyWindow(g_swm.xconn, monitor->barwin);
	free(monitor);
}

void Monitor__arrange(Monitor *monitor)
{
	strncpy(monitor->layout_symbol,
		monitor->layouts[monitor->selected_layout]->symbol,
		sizeof(monitor->layout_symbol));

	if (monitor->layouts[monitor->selected_layout]->handler) {
		monitor->layouts[monitor->selected_layout]->handler(monitor);
	}
}

int32_t Monitor__get_num_clients(Monitor *monitor)
{
	uint32_t n = 0;
	Client *c = NULL;
	// collect number of visible clients on this monitor.
	for (c = monitor->client_list; c; c = c->next) {
		if (ISVISIBLE(c)) {
			n++;
		}
	}
	return n;
}

void Monitor__layout_monocle(Monitor *monitor)
{
	Client *temp_client = NULL;
	uint32_t number_of_clients = Monitor__get_num_clients(monitor);

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

void Monitor__updatebarpos(Monitor *monitor)
{
	// TODO: Refactor: Could be moved into a new Bar stucture
	monitor->wy = monitor->my;
	monitor->wh = monitor->mh;
	if (monitor->showbar) {
		monitor->wh -= g_swm.bar_height;
		monitor->by = monitor->topbar ? monitor->wy :
						monitor->wy + monitor->wh;
		monitor->wy = monitor->topbar ? monitor->wy + g_swm.bar_height :
						monitor->wy;
	} else {
		monitor->by = -g_swm.bar_height;
	}
}
