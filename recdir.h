#ifndef RECDIR_H__
#define RECDIR_H__

#include <regex.h>

typedef struct RECDIR RECDIR;

RECDIR *recdiropen(const char *path, regex_t *exclude_reg, size_t maxdepth,
                   size_t mindepth, int verbose);
void recdirclose(RECDIR *recdir);
char *recdirread(RECDIR *recdir);

#endif
