#ifndef SWM_EVENTS_H
#define SWM_EVENTS_H

/// X11 Event handler functions

// X11 Libraries
#include <X11/Xlib.h>

/// @brief Event Loop
void Swm__event_loop(XEvent *event);

/// @brief Handles X11 ButtonPress events
///
/// @details The X server reports KeyPress or KeyRelease events to clients wanting information
/// about keys that logically change state. Note that these events are generated for all keys,
/// even those mapped to modifier bits. The X server reports ButtonPress or ButtonRelease events
/// to clients wanting information about buttons that logically change state.
///
/// https://tronche.com/gui/x/xlib/events/keyboard-pointer/keyboard-pointer.html
void Swm__event_button_press(XEvent *event);

/// @brief Handles X11 ClientMessage events
///
/// @details The X server generates ClientMessage events only when a client calls the function XSendEvent().
///
/// https://tronche.com/gui/x/xlib/events/client-communication/client-message.html
void Swm__event_client_message(XEvent *event);

/// @brief Handles X11 ConfigureNotify events
///
/// @details The X server can report ConfigureRequest events to clients wanting information about when a different client
/// initiates a configure window request on any child of a specified window. The configure window request attempts to
/// reconfigure a window's size, position, border, and stacking order.
///
/// https://tronche.com/gui/x/xlib/events/structure-control/configure.html
void Swm__event_configure_request(XEvent *event);

/// @brief ConfigureNotify event handler
///
/// The X server can report ConfigureNotify events to clients wanting information about actual changes to a window's state,
/// such as size, position, border, and stacking order. The X server generates this event type
/// whenever one of the following configure window requests made by a client application actually completes:
///
/// https://tronche.com/gui/x/xlib/events/window-state-change/configure.html
void Swm__event_configure_notify(XEvent *event);

/// @brief Handles X11 DestroyNotify events
///
/// @details The X server can report DestroyNotify events to clients wanting information about which windows are destroyed.
/// The X server generates this event whenever a client application destroys a window by calling XDestroyWindow() or XDestroySubwindows().
///
/// https://tronche.com/gui/x/xlib/events/window-state-change/destroy.html
void Swm__event_destroy_notify(XEvent *event);

/// @brief Handles X11 EnterNotify events
///
/// @details  If a pointer motion or a window hierarchy change causes the pointer to be in a different window than before,
/// the X server reports EnterNotify or LeaveNotify events to clients who have selected for these events.
///
/// https://tronche.com/gui/x/xlib/events/window-entry-exit/
void Swm__event_enter_notify(XEvent *event);

/// @brief Handles X11 Expose events
///
/// @details The X server can report Expose events to clients wanting information about when the
/// contents of window regions have been lost.
///
/// https://tronche.com/gui/x/xlib/events/exposure/expose.html
void Swm__event_expose(XEvent *event);

/// @brief Handles X11 FocusIn events
/// NOTE: There are some broken focus acquiring clients needing extra handling
///
/// @details The X server can report FocusIn or FocusOut events to clients wanting information about when the input focus
/// changes. The keyboard is always attached to some window (typically, the root window or a top-level window),
/// which is called the focus window. The focus window and the position of the pointer determine the window that receives
/// keyboard input. Clients may need to know when the input focus changes to control highlighting of areas on the screen.
///
/// https://tronche.com/gui/x/xlib/events/input-focus/
void Swm__event_focusin(XEvent *event);

/// @brief Handles X11 KeyPress events
///
/// @details The X server reports KeyPress or KeyRelease events to clients wanting information about keys that logically change state.
/// Note that these events are generated for all keys, even those mapped to modifier bits.
/// The X server reports ButtonPress or ButtonRelease events to clients wanting information about buttons that logically
/// change state.
///
/// https://tronche.com/gui/x/xlib/events/keyboard-pointer/keyboard-pointer.html
void Swm__event_keypress(XEvent *event);

/// @brief Handles X11 MappingNotify events
///
/// @details The X server reports MappingNotify events to all clients. There is no mechanism to express disinterest in this event.
/// The X server generates this event type whenever a client application successfully calls:
///
/// https://tronche.com/gui/x/xlib/events/window-state-change/mapping.html
void Swm__event_mapping_notify(XEvent *event);

/// @brief Handles X11 MapRequest events
///
/// @details The X server can report MapRequest events to clients wanting information about a different client's
/// desire to map windows. A window is considered mapped when a map window request completes.
/// The X server generates this event whenever a different client initiates a map window request on an unmapped
/// window whose override_redirect member is set to False .
///
/// https://tronche.com/gui/x/xlib/events/structure-control/map.html
void Swm__event_map_request(XEvent *event);

/// @brief Handles X11 MotionNotify events
///
/// @details The X server reports MotionNotify events to clients wanting information about when the pointer logically moves.
/// The X server generates this event whenever the pointer is moved and the pointer motion begins and ends in the window.
/// The granularity of MotionNotify events is not guaranteed, but a client that selects this event type is guaranteed to
/// receive at least one event when the pointer moves and then rests.
///
/// https://tronche.com/gui/x/xlib/events/keyboard-pointer/keyboard-pointer.html
void Swm__event_motion_notify(XEvent *event);

/// @brief Handles X11 PropertyNotify events
///
/// @details The X server can report PropertyNotify events to clients wanting information about property changes for a
/// specified window.
///
/// https://tronche.com/gui/x/xlib/events/client-communication/property.html
void Swm__event_property_notify(XEvent *event);

/// @brief Handles X11 UnmapNotify events
///
/// @details The X server can report UnmapNotify events to clients wanting information about which windows are unmapped.
/// The X server generates this event type whenever a client application changes the window's state from mapped to unmapped.
///
/// https://tronche.com/gui/x/xlib/events/window-state-change/unmap.html
void Swm__event_unmap_notify(XEvent *event);

#endif
