/* #include "args.h" */

typedef struct RECDIR RECDIR;

RECDIR *recdiropen(const struct args *args);
void recdirclose(RECDIR *recdir);
char *recdirread(RECDIR *recdir);
