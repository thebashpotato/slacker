#ifndef SWM_UTILS_H
#define SWM_UTILS_H

// X11 Libraries
#include "X11/Xlib.h"

// Standard Libraries
#include <stdlib.h>

// Slacker Libraries
#include "swm.h"
#include "config.h"
#include "constants.h"

/////////////////////////////////////////////////
///			Generic Helper macros
/////////////////////////////////////////////////

#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define BETWEEN(X, A, B) ((A) <= (X) && (X) <= (B))
#define LENGTH(X) (sizeof X / sizeof X[0])

/// @brief Compile time check to ensure that the number of tags is less than 32
struct NumTags {
	uint32_t limitexceeded[LENGTH(G_TAGS) > MAX_SUPPORTED_TAGS ? -1 : 1];
};

/// Client text which displays in the bar when the client is broken
// TODO: Move this into the Client struct
extern const char CLIENT_WINDOW_BROKEN[];

/// @brief Print error message and exit the window manager
void die(const char *fmt, ...);

/// @brief Allocate memory and check for errors
void *ecalloc(size_t nmemb, size_t size);

/// @brief Clean up the environment inherited from the parent process.
///
/// @details Private function, only called once in `Slacker__init`
void clean_environment(void);

/// @brief Execute startup commands
///
/// @param autostart_pids: Array of process IDs, remembered for shutdown.
/// @param autostart_len: Length of the array of process IDs.
void autostart(pid_t *autostart_pids, size_t *autostart_len);

/// @brief Kill the autostart pids on system shutdown
void autokill(pid_t *autostart_pids, size_t *autostart_len);

#endif // SLACKER_UTILS_H
