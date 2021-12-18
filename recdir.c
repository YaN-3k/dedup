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

#define MAX_DIR_DEPTH 1024

struct RECDIR_ {
    size_t size;
    DIR *dirs[MAX_DIR_DEPTH];
};

static int recdirpush(RECDIR recdir, int fd);
static int recdirpop(RECDIR recdir);
static DIR *recdirtop(RECDIR recdir);

int
recdirpush(RECDIR recdir, int fd)
{
    assert(recdir->size < MAX_DIR_DEPTH);
    recdir->dirs[recdir->size++] = fdopendir(fd);
    return errno;
}

int
recdirpop(RECDIR recdir)
{
    assert(recdir->size > 0);
    closedir(recdir->dirs[--recdir->size]);
    return errno != 0;
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

    recdir->dirs[recdir->size++] = opendir(path);
    if (errno != 0) {
        free(recdir);
        return NULL;
    }

    return recdir;
}

void
recdirclose(RECDIR recdir)
{
    free(recdir);
}

int
recdirread(RECDIR recdir)
{
    struct dirent *ent;
    DIR *top;
    int dir_fd;
    int fd;

    if (recdir->size == 0)
        return -1;

    do {
        top = recdirtop(recdir);
        dir_fd = dirfd(top);
        while ((ent = readdir(top)) != NULL &&
               (strcmp(ent->d_name, ".") == 0 ||
                strcmp(ent->d_name, "..") == 0));

        if (errno != 0)
            return -1;

        if (ent == NULL) {
            recdirpop(recdir);
            if (recdir->size == 0)
                return -1;
        } else if (ent->d_type == DT_DIR) {
            fd = openat(dir_fd, ent->d_name, O_RDONLY);
            if (errno != 0)
                return -1;
            recdirpush(recdir, fd);
            if (errno != 0)
                return -1;
        }
    } while (ent == NULL || ent->d_type != DT_REG);
    
    printf("%s\n", ent->d_name);
    return openat(dir_fd, ent->d_name, O_RDONLY);
}
