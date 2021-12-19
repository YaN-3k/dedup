#ifndef RECDIR_H__
#define RECDIR_H__

typedef struct RECDIR_ *RECDIR;

RECDIR recdiropen(const char *path);
int recdirclose(RECDIR recdir);
int recdirread(RECDIR recdir);

#endif
