#ifndef SWM_MONITOR_H
#define SWM_MONITOR_H

// X11 Libraries
#include <X11/Xlib.h>
#include <X11/keysym.h>

// Standard Libraries
#include <bits/stdint-intn.h>
#include <bits/stdint-uintn.h>
#include <stdbool.h>

// Slacker Headers
#include "constants.h"
#include "client.h"

typedef struct Layout Layout;
typedef void (*LayoutHandler)(Monitor *);

/// @brief Represents a layout on a Monitor
struct Layout {
	/// Layout symbol thats displayed in the bar
	const char *symbol;
	/// Layout callback function
	LayoutHandler handler;
};

/// @brief Helper macro to detect geometrical intersections between two windows on a monitor
#define INTERSECT(x, y, w, h, m)                                         \
	(MAX(0, MIN((x) + (w), (m)->wx + (m)->ww) - MAX((x), (m)->wx)) * \
	 MAX(0, MIN((y) + (h), (m)->wy + (m)->wh) - MAX((y), (m)->wy)))

/// @brief Represents a physical monitor
struct Monitor {
	/// Layout symbol thats displayed in the bar
	char layout_symbol[MAX_LAYOUT_SYMBOL_LEN];
	/// Master width factor (how much space the master window takes up)
	float master_width_factor;
	/// Number of windows in the master area, default is 1
	int32_t num_master;
	/// Number of all monitors?
	int32_t num;
	/// Bar geometry
	int32_t bar_y;
	/// Screen geometry
	int32_t mx, my, mw, mh;
	/// Window area
	int32_t wx, wy, ww, wh;
	/// Selected tags
	uint32_t selected_tags;
	/// Selected layout
	uint32_t selected_layout;
	/// Tag set
	uint32_t tag_set[MAX_TAG_SETS];
	/// Bar is shown
	int32_t show_bar;
	/// Bar is on top 1 for true, 0 for false
	int32_t top_bar;
	/// Currently selected client
	Client *selected_client;
	/// A linked list of all the clients on this monitor
	Client *client_list;
	/// Stack of clients, used to preserve order
	Client *client_stack;
	/// Next monitor in the linked list of monitors
	Monitor *next;
	/// Xid for the bar window
	Window bar_win_id;
	/// Layouts
	const Layout *layouts[MAX_LAYOUTS];
};

/// @brief Constructs a single monitor
Monitor *Monitor__new(void);

/// @brief Destroys a monitor and frees all memory allocated to it.
///
/// @details Also unmaps the bar window and destroys it.
void Monitor__delete(Monitor *monitor);

/// @brief Checks if the layout is monocle mode
bool Monitor__is_layout_monocle(Monitor *monitor);

/// @brief Updates the layout symbol, then calls the layout's arrange function
/// for the given monitor.
void Monitor__arrange(Monitor *monitor);

/// @brief Get total number of clients on this monitor
int32_t Monitor__get_num_clients(Monitor *monitor);

/// @brief Update the status bar position for one monitor
void Monitor__updatebarpos(Monitor *monitor);

/// @brief Sets the layout to master stack for a monitor
void Monitor__layout_master_stack(Monitor *m);

/// @brief Sets the layout to monocle for a monitor
void Monitor__layout_monocle(Monitor *m);

#endif // SLACKER_H
