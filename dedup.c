#include <errno.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <openssl/sha.h>
#include <regex.h>

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

static void sigint_handler(int sig);

static void task_add(const char *fpath, queue_t *queue);
static void task_free(struct task_entry *entry);
static void task_queue_destroy(queue_t **queue);

static void *process_file(void *datap);
static void *write_database(void *datap);

static int terminate = 0;

void
sigint_handler(int sig)
{
    (void) sig;
    fputs("terminating...\n", stderr);
    terminate = 1;
}

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

void
task_queue_destroy(queue_t **queue)
{
    while (!queue_empty(*queue))
        task_free(dequeue(*queue, struct task_entry, lnk));

    queue_destroy(queue);
}

void *
process_file(void *datap)
{
    struct data *data = datap;
    struct task_entry *entry;
    FILE *fp;

    while (1) {
        entry = dequeue(data->read_q, struct task_entry, lnk);

        if (terminate > 0 || entry->fpath == NULL)
            break;

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

    while (1) {
        entry = dequeue(data->write_q, struct task_entry, lnk);

        if (terminate > 0 || entry->fpath == NULL)
            break;

        if (sql_insert(data->sql, entry->fpath, entry->hash) != 0) {
            fprintf(stderr, "sqlite3: %s\n", sql_errmsg(data->sql));
            terminate = 1;
            errno = 0;
            break;
        }
        task_free(entry);
    }

    task_free(entry);
    return 0;
}

int
main(int argc, char *argv[])
{
    pthread_t threads[THREADS + 1] = {0};
    size_t threads_sz = 0;
    struct data data = {0};
    RECDIR *recdir = 0;
    char *fpath;
    size_t i;

    signal(SIGINT, sigint_handler);

    argsparse(argc, argv, &data.args);

    if (data.args.db && sql_open(&data.sql, data.args.db) != 0) {
        fprintf(stderr, "sqlite3: %s\n", sql_errmsg(data.sql));
        terminate = 1;
        errno = 0;
        goto cleanup;
    }
    
    if ((recdir = recdiropen(&data.args)) == NULL) {
        perror(data.args.path);
        terminate = 1;
        errno = 0;
        goto cleanup;
    }

    queue_init(&data.read_q);
    queue_init(&data.write_q);

    for (threads_sz = 0; threads_sz < THREADS; threads_sz++) {
        if (pthread_create(threads + threads_sz, NULL, process_file, &data)) {
            perror("failed to create thread");
            terminate = 1;
            errno = 0;
            goto cleanup;
        }
    }

    if (pthread_create(threads + threads_sz, NULL, write_database, &data)) {
        perror("failed to create thread");
        terminate = 1;
        errno = 0;
        goto cleanup;
    }

    while (!terminate && (fpath = recdirread(recdir)) != NULL)
        task_add(fpath, data.read_q);

cleanup:
    for (i = 0; i < threads_sz; i++)
        task_add(NULL, data.read_q);

    for (i = 0; i < threads_sz; i++)
        pthread_join(threads[i], NULL);

    if (threads[threads_sz]) {
        task_add(NULL, data.write_q);
        pthread_join(threads[threads_sz], NULL);
    }

    if (data.write_q) task_queue_destroy(&data.write_q);
    if (data.read_q) task_queue_destroy(&data.read_q);
    if (data.sql) sql_close(data.sql);
    if (recdir) recdirclose(recdir);
    argsfree(&data.args);

    if (errno != 0)
        die("could not read directory:");

    return terminate;
}
