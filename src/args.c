#include "args.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <unistd.h>

#include "util.h"

static void compreg(const char *regstr, regex_t *reg);
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
compreg(const char *regstr, regex_t *reg)
{
    size_t error_len;
    char *error_msg;
    int errcode;

    if ((errcode = regcomp(reg, regstr, 0)) != 0) {
        error_len = regerror(errcode, reg, NULL, 0);
        error_msg = alloca(error_len);
        regerror(errcode, reg, error_msg, error_len);
        die("%s: %s", regstr, error_msg);
    }
}

void
argsparse(int argc, char *argv[], Args *args)
{
    char *tmp;
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
                args->exclude_reg = malloc(sizeof(regex_t));
                compreg(optarg, args->exclude_reg);
                break;
            case 'r':
                args->realpath = 1;
                break;
            case 'h':
                usage(*argv);
                break;
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

    if (args->realpath) {
        if ((tmp = realpath(args->path, NULL)) == NULL)
            die("%s:", args->path);
        else
            args->path = tmp;
    }
    errno = 0;


    switch (args->verbose) {
    case  3:  args->verbose = VERBOSE_STACK | VERBOSE_HASH; break;
    case  2:  args->verbose = VERBOSE_STACK;  break;
    case  1:  args->verbose = VERBOSE_HASH;   break;
    case  0:  args->verbose = 0; break;
    case -1: args->verbose = args->db == NULL ? VERBOSE_HASH : 0; break;
    default: die("%s: invalid value for option 'v': %d", *argv, args->verbose);
    }
}

void
argsfree(Args *args)
{
    if (args->exclude_reg) {
        regfree(args->exclude_reg);
        free(args->exclude_reg);
    }

    if (args->realpath)
        free((char *)args->path);
}
