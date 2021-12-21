#include "util.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ALLOC(alloc_fn, ...)          \
    void *p;                          \
    if (!(p = alloc_fn(__VA_ARGS__))) \
        die(#alloc_fn ":");           \
    return p;                         \

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

void *emalloc(size_t size) { ALLOC(malloc, size); }
void *erealloc(void *ptr, size_t new_size) { ALLOC(realloc, ptr, new_size); }
void *ecalloc(size_t nmemb, size_t size) { ALLOC(calloc, nmemb, size); }
