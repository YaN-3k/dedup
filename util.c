#include "libc.h"
#include "util.h"

void
die(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	if(fmt[0] && fmt[strlen(fmt) - 1] == ':') {
		fputc(' ', stderr);
		perror(NULL);
	} else fputc('\n', stderr);
	exit(1);
}

void *
xmalloc(size_t size)
{
	void *p;

	if(!(p = malloc(size))) die("malloc:");
	return p;
}

void *
xrealloc(void *ptr, size_t size)
{
	void *p;

	if(!(p = realloc(ptr, size))) die("realloc:");
	return p;
}

void *
xcalloc(size_t nmemb, size_t size)
{
	void *p;

	if(!(p = calloc(nmemb, size))) die("calloc:");
	return p;
}

char *
xstrdup(const char *s)
{
	if(!(s = strdup(s))) die("strdup:");
	return (char *)s;
}
