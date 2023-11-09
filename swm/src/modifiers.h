#ifndef SWM_MODIFIERS_H
#define SWM_MODIFIERS_H

#include "common.h"
#include "monitor.h"

/// @brief Kills the currently selected client.
///
/// @param `arg` Unused
void Swm__kill_client(const Arg *arg);

/// @brief Increments master window in the stacker area by (+- n)
void Swm__increment_n_master(const Arg *arg);

/// @brief Confusing code. I think this function is used to move a window via the mouse.
///
/// @details TODO: Refactor this function, it is confusing and hard to read.
void Swm__move_with_mouse(const Arg *arg);

/// @brief Shutdown the window manager
void Swm__quit(const Arg *arg);

/// @brief Adds support for resizing a client with the mouse.
///
/// @details Does not support resizing fullscreen windows, because this is a tiling window manager.
///
/// @param `arg` The argument to pass to the function
void Swm__resize_client_with_mouse(const Arg *arg);

/// @brief Sets the layout of a monitor.
///
/// @param `arg` The argument to pass to the function, the void * is a Layout *
void Swm__setlayout(const Arg *arg);

/// @brief arg > 1.0 will set mfact absolutely
///
/// @param `arg` The argument to pass to the function
void Swm__setmfact(const Arg *arg);

/// @brief Helper function to spawn shell commands
///
/// @param `arg` The argument Union to pass to the function, the void * is a char *
void Swm__spawn(const Arg *arg);

/// @brief Uses the arg.ui field as a bitmask to toggle the tag of a client.
///
/// @param `arg` The argument Union to pass to the function, uses the ui field as a bitmask
void Swm__tag(const Arg *arg);

/// @brief Not sure what this does, debug
void Swm__tagmon(const Arg *arg);

/// @brief Toggle the bar on the selected monitor.
///
/// @param `arg` Unused argument
void Swm__togglebar(const Arg *arg);

/// @brief Toggle the floating state of a client.
///
/// @param `arg` Unused
void Swm__togglefloating(const Arg *arg);

/// @brief Called when a tag is clicked, or a keybinding is pressed to change to that tag view.
///
/// @param `arg` Unused
void Swm__toggletag(const Arg *arg);

/// @brief Change the view to a new tag, or monitor.
///
/// @param `arg` Unused
void Swm__toggleview(const Arg *arg);

/// @brief Changes view to a new tag/monitor
///
/// @param `arg` Uses the ui field as a bitmask to change to the proper tag.
void Swm__view(const Arg *arg);

/// @brief TODO: Document
void Swm__zoom(const Arg *arg);

/// @brief Focuses on the next client in the stack.
void Swm__focus_stack(const Arg *arg);

/// @brief When a user changed to a different monitor, this function is called.
void Swm__focus_monitor(const Arg *arg);

#endif
