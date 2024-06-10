/// Standard Library
#include <string.h>
#include <stdio.h>

/// Slacker Headers
#include "config.h"
#include "monitor.h"
#include "utils.h"
#include "swm.h"

Monitor *Monitor__new(void)
{
	Monitor *m;

	m = ecalloc(1, sizeof(Monitor));
	m->tag_set[0] = m->tag_set[1] = 1;
	m->master_width_factor = G_MASTER_FACTOR;
	m->num_master = G_MASTER_COUNT;
	m->show_bar = G_SHOW_BAR;
	m->top_bar = G_TOP_BAR;
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
	XUnmapWindow(g_swm.ctx.xconn, monitor->bar_win_id);
	XDestroyWindow(g_swm.ctx.xconn, monitor->bar_win_id);
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

bool Monitor__is_layout_monocle(Monitor *monitor)
{
	return monitor->layouts[monitor->selected_layout]->handler ==
	       Monitor__layout_monocle;
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
	uint32_t num_clients = Monitor__get_num_clients(monitor);

	// If we have clients, override the layout symbol in the bar.
	if (num_clients > 0) {
		snprintf(monitor->layout_symbol, sizeof(monitor->layout_symbol),
			 "[%d]", num_clients);
	}

	for (temp_client = Client__next_tiled(monitor->client_list);
	     temp_client; temp_client = Client__next_tiled(temp_client->next)) {
		Client__resize(temp_client, monitor->wx, monitor->wy,
			       monitor->ww - 2 * temp_client->bw,
			       monitor->wh - 2 * temp_client->bw, 0);
	}
}

void Monitor__layout_master_stack(Monitor *mon)
{
	uint32_t i = 0;
	uint32_t num_tiled_clients = 0;
	uint32_t temp_height = 0;
	uint32_t mon_width = 0;
	uint32_t mon_y = 0;
	uint32_t tiled_y = 0;
	Client *c = NULL;

	// Check to see if we have any tiled clients on this monitor.
	for (num_tiled_clients = 0, c = Client__next_tiled(mon->client_list); c;
	     c = Client__next_tiled(c->next), ++num_tiled_clients) {
		;
	}
	// If we have no tiled clients, we have nothing to do.
	if (num_tiled_clients == 0) {
		return;
	}

	if (num_tiled_clients > mon->num_master) {
		mon_width = mon->num_master ?
				    mon->ww * mon->master_width_factor :
				    0;
	} else {
		mon_width = mon->ww;
	}

	// TODO:  Refactor: Unreadable
	for (i = mon_y = tiled_y = 0, c = Client__next_tiled(mon->client_list);
	     c; c = Client__next_tiled(c->next), ++i) {
		if (i < mon->num_master) {
			temp_height =
				(mon->wh - mon_y) /
				(MIN(num_tiled_clients, mon->num_master) - i);

			// FIXME: G_GAP_PIXEL should not be factoryed into the width
			// calculation.
			Client__resize(c, mon->wx, mon->wy + mon_y,
				       mon_width - (2 * c->bw) +
					       (num_tiled_clients > 1 ?
							G_GAP_PIXEL :
							0),
				       temp_height - (2 * c->bw), 0);

			if (mon_y + HEIGHT(c) < mon->wh) {
				mon_y += HEIGHT(c);
			}
		} else {
			temp_height =
				(mon->wh - tiled_y) / (num_tiled_clients - i);
			Client__resize(c, mon->wx + mon_width,
				       mon->wy + tiled_y,
				       mon->ww - mon_width - (2 * c->bw),
				       temp_height - (2 * c->bw), 0);
			if (tiled_y + HEIGHT(c) < mon->wh) {
				tiled_y += HEIGHT(c);
			}
		}
	}
}

void Monitor__updatebarpos(Monitor *monitor)
{
	// TODO: Refactor: Could be moved into a new Bar stucture
	monitor->wy = monitor->my;
	monitor->wh = monitor->mh;
	if (monitor->show_bar) {
		monitor->wh -= g_swm.bar_height;
		monitor->bar_y = monitor->top_bar ? monitor->wy :
						    monitor->wy + monitor->wh;
		monitor->wy = monitor->top_bar ?
				      monitor->wy + g_swm.bar_height :
				      monitor->wy;
	} else {
		monitor->bar_y = -g_swm.bar_height;
	}
}
