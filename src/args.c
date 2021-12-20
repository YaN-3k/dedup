#include "args.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

#include "util.h"

static int isnumber(const char *str);
static void usage(const char *argv0);

int
isnumber(const char *str)
{
    for (; *str && isdigit(*str); str++);
    return *str == '\0';
}

void
usage(const char *argv0)
{
    die("usage: %s [-v] directory [-e exclude]", argv0);
}

void
parseargs(int argc, char *argv[], Args *args)
{
    char opt;
    int i;

    args->verbose = -1;
    while ((opt = getopt(argc, argv, "v:e:rh")) >= 0) {
        switch (opt) {
            case 'v':
                if (isnumber(optarg)) {
                    args->verbose = atoi(optarg);
                } else {
                    printf("%s: invalid argument for option 'v': %s\n", *argv, optarg);
                    exit(1);
                }
                break;
            case 'e':
                args->exclude_reg = optarg;
                break;
            case 'r':
                args->realpath = 1;
                break;
            case 'h':
                usage(*argv);
            case '?':
            case ':':
                exit(1);
        }
    }
    for (i = optind; i < argc; i++) {
        if (args->path == NULL)
            args->path = argv[i];
        else if (args->db == NULL)
            args->db = argv[i];
        else
            die("%s: invalid argument -- '%s'", *argv, argv[i]);
    }

    if (args->path == NULL)
        args->path = "./";

    switch (args->verbose) {
    case 3:  args->verbose = VERBOSE_STACK | VERBOSE_HASH; break;
    case 2:  args->verbose = VERBOSE_STACK;  break;
    case 1:  args->verbose = VERBOSE_HASH;   break;
    case -1: args->verbose = args->db == NULL ? VERBOSE_HASH : 0; break;
    }
}
