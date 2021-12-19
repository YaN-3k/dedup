#include "recdir.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_DIR_DEPTH 128

struct RECDIR_ {
    int fd;
    size_t size;
    DIR *dirs[MAX_DIR_DEPTH];
};

static int recdirpush(RECDIR recdir, int fd);
static int recdirpop(RECDIR recdir);
static DIR *recdirtop(RECDIR recdir);

int
recdirpush(RECDIR recdir, int fd)
{
    DIR *dir;

    assert(recdir->size < MAX_DIR_DEPTH);
    dir = fdopendir(fd);
    if (dir != NULL)
        recdir->dirs[recdir->size++] = dir;
    return dir != NULL;
}

int
recdirpop(RECDIR recdir)
{
    assert(recdir->size > 0);
    return closedir(recdir->dirs[--recdir->size]);
}

DIR *
recdirtop(RECDIR recdir)
{
    return recdir->dirs[recdir->size - 1];
}

RECDIR
recdiropen(const char *path)
{
    RECDIR recdir;

    recdir = malloc(sizeof(struct RECDIR_));
    memset(recdir, 0, sizeof(struct RECDIR_));
    recdir->fd = -1;

    recdir->dirs[recdir->size++] = opendir(path);
    if (errno != 0) {
        free(recdir);
        return NULL;
    }

    return recdir;
}

int
recdirclose(RECDIR recdir)
{
    if (recdir->fd != -1 && close(recdir->fd) != 0)
        return -1;
    free(recdir);
    return 0;
}

int
recdirread(RECDIR recdir)
{
    struct dirent *ent;
    DIR *top;
    int dir_fd;
    int sub_fd;

    if (recdir->fd != -1 && close(recdir->fd) != 0)
        return -1;

    while (1) {
        top = recdirtop(recdir);
        dir_fd = dirfd(top);

        while ((ent = readdir(top)) != NULL &&
               (strcmp(ent->d_name, ".") == 0 ||
                strcmp(ent->d_name, "..") == 0));


        if (errno != 0 || ent == NULL) {
            errno = 0;
            if (recdirpop(recdir))
                return -1;
            if (recdir->size == 0)
                return -1;
            continue;
        }

        switch (ent->d_type) {
        case DT_DIR:
            if ((sub_fd = openat(dir_fd, ent->d_name, O_RDONLY)) == -1) {
                errno = 0;
                continue;
            }
            if (recdirpush(recdir, sub_fd) != 0) {
                errno = 0;
                continue;
            }
            break;
        case DT_REG:
            if ((recdir->fd = openat(dir_fd, ent->d_name, O_RDONLY)) == -1) {
                errno = 0;
                continue;
            }
            return recdir->fd;
        }
    }
}
