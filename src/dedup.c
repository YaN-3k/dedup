#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "recdir.h"
#include "sha256.h"
#include "args.h"
#include "util.h"

int
main(int argc, char *argv[])
{
    unsigned char hash[SHA256_LENGTH];
    char hash_cstr[SHA256_CSTR_LENGTH];
    RECDIR *recdir;
    Args args = {0};
    char *fpath;

    argsparse(argc, argv, &args);

    recdir = recdiropen(
        args.path, args.exclude_reg,
        args.maxdepth, args.mindepth, args.verbose
    );

    if (recdir == NULL) {
        perror(args.path);
        argsfree(&args);
        exit(1);
    }

    while ((fpath = recdirread(recdir)) != NULL) {
        if (sha256(hash, fpath, args.nbytes) < 0) {
            if (errno != 0) {
                perror(fpath);
                errno = 0;
            }
            continue;
        }

        if (args.verbose & VERBOSE_HASH) {
            hash2cstr(hash, hash_cstr);
            printf("%-64s  %s\n", hash_cstr, fpath);
        }
    }

    recdirclose(recdir);
    argsfree(&args);

    if (errno != 0)
        die("Could not read directory:");

    return 0;
}
