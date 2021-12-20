#ifndef ARGS_H__
#define ARGS_H__

enum {
    VERBOSE_STACK  = 1 << 0,
    VERBOSE_HASH   = 1 << 1,
};

typedef struct {
    const char *path;
    const char *db;
    const char *exclude_reg;
    int realpath;
    int verbose;
} Args;

void parseargs(int argc, char *argv[], Args *args);

#endif
