#ifndef SWM_EVENTS_H
#define SWM_EVENTS_H

// X11 Libraries
#include <X11/Xlib.h>

// Standard Libraries

// Slacker Headers

/// @brief Handles X11 ButtonPress events
///
/// @param `event` The X11 event to handle
void event_buttonpress(XEvent *event);

/// @brief Handles X11 ClientMessage events
///
/// @param `event` The X11 event to handle
void event_clientmessage(XEvent *event);

/// @brief Handles X11 ConfigureNotify events
///
/// @param `event` The X11 event to handle
void event_configurerequest(XEvent *event);

/// @brief Handles X11 ConfigureNotify events
///
/// @param `event` The X11 event to handle
void event_configurenotify(XEvent *event);

/// @brief Handles X11 DestroyNotify events
///
/// @param `event` The X11 event to handle
void event_destroynotify(XEvent *event);

/// @brief Handles X11 EnterNotify events
///
/// @param `event` The X11 event to handle
void event_enternotify(XEvent *event);

/// @brief Handles X11 Expose events
///
/// @param `event` The X11 event to handle
void event_expose(XEvent *event);

/// @brief Handles X11 FocusIn events
///
/// @details There are some broken focus acquiring clients needing extra handling
/// @param `event` The X11 event to handle
void event_focusin(XEvent *event);

/// @brief Handles X11 KeyPress events
///
/// @param `event` The X11 event to handle
void event_keypress(XEvent *event);

/// @brief Handles X11 MappingNotify events
///
/// @param `event` The X11 event to handle
void event_mappingnotify(XEvent *event);

/// @brief Handles X11 MapRequest events
///
/// @param `event` The X11 event to handle
void event_maprequest(XEvent *event);

/// @brief Handles X11 MotionNotify events
///
/// @param `event` The X11 event to handle
void event_motionnotify(XEvent *event);

/// @brief Handles X11 PropertyNotify events
///
/// @param `event` The X11 event to handle
void event_propertynotify(XEvent *event);

/// @brief Handles X11 UnmapNotify events
///
/// @param `event` The X11 event to handle
void event_unmapnotify(XEvent *event);

#endif
