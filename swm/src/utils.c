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

void autostart(pid_t *autostart_pids, size_t *autostart_len)
{
	const char *const *p;
	size_t i = 0;

	// count the number of commands
	for (p = G_AUTOSTART_COMMANDS; *p; autostart_len++, p++) {
		while (*++p) {
			;
		}
	}

	// FIXME: Switch to ecalloc
	do {
		autostart_pids = malloc(*autostart_len * sizeof(pid_t));
	} while (autostart_pids == NULL);

	for (p = G_AUTOSTART_COMMANDS; *p; i++, p++) {
		if ((autostart_pids[i] == fork()) == 0) {
			setsid();
			execvp(*p, (char *const *)p);
			fprintf(stderr, "slacker: execvp %s", *p);
			perror(" failed");
			_exit(EXIT_FAILURE);
		}
		// skip arguments
		while (*++p) {
			;
		}
	}
}

void autokill(pid_t *autostart_pids, size_t *autostart_len)
{
	for (size_t i = 0; i < *autostart_len; i++) {
		if (autostart_pids[i] > 0) {
			kill(autostart_pids[i], SIGTERM);
			waitpid(autostart_pids[i], NULL, 0);
		}
	}
}
