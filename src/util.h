#ifndef UTIL_H__
#define UTIL_H__

#include <stddef.h>

void die(const char *fmt, ...);
void *emalloc(size_t size);
void *erealloc(void *ptr, size_t new_size);
void *ecalloc(size_t nmemb, size_t size);

#endif
