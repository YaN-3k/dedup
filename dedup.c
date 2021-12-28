#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"
#include "queue.h"
#include "recdir.h"
#include "sha256.h"
#include "sql.h"
#include "util.h"

SQL *sql;
Args args;

struct task_entry {
    const char *fpath;
    struct queue_link link;
};

void
task_add(const char *fpath, struct queue_head *head)
{
    struct task_entry *entry = emalloc(sizeof(struct task_entry));
    entry->fpath = fpath;
    enqueue(&entry->link, head);
}

void
task_free(struct task_entry *entry)
{
    free((char *)entry->fpath);
    free(entry);
}

void *
process_file(void *data)
{
    unsigned char hash[SHA256_LENGTH];
    char hash_cstr[SHA256_CSTR_LENGTH];
    struct queue_head *tasks = data;
    struct task_entry *entry;
    FILE *fp;

    while (1) {
        entry = dequeue_entry(tasks, struct task_entry, link);

        if (entry->fpath == NULL)
            break;

        if ((fp = fopen(entry->fpath, "r")) == NULL) {
            if (errno != 0) {
                perror(entry->fpath);
                errno = 0;
            }
            task_free(entry);
            continue;
        }

        sha256(hash, fp, args.nbytes);
        fclose(fp);

        if (args.verbose & VERBOSE_HASH) {
            hash2cstr(hash, hash_cstr);
            printf("%-64s  %s\n", hash_cstr, entry->fpath);
        }

        if (sql != NULL && sql_insert(sql, entry->fpath, hash) != 0) {
            fprintf(stderr, "sqlite3: %s\n", sql_errmsg(sql));
            /*fprintf(stderr, "terminating...\n");*/
            errno = 0;
            break;
        }
        task_free(entry);
    }

    task_free(entry);
    pthread_exit(0);
}

int
main(int argc, char *argv[])
{
    pthread_t threads[THREADS];
    struct queue_head tasks;
    RECDIR *recdir = NULL;
    int excode = 0;
    char *fpath;
    size_t i;

    queue_head_init(&tasks);

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

    for (i = 0; i < THREADS; i++)
        pthread_create(threads + i, NULL, process_file, &tasks);

    while ((fpath = recdirread(recdir)) != NULL)
        task_add(fpath, &tasks);

    for (i = 0; i < THREADS; i++)
        task_add(NULL, &tasks);
    
    for (i = 0; i < THREADS; i++)
        pthread_join(threads[i], NULL);

cleanup:
    if (sql) sql_close(sql);
    if (recdir) recdirclose(recdir);
    argsfree(&args);
    queue_head_destroy(&tasks);

    if (errno != 0)
        die("Could not read directory:");

    return excode;
}
