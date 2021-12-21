#ifndef ARGS_H__
#define ARGS_H__

#include <regex.h>

enum {
    VERBOSE_STACK = 1 << 0,
    VERBOSE_HASH  = 1 << 1,
};

typedef struct {
    regex_t *exclude_reg;
    const char *path;
    const char *db;
    int maxdepth;
    int mindepth;
    int nbytes;
    int realpath;
    int verbose;
} Args;

void argsparse(int argc, char *argv[], Args *args);
void argsfree(Args *args);

#endif
