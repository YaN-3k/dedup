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
hash_cstr(unsigned char hash[], char buffer[])
{
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        sprintf(buffer + strlen(buffer), "%x", hash[i]);

}


void
compreg(const char *regstr, regex_t *reg)
{
    size_t error_len;
    char *error_buf;
    int errcode;

    if ((errcode = regcomp(reg, regstr, 0)) != 0) {
        error_len = regerror(errcode, reg, NULL, 0);
        error_buf = alloca(error_len);
        regerror(errcode, reg, error_buf, error_len);
        die("ERROR: Could not compile regex %s: %s", regstr, error_buf);
    }
}

int
main(int argc, char *argv[])
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    regex_t exclude_reg;
    char *dpath, *fpath;
    RECDIR *recdir;
    char buffer[64];
    FILE *fp;

    if (argc < 3 || argc > 4)
        die("usage: %s [dir] [database] [exclude-pattern]", argv[0]);

    if (argc > 3)
        compreg(argv[3], &exclude_reg);

    if ((dpath = realpath(argv[1], NULL)) == NULL)
        die("ERROR: Could not resolve path %s:", argv[1]);
    errno = 0;

    recdir = recdiropen(dpath, argc > 3 ? &exclude_reg : NULL, 1);
    free(dpath);

    if (recdir == NULL)
        die("ERROR: Could not open directory %s:", argv[1]);

    while ((fpath = recdirread(recdir)) != NULL) {
        fp = fopen(fpath, "r");
        if (sha256(fp, hash) == 0) {
            fclose(fp);
            continue;
        }
        memset(buffer, 0, sizeof(buffer));
        hash_cstr(hash, buffer);
        //for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
            //printf("%x", hash[i]);
        //putchar('\n');
        fclose(fp);
    }

    if (errno != 0)
        die("ERROR: Could not read directory:");

    recdirclose(recdir);
    regfree(&exclude_reg);

    return 0;
}
