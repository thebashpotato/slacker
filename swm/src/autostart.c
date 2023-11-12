
// Slacker Headers
#include "autostart.h"

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
		as->size += 1;
		as->cmds[as->size++] = NULL;
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
					as->pids[i] = -1;
				}
				// reset command buffer
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
		if (as->pids[i] > 0) {
			kill(as->pids[i], SIGTERM);
			waitpid(as->pids[i], NULL, WNOHANG);
		}
	}
	memset(as->cmds, 0, sizeof(as->cmds));
	memset(as->pids, 0, sizeof(as->pids));
	as->size = 0;
}
