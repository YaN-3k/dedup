#ifndef RECDIR_H__
#define RECDIR_H__

#include <regex.h>

#include "args.h"

typedef struct RECDIR RECDIR;

RECDIR *recdiropen(const struct args *args);
void recdirclose(RECDIR *recdir);
char *recdirread(RECDIR *recdir);

#endif
