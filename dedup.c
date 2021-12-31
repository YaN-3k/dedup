#include <openssl/sha.h>
#include <signal.h>
#include "libc.h"
#include "arg.h"
#include "queue.h"
#include "rcdir.h"
#include "sha256.h"
#include "sql.h"

typedef struct {
	Arg arg;
	SQL *sql;
	queue_t *readq;
	queue_t *writeq;
} Data;

typedef struct {
	queue_lnk lnk;
	const char *path;
	uchar hash[SHA256_LEN];
	char hashstr[SHA256_STR_LEN];
} Task;

static void sigint_handler(int sig);
static void task_add(const char *path, queue_t *queue);
static void task_free(Task *task);
static void task_queue_destroy(queue_t **queue);
static void *prcsf(void *datap);
static void *writedb(void *datap);

static int terminate;

void
sigint_handler(int sig)
{
	(void)sig;
	fputs("terminating...\n", stderr);
	terminate = 1;
}

void
task_add(const char *path, queue_t *queue)
{
	Task *task;

	task = xmalloc(sizeof *task);
	task->path = path;
	enqueue(&task->lnk, queue);
}

void
task_free(Task *task)
{
	free((char *)task->path);
	free(task);
}

void
task_queue_destroy(queue_t **queue)
{
	while(!queue_empty(*queue)) task_free(dequeue(*queue, Task, lnk));
	queue_destroy(queue);
}

void *
prcsf(void *datap)
{
	Data *data;
	Task *task;
	FILE *fp;

	data = datap;
	while(1) {
		task = dequeue(data->readq, Task, lnk);
		if(terminate > 0 || task->path == NULL) break;
		if((fp = fopen(task->path, "r")) == NULL) {
			if(errno != 0) {
				perror(task->path);
				errno = 0;
			}
			task_free(task);
			continue;
		}
		sha256(task->hash, fp, data->arg.nbytes);
		fclose(fp);
		if(data->arg.verbose & VERBOSE_HASH) {
			hash2str(task->hash, task->hashstr);
			printf("%-64s  %s\n", task->hashstr, task->path);
		}
		if(data->sql != NULL) enqueue(&task->lnk, data->writeq);
		else task_free(task);
	}
	task_free(task);
	return 0;
}

void *
writedb(void *datap)
{
	Data *data;
	Task *task;

	data = datap;
	while(1) {
		task = dequeue(data->writeq, Task, lnk);
		if(terminate > 0 || task->path == NULL)
			break;
		if(sql_insert(data->sql, task->path, task->hash) != 0) {
			fprintf(stderr, "sqlite: %s\n", sql_errmsg(data->sql));
			terminate = 1;
			errno = 0;
			break;
		}
		task_free(task);
	}
	task_free(task);
	return 0;
}

int
main(int argc, char *argv[])
{
	pthread_t tids[THREADS + 1];
	RCDIR *rcdir;
	Data data;
	char *path;
	size_t tidc, i;

	tidc = 0;
	rcdir = NULL;
	memset(tids, 0, sizeof tids);
	memset(&data, 0, sizeof data);

	signal(SIGINT, sigint_handler);
	argparse(argc, argv, &data.arg);
	if(data.arg.db && sql_open(&data.sql, data.arg.db) != 0) {
		fprintf(stderr, "sqlite: %s\n", sql_errmsg(data.sql));
		terminate = 1;
		errno = 0;
		goto cleanup;
	}
	if((rcdir = rcdiropen(&data.arg)) == NULL) {
		perror(data.arg.path);
		terminate = 1;
		errno = 0;
		goto cleanup;
	}
	queue_init(&data.readq);
	queue_init(&data.writeq);
	for(tidc = 0; tidc < THREADS; tidc++)
		if(pthread_create(tids + tidc, NULL, prcsf, &data)) {
			perror("failed to create thread");
			terminate = 1;
			errno = 0;
			goto cleanup;
		}
	if(pthread_create(tids + tidc, NULL, writedb, &data)) {
		perror("failed to create thread");
		terminate = 1;
		errno = 0;
		goto cleanup;
	}
	while(!terminate && (path = rcdirread(rcdir)) != NULL)
		task_add(path, data.readq);
cleanup:
	for(i = 0; i < tidc; i++) task_add(NULL, data.readq);
	for(i = 0; i < tidc; i++) pthread_join(tids[i], NULL);
	if(tids[tidc]) {
		task_add(NULL, data.writeq);
		pthread_join(tids[tidc], NULL);
	}
	if(data.writeq) task_queue_destroy(&data.writeq);
	if(data.readq) task_queue_destroy(&data.readq);
	if(data.sql) sql_close(data.sql);
	if(rcdir) rcdirclose(rcdir);
	argfree(&data.arg);
	if(errno != 0) die("could not read directory:");
	return terminate;
}
