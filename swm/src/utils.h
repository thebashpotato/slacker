#ifndef SWM_UTILS_H
#define SWM_UTILS_H

// Standard Libraries
#include <stdlib.h>

// Slacker Libraries
#include "config.h"
#include "constants.h"

/////////////////////////////////////////////////
///			Generic Helper macros
/////////////////////////////////////////////////

#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define BETWEEN(X, A, B) ((A) <= (X) && (X) <= (B))
#define LENGTH(X) (sizeof X / sizeof X[0])

////////////////////////////////////////////////////
/// 			Helper client macros
////////////////////////////////////////////////////

/// @brief Check if a client is visible on a monitor
#define ISVISIBLE(client) \
	((client->tags & client->mon->tag_set[client->mon->selected_tags]))

/// @brief Get the Width of a client + 2 * the border width
#define WIDTH(client) ((client)->w + 2 * (client)->bw + G_GAP_PIXEL)

/// @brief Get the Height of a client + 2 * the border width
#define HEIGHT(client) ((client)->h + 2 * (client)->bw + G_GAP_PIXEL)

/// @brief Compile time check to ensure that the number of tags is less than 32
struct NumTagsCompileTimeCheck {
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

/// @brief Execute a shell command in a child process.
///
/// @details Doesn't block the main process, and returns the status of the command.
///
/// @param cmdstring The command to execute.
/// @param pid Reference to the proccess id of the command.
/// The called can use this to keep kill the pid later on if they want.
///
/// @return The status of the command, 0 is good, anything < 0 is bad
int32_t system_exec(const char *cmdstring, pid_t *pid);

#endif // SLACKER_UTILS_H
