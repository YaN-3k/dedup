#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

#include <signal.h>

#include "args.h"
#include "recdir.h"
#include "sha256.h"
#include "util.h"
#include "sql.h"

int terminated;

void
terminate()
{
    fputs("\nterminating...\n", stderr);
    terminated = 1;
}

int
main(int argc, char *argv[])
{
    unsigned char hash[SHA256_LENGTH];
    char hash_cstr[SHA256_CSTR_LENGTH];
    RECDIR *recdir = NULL;
    SQL *sql = NULL;
    Args args = {0};
    int excode = 0;
    char *fpath;
    FILE *fp;

    signal(SIGINT, terminate);

    argsparse(argc, argv, &args);

    if (args.db && sql_open(&sql, args.db) != 0) {
        fprintf(stderr, "sqlite3: %s\n", sql_errmsg(sql));
        errno = 0;
        excode = 1;
        goto cleanup;
    }

    recdir = recdiropen(
        args.path, args.exclude_reg,
        args.maxdepth, args.mindepth, args.verbose
    );

    if (recdir == NULL) {
        perror(args.path);
        errno = 0;
        excode = 1;
        goto cleanup;
    }

    while ((fpath = recdirread(recdir)) != NULL && !terminated) {
        if ((fp = fopen(fpath, "r")) == NULL) {
            if (errno != 0) {
                perror(fpath);
                errno = 0;
            }
            continue;
        }

        sha256(hash, fp, args.nbytes);
        fclose(fp);

        if (args.verbose & VERBOSE_HASH) {
            hash2cstr(hash, hash_cstr);
            printf("%-64s  %s\n", hash_cstr, fpath);
        }

        if (sql != NULL && sql_insert(sql, fpath, hash) != 0) {
            fprintf(stderr, "sqlite3: %s\n", sql_errmsg(sql));
            fprintf(stderr, "terminating...\n");
            errno = 0;
            excode = 1;
            goto cleanup;
        }
    }

cleanup:
    if (sql) sql_close(sql);
    if (recdir) recdirclose(recdir);
    argsfree(&args);

    if (errno != 0)
        die("Could not read directory:");

    return excode;
}
