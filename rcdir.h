/* #include "args.h" */

typedef struct RCDIR RCDIR;

RCDIR *rcdiropen(const Arg *args);
void rcdirclose(RCDIR *rcdir);
char *rcdirread(RCDIR *rcdir);
