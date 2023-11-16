// Standard Library
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

// Slacker Headers
#include "config.h"
#include "error.h"
#include "utils.h"

const char CLIENT_WINDOW_BROKEN[] = "broken";

void die(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	if (fmt[0] && fmt[strlen(fmt) - 1] == ':') {
		fputc(' ', stderr);
		perror(NULL);
	} else {
		fputc('\n', stderr);
	}

	exit(EXIT_FAILURE);
}

void *ecalloc(size_t nmemb, size_t size)
{
	void *p;

	if (!(p = calloc(nmemb, size))) {
		die("calloc:");
	}
	return p;
}

void clean_environment(void)
{
	struct sigaction sa;
	// Do not transform children into zombies when they terminate
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_NOCLDSTOP | SA_NOCLDWAIT | SA_RESTART;
	sa.sa_handler = SIG_IGN;
	sigaction(SIGCHLD, &sa, NULL);

	// Clean up any zombies (inherited from .xinitrc etc) immediately
	while (waitpid(-1, NULL, WNOHANG) > 0) {
		;
	}
}

int32_t system_exec(const char *cmdstring, pid_t *pid)
{
	int32_t status = 0;

	if (cmdstring == NULL) {
		// always a command processor with UNIX
		return (1);
	}

	if ((*pid = fork()) < 0) {
		// probably out of processes
		status = -1;
	} else if (*pid == 0) {
		execl("/bin/sh", "sh", "-c", cmdstring, (char *)0);
		// execl error
		_exit(127);
	}

	return (status);
}
