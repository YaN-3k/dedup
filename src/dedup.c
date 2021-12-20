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
    size_t nbytes, fcount = 0;
    char data[1024];
    char *fpath;
    FILE *fp;

    argsparse(argc, argv, &args);

    recdir = recdiropen(args.path, args.exclude_reg, args.verbose);

    if (recdir == NULL)
        die("%s:", args.path);

    while ((fpath = recdirread(recdir)) != NULL) {
        if ((fp = fopen(fpath, "r")) == NULL)
            continue;

        fcount++;
        memset(data, 0, sizeof(data));
        nbytes = fread(data, 1, sizeof(data), fp);
        fclose(fp);

        sha256(hash, data, nbytes);

        if (args.verbose & VERBOSE_HASH) {
            memset(hash_cstr, 0, sizeof(hash_cstr));
            hash2cstr(hash, hash_cstr);
            printf("%-64s %s\n", hash_cstr, fpath);
        }
    }

    if (errno != 0)
        die("Could not read directory:");

    recdirclose(recdir);
    argsfree(&args);

    printf("[ opened: %ld | excluded: 20 | skipped: 10 | readed: 20 ]\n", fcount);

    return 0;
}
