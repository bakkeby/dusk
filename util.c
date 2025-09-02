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

	/* calloc allocates memory for an array of n elements and initializes all bits to zero */
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

void
setenabled(const uint64_t functionality, int enabled)
{
	if (enabled) {
		enablefunc(functionality);
	} else {
		disablefunc(functionality);
	}
}

void
freestrdup(char **dest, const char *src)
{
	if (dest == NULL)
		return;

	free(*dest);

	*dest = src ? strdup(src) : NULL;
}

int
freesprintf(char **dest, const char *format, ...)
{
	va_list args;
	int result;
	size_t size;

	free(*dest);

	va_start(args, format);
	size = vsnprintf(NULL, 0, format, args);
	va_end(args);

	*dest = ecalloc(size + 1, sizeof(char));

	va_start(args, format);
	result = vsnprintf(*dest, size + 1, format, args);
	va_end(args);

	return result;
}

int startswith(const char *needle, const char *haystack)
{
	return !strncmp(haystack, needle, strlen(needle));
}

#ifdef __linux__
/*
 * Copy string src to buffer dst of size dsize.  At most dsize-1
 * chars will be copied.  Always NUL terminates (unless dsize == 0).
 * Returns strlen(src); if retval >= dsize, truncation occurred.
 *
 * From:
 * https://github.com/freebsd/freebsd-src/blob/master/sys/libkern/strlcpy.c
 */
size_t
strlcpy(char * __restrict dst, const char * __restrict src, size_t dsize)
{
	const char *osrc = src;
	size_t nleft = dsize;

	/* Copy as many bytes as will fit. */
	if (nleft != 0) {
		while (--nleft != 0) {
			if ((*dst++ = *src++) == '\0')
				break;
		}
	}

	/* Not enough room in dst, add NUL and traverse rest of src. */
	if (nleft == 0) {
		if (dsize != 0)
			*dst = '\0';		/* NUL-terminate dst */
		while (*src++)
			;
	}

	return (src - osrc - 1);	/* count does not include NUL */
}

/*
 * Appends src to string dst of size siz (unlike strncat, siz is the
 * full size of dst, not space left).  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz <= strlen(dst)).
 * Returns strlen(src) + MIN(siz, strlen(initial dst)).
 * If retval >= siz, truncation occurred.
 */
size_t
strlcat(char *dst, const char *src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;
	size_t dlen;

	/* Find the end of dst and adjust bytes left but don't go past end */
	while (n-- != 0 && *d != '\0')
		d++;
	dlen = d - dst;
	n = siz - dlen;

	if (n == 0)
		return(dlen + strlen(s));
	while (*s != '\0') {
		if (n != 1) {
			*d++ = *s;
			n--;
		}
		s++;
	}
	*d = '\0';

	return(dlen + (s - src));   /* count does not include NUL */
}
#endif /* __linux__ */
