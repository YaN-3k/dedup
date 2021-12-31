#include "libc.h"
#include "arg.h"

#define ARG_NUMBER(arg)                                                       \
        do {                                                                  \
                arg = atoi(optarg);                                           \
                if(arg == 0 && optarg[0] != '0')                              \
                        die("%s: invalid argument for option '%c' -- %s",     \
                            *argv, opt, optarg);                              \
        } while(0)

void
usage(const char *argv0)
{
	die("usage: %s [-rhV] [-v level] [-c nbytes] [-m mindepth]"
	    " [-M maxdepth] [-e exclude] directory [database]", argv0);
	exit(1);
}

void
argparse(int argc, char *argv[], Arg *arg)
{
	const char *exclude;
	size_t errlen;
	char *errmsg;
	int errcode;
	char *tmp_path;
	char opt;
	int i;

	exclude       = NULL;
	arg->mindepth =  0;
	arg->maxdepth = -1;
	arg->nbytes   = -1;
	arg->verbose  = -1;

	while((opt = getopt(argc, argv, "rhVv:c:m:M:e:")) >= 0) {
		switch(opt) {
		case 'm': ARG_NUMBER(arg->mindepth); break;
		case 'M': ARG_NUMBER(arg->maxdepth); break;
		case 'v': ARG_NUMBER(arg->verbose);  break;
		case 'c': ARG_NUMBER(arg->nbytes);   break;
		case 'V': die("%s " VERSION, *argv); break;
		case 'e': exclude = optarg;          break;
		case 'r': arg->realpath = 1;         break;
		case 'h': usage(*argv);              break;
		case '?': usage(*argv);              break;
		case ':': usage(*argv);              break;
		}
	}
	for(i = optind; i < argc; i++) {
		if(arg->path == NULL)    arg->path = argv[i];
		else if(arg->db == NULL) arg->db   = argv[i];
		else die("%s: invalid argument -- '%s'", *argv, argv[i]);
	}
	if(arg->path == NULL) arg->path = "./";
	if(arg->realpath) {
		if((tmp_path = realpath(arg->path, NULL)) == NULL) {
			arg->realpath = 0;
			argfree(arg);
			die("%s:", arg->path);
		} else arg->path = tmp_path;
	}
	errno = 0;
	if(exclude != NULL) {
		arg->exclude = xmalloc(sizeof *arg->exclude);
		if((errcode = regcomp(arg->exclude, exclude, 0)) != 0) {
			errlen = regerror(errcode, arg->exclude, NULL, 0);
			errmsg = xmalloc(errlen);
			regerror(errcode, arg->exclude, errmsg, errlen);
			fprintf(stderr, "%s: %s\n", exclude, errmsg);
			argfree(arg);
			free(errmsg);
			exit(1);
		}
	}
	switch(arg->verbose) {
	case -1: arg->verbose = (arg->db == NULL) ? VERBOSE_HASH : 0; break;
	case  0: arg->verbose = VERBOSE_SILENT;                       break;
	case  1: arg->verbose = VERBOSE_HASH;                         break;
	case  2: arg->verbose = VERBOSE_STACK;                        break;
	default: arg->verbose = VERBOSE_STACK | VERBOSE_HASH;         break;
	}
}

void
argfree(Arg *arg)
{
	if(arg->realpath) free((char *)arg->path);
	if(arg->exclude) {
		regfree(arg->exclude);
		free(arg->exclude);
	}
}
