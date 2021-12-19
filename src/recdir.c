#include "recdir.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dirent.h>
#include <fcntl.h>
#include <regex.h>
#include <sys/types.h>
#include <unistd.h>

#include "util.h"

#define MAX_DIR_DEPTH 128

typedef struct {
    char *path;
    DIR *dir;
} RECDIR_FRAME;

struct RECDIR_ {
    regex_t *exclude_reg;
    int show_progress;
    char *fpath;
    size_t size;
    RECDIR_FRAME frames[MAX_DIR_DEPTH];
};

static RECDIR_FRAME *recdirtop(RECDIR *recdir);
static int recdirpush(RECDIR *recdir, const char *path);
static int recdirpop(RECDIR *recdir);
static char *join_path(const char *p1, const char *p2, char *joined);

char *
join_path(const char *p1, const char *p2, char *joined)
{
    int p1_len = strlen(p1);
    const char *fmt;

    if (joined == NULL)
        joined = emalloc(p1_len + strlen(p2) + 2);
    fmt = p1[p1_len - 1] != '/' ? "%s/%s" : "%s%s";
    sprintf(joined, fmt, p1, p2);
    return joined;
}

int
recdirpush(RECDIR *recdir, const char *path)
{
    RECDIR_FRAME *top;
    char *dpath;
    DIR *dir;

    assert(recdir->size < MAX_DIR_DEPTH);
    dpath = join_path(recdirtop(recdir)->path, path, NULL);
    dir = opendir(dpath);
    if (dir != NULL) {
        recdir->size++;
        top = recdirtop(recdir);
        top->dir = dir;
        top->path = dpath;
    } else {
        free(dpath);
    }
    return dir == NULL;
}

int
recdirpop(RECDIR *recdir)
{
    RECDIR_FRAME *top;

    assert(recdir->size > 0);
    top = recdirtop(recdir);
    recdir->size--;
    free(top->path);
    return closedir(top->dir);
}

RECDIR_FRAME *
recdirtop(RECDIR *recdir)
{
    return &recdir->frames[recdir->size - 1];
}

RECDIR *
recdiropen(const char *path, regex_t *exclude_reg, int show_progress)
{
    RECDIR_FRAME *top;
    RECDIR *recdir;
    DIR *dir;

    recdir = malloc(sizeof(struct RECDIR_));
    memset(recdir, 0, sizeof(struct RECDIR_));
    recdir->exclude_reg = exclude_reg;
    recdir->show_progress = show_progress;

    if ((dir = opendir(path)) == NULL) {
        free(recdir);
        return NULL;
    }

    recdir->size++;
    top = recdirtop(recdir);
    top->dir = dir;
    top->path = strdup(path);

    if (recdir->show_progress)
        printf("OPEN       %s\n", recdirtop(recdir)->path);

    return recdir;
}

int
recdirclose(RECDIR *recdir)
{
    free(recdir);
    return 0;
}

char *
recdirread(RECDIR *recdir)
{
    struct dirent *ent;
    RECDIR_FRAME *top;
    char *dpath;

    if (recdir->fpath != NULL) {
        free(recdir->fpath);
        recdir->fpath = NULL;
    }

    while (1) {
        top = recdirtop(recdir);

        while ((ent = readdir(top->dir)) != NULL &&
               (strcmp(ent->d_name, ".") == 0 ||
                strcmp(ent->d_name, "..") == 0));


        if (errno != 0 || ent == NULL) {
            errno = 0;
            if (recdir->show_progress)
                printf("CLOSE      %s\n", top->path);
            if (recdirpop(recdir))
                return NULL;
            if (recdir->size == 0)
                return NULL;
            continue;
        }

        if (faccessat(dirfd(top->dir), ent->d_name, R_OK, AT_EACCESS) != 0) {
            dpath = alloca(strlen(top->path) + strlen(ent->d_name) + 2);
            join_path(top->path, ent->d_name, dpath);
            if (recdir->show_progress)
                printf("SKIP       %s\n", dpath);
            errno = 0;
            continue;
        }

        switch (ent->d_type) {
        case DT_DIR:
            if (recdir->exclude_reg != NULL) {
                dpath = alloca(strlen(top->path) + strlen(ent->d_name) + 2);
                join_path(top->path, ent->d_name, dpath);
                if (regexec(recdir->exclude_reg, dpath, 0, NULL, 0) == 0) {
                    if (recdir->show_progress)
                        printf("SKIP       %s\n", dpath);
                    continue;
                }   
            }
            if (recdirpush(recdir, ent->d_name) != 0) {
                errno = 0;
                if (recdir->show_progress)
                    printf("SKIP       %s\n", top->path);
                continue;
            }
            if (recdir->show_progress)
                printf("OPEN       %s\n", recdirtop(recdir)->path);
            break;
        case DT_REG:
            recdir->fpath = join_path(top->path, ent->d_name, NULL);
            return recdir->fpath;
        }
    }
}
