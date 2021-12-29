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

struct data {
    sql_t *sql;
    queue_t *read_q;
    queue_t *write_q;
    struct args args;
};

struct task_entry {
    queue_lnk lnk;
    const char *fpath;
    unsigned char hash[SHA256_LENGTH];
    char hash_cstr[SHA256_CSTR_LENGTH];
};

void
task_add(const char *fpath, queue_t *queue)
{
    struct task_entry *entry = emalloc(sizeof(struct task_entry));
    entry->fpath = fpath;
    enqueue(&entry->lnk, queue);
}

void
task_free(struct task_entry *entry)
{
    free((char *)entry->fpath);
    free(entry);
}

void *
process_file(void *datap)
{
    struct data *data = datap;
    struct task_entry *entry;
    FILE *fp;

    while ((entry = dequeue(data->read_q, struct task_entry, lnk))->fpath) {
        if ((fp = fopen(entry->fpath, "r")) == NULL) {
            if (errno != 0) {
                perror(entry->fpath);
                errno = 0;
            }
            task_free(entry);
            continue;
        }

        sha256(entry->hash, fp, data->args.nbytes);
        fclose(fp);

        if (data->args.verbose & VERBOSE_HASH) {
            hash2cstr(entry->hash, entry->hash_cstr);
            printf("%-64s  %s\n", entry->hash_cstr, entry->fpath);
        }

        if (data->sql != NULL)
            enqueue(&entry->lnk, data->write_q);
        else
            task_free(entry);
    }

    task_free(entry);
    pthread_exit(0);
}

void *
write_database(void *datap)
{
    struct data *data = datap;
    struct task_entry *entry;

    while ((entry = dequeue(data->write_q, struct task_entry, lnk))->fpath) {
        if (sql_insert(data->sql, entry->fpath, entry->hash) != 0) {
            fprintf(stderr, "sqlite3: %s\n", sql_errmsg(data->sql));
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
    pthread_t threads[THREADS + 1];
    RECDIR *recdir = 0;
    struct data data = {0};
    int excode = 0;
    char *fpath;
    size_t i;

    argsparse(argc, argv, &data.args);

    if (data.args.db && sql_open(&data.sql, data.args.db) != 0) {
        fprintf(stderr, "sqlite3: %s\n", sql_errmsg(data.sql));
        errno = 0;
        excode = 1;
        goto cleanup;
    }
    
    if ((recdir = recdiropen(&data.args)) == NULL) {
        perror(data.args.path);
        errno = 0;
        excode = 1;
        goto cleanup;
    }

    queue_init(&data.read_q);
    queue_init(&data.write_q);

    for (i = 0; i < THREADS; i++)
        pthread_create(threads + i, NULL, process_file, &data);

    pthread_create(threads + THREADS, NULL, write_database, &data);

    while ((fpath = recdirread(recdir)) != NULL)
        task_add(fpath, data.read_q);

    for (i = 0; i < THREADS; i++)
        task_add(NULL, data.read_q);

    for (i = 0; i < THREADS; i++)
        pthread_join(threads[i], NULL);

    task_add(NULL, data.write_q);
    pthread_join(threads[THREADS], NULL);


cleanup:
    if (data.write_q) queue_destroy(&data.write_q);
    if (data.read_q) queue_destroy(&data.read_q);
    if (data.sql) sql_close(data.sql);
    if (recdir) recdirclose(recdir);
    argsfree(&data.args);

    if (errno != 0)
        die("Could not read directory:");

    return excode;
}
