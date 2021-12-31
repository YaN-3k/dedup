#include <dirent.h>
#include <sys/stat.h>
#include "libc.h"
#include "arg.h"
#include "rcdir.h"

#define FRM_ALLOC_SZ 16
#define RCDIR_LOG(fmt, op, path) if(fmt) printf(fmt, op, path)

typedef struct {
	const char *path;
	DIR *dir;
} FRM;

struct RCDIR {
	FRM *frms;
	size_t frmsc;
	regex_t *exclude;
	const char *path;
	const char *fmt;
	size_t maxdepth;
	size_t mindepth;
	size_t depth;
};

static FRM *rcdirtop(RCDIR *rcdir);
static int rcdirpush(RCDIR *rcdir, const char *path);
static int rcdirpop(RCDIR *rcdir);
static char *makepath(const char *p1, const char *p2);

FRM *
rcdirtop(RCDIR *rcdir)
{
	return &rcdir->frms[rcdir->depth - 1];
}

char *
makepath(const char *p1, const char *p2)
{
	const char *fmt;
	char *outpath;
	int p1sz;

	p1sz = strlen(p1);
	fmt = p1[p1sz - 1] != '/' ? "%s/%s" : "%s%s";
	outpath = xmalloc(p1sz + strlen(p2) + 2);
	sprintf(outpath, fmt, p1, p2);
	return outpath;
}

int
rcdirpush(RCDIR *rcdir, const char *path)
{
	FRM *top;
	DIR *dir;

	if(!(dir = opendir(path)))
		return -1;
	if(++rcdir->depth >= rcdir->frmsc) {
		rcdir->frmsc += FRM_ALLOC_SZ;
		rcdir->frms = xrealloc(rcdir->frms, sizeof(FRM) * rcdir->frmsc);
	}
	top = rcdirtop(rcdir);
	top->dir = dir;
	top->path = xstrdup(path);
	return 0;
}

int
rcdirpop(RCDIR *rcdir)
{
	FRM *top;
	int excode;

	assert(rcdir->depth > 0);
	top = rcdirtop(rcdir);
	free((char *)top->path);
	excode = closedir(top->dir);
	if(--rcdir->depth < rcdir->frmsc - FRM_ALLOC_SZ) {
		rcdir->frmsc -= FRM_ALLOC_SZ;
		rcdir->frms = xrealloc(rcdir->frms, sizeof(FRM) * rcdir->frmsc);
	}
	return excode;
}

RCDIR *
rcdiropen(const Arg *arg)
{
	RCDIR *rcdir;

	rcdir = xcalloc(1, sizeof *rcdir);
	if(rcdirpush(rcdir, arg->path) != 0) {
		free(rcdir);
		return NULL;
	}
	if(arg->verbose & VERBOSE_STACK) {
		if(arg->verbose & VERBOSE_HASH) rcdir->fmt = "%-64s  %s\n";
		else rcdir->fmt = "%-10s  %s\n";
	}
	rcdir->maxdepth = arg->maxdepth;
	rcdir->mindepth = arg->mindepth;
	rcdir->exclude = arg->exclude;
	RCDIR_LOG(rcdir->fmt, "OPEN", arg->path);
	return rcdir;
}

void
rcdirclose(RCDIR *rcdir)
{
	while(rcdir->depth) {
		RCDIR_LOG(rcdir->fmt, "CLOSE", rcdirtop(rcdir)->path);
		rcdirpop(rcdir);
	}
	free((char *)rcdir->path);
	free(rcdir->frms);
	free(rcdir);
}

char *
rcdirread(RCDIR *rcdir)
{
	struct dirent *ent;
	struct stat st;
	FRM *top;

	errno = 0;
	while(1) {
		top = rcdirtop(rcdir);
		while((ent = readdir(top->dir)) != NULL &&
		      (strcmp(ent->d_name, ".") == 0 ||
		      strcmp(ent->d_name, "..") == 0));
		if(errno != 0) {
			perror(top->path);
			errno = 0;
			if(rcdirpop(rcdir) < 0 || rcdir->depth == 0)
				return NULL;
			continue;
		}
		if(ent == NULL) {
			RCDIR_LOG(rcdir->fmt, "CLOSE", top->path);
			if(rcdirpop(rcdir) < 0 || rcdir->depth == 0)
				return NULL;
			continue;
		}
		free((char *)rcdir->path);
		rcdir->path = makepath(top->path, ent->d_name);
		if(ent->d_type == DT_LNK || ent->d_type == DT_UNKNOWN) {
			if(stat(rcdir->path, &st) < 0) {
				perror(rcdir->path);
				errno = 0;
				continue;
			}
			ent->d_type = st.st_mode >> 12;
		}
		switch(ent->d_type) {
		case DT_DIR:
			if(rcdir->maxdepth <= rcdir->depth) continue;
			if(rcdir->exclude != NULL &&
			   regexec(rcdir->exclude, rcdir->path, 0, NULL, 0) == 0) {
				RCDIR_LOG(rcdir->fmt, "exclude", rcdir->path);
				continue;
			}
			if(rcdirpush(rcdir, rcdir->path) != 0) {
				perror(rcdir->path);
				errno = 0;
				continue;
			}
			RCDIR_LOG(rcdir->fmt, "OPEN", rcdirtop(rcdir)->path);
			break;
		case DT_REG:
			if(rcdir->mindepth > rcdir->depth) continue;
			return xstrdup(rcdir->path);
		default:
			RCDIR_LOG(rcdir->fmt, "SKIP", rcdir->path);
		}
	}
}
