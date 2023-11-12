#ifndef SWM_AUTOSTART_H
#define SWM_AUTOSTART_H

// Standard Libraries
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Slacker Headers
#include "constants.h"
#include "utils.h"

typedef struct Autostart Autostart;

/// Holds the auto start shell commands
struct Autostart {
	// Enter commands here, NULL terminated
	char *cmds[MAX_AUTOSTART_COMMANDS];
	// process id array
	pid_t pids[MAX_AUTOSTART_COMMANDS];
	// Holds the number of commands added
	uint32_t size;
};

/// @brief Initialize the Autostart struct
Autostart Autostart__new(void);

/// @brief A simple interface which allows the user to add shell commands
/// that will be executed when the window manager starts up.
void Autostart__add(Autostart *as, const char *cmd);

/// @brief Function to execute shell commands and store their PIDS
void Autostart__exec(Autostart *as);

/// @brief Iterate through all the stored PIDS of start up programs
/// and kills them.
void Autostart__kill(Autostart *as);

#endif
