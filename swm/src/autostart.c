
// Slacker Headers
#include "autostart.h"
#include "utils.h"

Autostart Autostart__new(void)
{
	Autostart as = {
		.cmds = { 0 },
		.pids = { -1 },
		.size = 0,
	};
	return as;
}

void Autostart__add(Autostart *as, const char *cmd)
{
	if (!as || !cmd) {
		return;
	}

	if (as->size < MAX_AUTOSTART_COMMANDS + 1) {
		as->cmds[as->size++] = (char *)cmd;
		as->pids[as->size] = -1;
		as->size += 1;
	}
}

/// Function to execute shell commands and store their PIDS
void Autostart__exec(Autostart *as)
{
	if (!as) {
		return;
	}
	char buffer[MAX_AUTOSTART_COMMANDS] = { 0 };

	for (uint32_t i = 0; i < as->size; ++i) {
		if (as->cmds[i] != NULL) {
			strcat(buffer, as->cmds[i]);
		} else {
			if (buffer[0] != '\0') {
				if (system_exec(buffer, &as->pids[i]) < 0) {
					fprintf(stderr,
						"'%s' failed to start\n",
						buffer);
					as->pids[i] = -1;
				}

				// We need to reset the buffer for the next command to be loaded in.
				memset(buffer, 0, sizeof(buffer));
				buffer[0] = '\0';
			}
		}
	}
}

/// @brief Iterate through all the stored PIDS of start up programs and kill
/// them.
void Autostart__kill(Autostart *as)
{
	if (!as) {
		return;
	}

	for (uint32_t i = 0; i < as->size; ++i) {
		// The `as->size` variable contains null entries to seperate
		// commands. The pid array will hold no such null seperator values.
		// Basically ever odd index of the pid array will hold a real pid,
		// so if pid % 2 is 1, we have an odd index
		if ((as->pids[i] % 2 == 1) && as->pids[i] > 0) {
			kill(as->pids[i], SIGTERM);
			waitpid(as->pids[i], NULL, WNOHANG);
		}
	}
	memset(as->cmds, 0, sizeof(as->cmds));
	memset(as->pids, 0, sizeof(as->pids));
	as->size = 0;
}
