#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <regex.h>
#include <openssl/sha.h>

#include "recdir.h"
#include "sha256.h"
#include "args.h"
#include "util.h"

int
main(int argc, char *argv[])
{
    RECDIR *recdir;
    Args args = {0};
    unsigned char hash[SHA256_LENGTH];
    char hash_cstr[SHA256_CSTR_LENGTH];
    char *fpath;

    argsparse(argc, argv, &args);

    recdir = recdiropen(args.path, args.exclude_reg, args.verbose);

    /* TODO: fix args memory leak */
    if (recdir == NULL) {
        argsfree(&args);
        die(":");
    }

    while ((fpath = recdirread(recdir)) != NULL) {
        if (sha256(hash, fpath, -1) < 0)
            continue;

        if (args.verbose & VERBOSE_HASH) {
            hash2cstr(hash, hash_cstr);
            printf("%-64s %s\n", hash_cstr, fpath);
        }
    }


    if (errno != 0)
        die("Could not read directory:");

    recdirclose(recdir);
    argsfree(&args);

    //printf("[ opened: %ld | excluded: 20 | skipped: 10 | readed: 20 ]\n", fcount);

    return 0;
}
