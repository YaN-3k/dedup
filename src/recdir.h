#ifndef RECDIR_H__
#define RECDIR_H__

#include <regex.h>

typedef struct RECDIR_ RECDIR;

/* TODO: mindepth and maxdepth parameter */
RECDIR *recdiropen(const char *path, regex_t *exclude_reg, int verbose);
void recdirclose(RECDIR *recdir);
char *recdirread(RECDIR *recdir);

#endif
