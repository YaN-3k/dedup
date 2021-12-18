#ifndef RECDIR_H__
#define RECDIR_H__

typedef struct RECDIR_ *RECDIR;

RECDIR recdiropen(const char *path);
void recdirclose(RECDIR recdir);
int recdirread(RECDIR recdir);

#endif
