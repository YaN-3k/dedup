#ifndef ARGS_H__
#define ARGS_H__

#include <regex.h>

enum {
    VERBOSE_SILENT = 0,
    VERBOSE_STACK  = 1 << 0,
    VERBOSE_HASH   = 1 << 1
};

typedef struct {
    regex_t *exclude_reg;
    const char *path;
    const char *db;
    size_t maxdepth;
    size_t mindepth;
    size_t nbytes;
    int verbose;
    int realpath;
} Args;

void argsparse(int argc, char *argv[], Args *args);
void argsfree(Args *args);

#endif
