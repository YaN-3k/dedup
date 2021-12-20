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
#include "args.h"

#define FRAMES_REALLOC_SIZE 16
#define FRAMES_REALLOC(frames) \
        erealloc(frames, sizeof(RECDIR_FRAME) * recdir->max_size);

#define RECDIR_LOG(t, f) if (recdir->fmt) printf(recdir->fmt, t, f);

/* TODO: summary for recdir */
typedef struct {
    int excluded;
    int skipped;
    int opened;
} RECDIR_COUNT;

typedef struct {
    char *path;
    DIR *dir;
} RECDIR_FRAME;

struct RECDIR_ {
    regex_t *exclude_reg;
    const char *fmt;
    char *fpath;
    size_t size;
    size_t max_size;
    RECDIR_FRAME *frames;
};

static RECDIR_FRAME *recdirtop(RECDIR *recdir);
static int recdirpush(RECDIR *recdir, const char *path);
static int recdirpop(RECDIR *recdir);
static char *join_path(const char *p1, const char *p2, char *joined);

char *
join_path(const char *p1, const char *p2, char *joined)
{
    int p1len = strlen(p1);
    const char *fmt;

    if (joined == NULL) joined = emalloc(p1len + strlen(p2) + 2);
    fmt = p1[p1len - 1] != '/' ? "%s/%s" : "%s%s";
    sprintf(joined, fmt, p1, p2);
    return joined;
}

int
recdirpush(RECDIR *recdir, const char *path)
{
    RECDIR_FRAME *top;
    char *dpath;
    DIR *dir;

    dpath = join_path(recdirtop(recdir)->path, path, NULL);
    dir = opendir(dpath);
    if (dir == NULL) {
        free(dpath);
    } else {
        if (++recdir->size == recdir->max_size) {
            recdir->max_size += FRAMES_REALLOC_SIZE;
            recdir->frames = FRAMES_REALLOC(recdir->frames);
        }
        top = recdirtop(recdir);
        top->dir = dir;
        top->path = dpath;
    }
    return dir == NULL;
}

int
recdirpop(RECDIR *recdir)
{
    RECDIR_FRAME *top;
    int excode;

    assert(recdir->size > 0);
    top = recdirtop(recdir);
    free(top->path);
    excode = closedir(top->dir);
    if (--recdir->size < recdir->max_size - FRAMES_REALLOC_SIZE) {
        recdir->max_size -= FRAMES_REALLOC_SIZE;
        recdir->frames = FRAMES_REALLOC(recdir->frames);
    }
    return excode;
}

RECDIR_FRAME *
recdirtop(RECDIR *recdir)
{
    return &recdir->frames[recdir->size - 1];
}

RECDIR *
recdiropen(const char *path, regex_t *exclude_reg, int verbose)
{
    RECDIR_FRAME *top;
    RECDIR *recdir;
    DIR *dir;

    recdir = emalloc(sizeof(struct RECDIR_));
    memset(recdir, 0, sizeof(struct RECDIR_));

    recdir->max_size = FRAMES_REALLOC_SIZE;
    recdir->frames = FRAMES_REALLOC(recdir->frames);
    recdir->exclude_reg = exclude_reg;

    if (verbose & VERBOSE_STACK)
        recdir->fmt = (verbose & VERBOSE_HASH) ? "%-64s %s\n" : "%-10s %s\n";

    if ((dir = opendir(path)) == NULL) {
        free(recdir);
        return NULL;
    }

    recdir->size++;
    top = recdirtop(recdir);
    top->dir = dir;
    top->path = strdup(path);

    RECDIR_LOG("OPEN", top->path);

    return recdir;
}

void
recdirclose(RECDIR *recdir)
{
    free(recdir->frames);
    free(recdir);
}

char *
recdirread(RECDIR *recdir)
{
    struct dirent *ent;
    RECDIR_FRAME *top;
    char *path;

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
            RECDIR_LOG("CLOSE", top->path);
            if (recdirpop(recdir) || recdir->size == 0)
                return NULL;
            continue;
        }

        path = alloca(strlen(top->path) + strlen(ent->d_name) + 2);
        join_path(top->path, ent->d_name, path);

        if (access(path, R_OK) != 0) {
            RECDIR_LOG("SKIP [P]", path);
            errno = 0;
            continue;
        }

        switch (ent->d_type) {
        case DT_DIR:
            if (recdir->exclude_reg != NULL &&
                regexec(recdir->exclude_reg, path, 0, NULL, 0) == 0) {
                RECDIR_LOG("EXCLUDE", path);
                continue;
            }
            if (recdirpush(recdir, ent->d_name) != 0) {
                errno = 0;
                RECDIR_LOG("SKIP [E]", path);
                assert(0 && "UNREACHABLE?");
                continue;
            }
            RECDIR_LOG("OPEN", recdirtop(recdir)->path);
            break;
        case DT_REG:
            recdir->fpath = join_path(top->path, ent->d_name, NULL);
            return recdir->fpath;
        default:
            /* TODO: handle for symlinks */
            RECDIR_LOG("SKIP [T]", path);
        }
    }
}
