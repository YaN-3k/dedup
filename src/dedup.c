#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <unistd.h>
#include <regex.h>
#include <openssl/sha.h>

#include "recdir.h"
#include "sha256.h"
#include "util.h"

void
compreg(const char *regstr, regex_t *reg)
{
    size_t error_len;
    char *error_msg;
    int errcode;

    if ((errcode = regcomp(reg, regstr, 0)) != 0) {
        error_len = regerror(errcode, reg, NULL, 0);
        error_msg = alloca(error_len);
        regerror(errcode, reg, error_msg, error_len);
        die("ERROR: Could not compile regex %s: %s", regstr, error_msg);
    }
}

int
main(int argc, char *argv[])
{
    unsigned char hash[SHA256_LENGTH];
    char hash_cstr[SHA256_CSTR_LENGTH];
    int verbose = 2;
    RECDIR *recdir;
    regex_t exclude_reg = {0};
    char *dpath, *fpath;
    FILE *fp;

    if (argc < 3 || argc > 4)
        die("usage: %s [dir] [database] [exclude-pattern]", argv[0]);

    if (argc > 3)
        compreg(argv[3], &exclude_reg);

    if ((dpath = realpath(argv[1], NULL)) == NULL)
        die("ERROR: Could not resolve path %s:", argv[1]);
    errno = 0;

    recdir = recdiropen(dpath, argc > 3 ? &exclude_reg : NULL, verbose);
    free(dpath);

    if (recdir == NULL)
        die("ERROR: Could not open directory %s:", argv[1]);

    while ((fpath = recdirread(recdir)) != NULL) {
        if ((fp = fopen(fpath, "r")) == NULL)
            continue;

        if (sha256(fp, hash) == 0) {
            fclose(fp);
            if (verbose > 1)
                printf("%-50s -- [EMPTY]\n", fpath);
            continue;
        }
        fclose(fp);

        memset(hash_cstr, 0, sizeof(hash_cstr));
        hash2cstr(hash, hash_cstr);
        if (verbose > 1)
            printf("%-50s -- %s\n", fpath, hash_cstr);
    }

    if (errno != 0)
        die("ERROR: Could not read directory:");

    recdirclose(recdir);
    regfree(&exclude_reg);

    return 0;
}
