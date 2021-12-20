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

static void compreg(const char *regstr, regex_t *reg);

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
    char data[1024];
    int nbytes;
    Args args = {0};
    regex_t exclude_reg;
    RECDIR *recdir;
    char *fpath;
    FILE *fp;

    parseargs(argc, argv, &args);

    if (args.exclude_reg)
        compreg(args.exclude_reg, &exclude_reg);

    //if (args.realpath && (args.path = realpath(args.path, NULL)) == NULL)
        //die("ERROR: Could not resolve path %s:", args.path);
    //errno = 0;

    recdir = recdiropen(args.path, args.exclude_reg ? &exclude_reg : NULL, args.verbose);

    if (recdir == NULL)
        die("ERROR: Could not open directory %s:", argv[1]);

    while ((fpath = recdirread(recdir)) != NULL) {
        if ((fp = fopen(fpath, "r")) == NULL)
            continue;

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
        die("ERROR: Could not read directory:");

    recdirclose(recdir);
    if (args.exclude_reg) regfree(&exclude_reg);
    if (args.realpath) free((char *)args.path);

    return 0;
}
