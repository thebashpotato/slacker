// Standard Library
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Slacker Headers
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
