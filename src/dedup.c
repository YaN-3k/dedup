#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <unistd.h>
#include <openssl/sha.h>

#include "recdir.h"
#include "sha256.h"
#include "util.h"

int
main(int argc, char *argv[])
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    RECDIR recdir;
    int fd;

    assert(argc > 1);

    recdir = recdiropen(argv[1]);
    if (errno != 0)
        die("ERROR: Could not open directory %s:", argv[1]);

    while ((fd = recdirread(recdir)) != -1) {
        //if (!(sha256(fd, 1024, hash) > 0))
            //continue;
        printf("%d\n", fd);

        //for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
            //printf("%x", hash[i]);
        //putchar('\n');
    }

    if (errno != 0)
        die("ERROR: Could not read directory:");

    recdirclose(recdir);

    return 0;
}
