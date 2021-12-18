#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <unistd.h>

#include "recdir.h"
#include "util.h"

int
main(int argc, char *argv[])
{
    RECDIR recdir;
    int fd;

    assert(argc > 1);

    recdir = recdiropen(argv[1]);
    if (errno != 0)
        die("ERROR: Could not open directory %s:", argv[1]);

    while ((fd = recdirread(recdir)) != -1) {
        /* do something with fd */
        close(fd);
    }

    if (errno != 0)
        die("ERROR: Could not read directory:");

    recdirclose(recdir);
    return 0;
}
