#include "args.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

#include "util.h"

#define ARGS_NUMBER(arg)                                           \
        do {                                                       \
            arg = atoi(optarg);                                    \
            if (arg == 0 && optarg[0] != '0')                      \
                die("%s: invalid argument for option '%c' -- %s",  \
                    *argv, opt, optarg);                           \
        } while (0)

void
usage(const char *argv0)
{
    die("usage: %s [-rhV] [-v level] [-c nbytes] [-m mindepth]"
        " [-M maxdepth] [-e exclude] directory [database]", argv0);
    exit(1);
}

void
argsparse(int argc, char *argv[], struct args *args)
{
    const char *exclude_reg = NULL;
    size_t errlen;
    char *errmsg;
    int errcode;
    char *tmp_path;
    char opt;
    int i;

    args->mindepth =  0;
    args->maxdepth = -1;
    args->nbytes   = -1;
    args->verbose  = -1;

    while ((opt = getopt(argc, argv, "rhVv:c:m:M:e:")) >= 0) {
        switch (opt) {
        case 'm': ARGS_NUMBER(args->mindepth); break;
        case 'M': ARGS_NUMBER(args->maxdepth); break;
        case 'v': ARGS_NUMBER(args->verbose);  break;
        case 'c': ARGS_NUMBER(args->nbytes);   break;
        case 'V': die("%s " VERSION, *argv);   break;
        case 'e': exclude_reg = optarg;        break;
        case 'r': args->realpath = 1;          break;
        case 'h': usage(*argv);                break;
        case '?': usage(*argv);                break;
        case ':': usage(*argv);                break;
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
        if ((tmp_path = realpath(args->path, NULL)) == NULL) {
            args->realpath = 0;
            argsfree(args);
            die("%s:", args->path);
        } else {
            args->path = tmp_path;
        }
    }
    errno = 0;

    if (exclude_reg) {
        args->exclude_reg = emalloc(sizeof(regex_t));
        if ((errcode = regcomp(args->exclude_reg, exclude_reg, 0)) != 0) {
            errlen = regerror(errcode, args->exclude_reg, NULL, 0);
            errmsg = alloca(errlen);
            regerror(errcode, args->exclude_reg, errmsg, errlen);
            argsfree(args);
            die("%s: %s", exclude_reg, errmsg);
        }
    }

    switch (args->verbose) {
    case -1: args->verbose = (args->db == NULL) ? VERBOSE_HASH : 0; break;
    case  0: args->verbose = VERBOSE_SILENT;                        break;
    case  1: args->verbose = VERBOSE_HASH;                          break;
    case  2: args->verbose = VERBOSE_STACK;                         break;
    default: args->verbose = VERBOSE_STACK | VERBOSE_HASH;          break;
    }
}

void
argsfree(struct args *args)
{
    if (args->exclude_reg) {
        regfree(args->exclude_reg);
        free(args->exclude_reg);
    }

    if (args->realpath)
        free((char *)args->path);
}
