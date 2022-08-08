/* See LICENSE file for copyright and license details. */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

uint64_t settings = 0;

void
die(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	if (fmt[0] && fmt[strlen(fmt)-1] == ':') {
		fputc(' ', stderr);
		perror(NULL);
	} else {
		fputc('\n', stderr);
	}

	exit(1);
}

void *
ecalloc(size_t nmemb, size_t size)
{
	void *p;

	if (!(p = calloc(nmemb, size)))
		die("calloc:");
	return p;
}

int
enabled(const uint64_t functionality)
{
	return (settings & functionality) > 0;
}

int
disabled(const uint64_t functionality)
{
	return !(settings & functionality);
}

void
enablefunc(const uint64_t functionality)
{
	settings |= functionality;
}

void
disablefunc(const uint64_t functionality)
{
	settings &= ~functionality;
}

void
togglefunc(const uint64_t functionality)
{
	settings ^= functionality;
}
