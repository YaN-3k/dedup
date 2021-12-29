#ifndef SQL_H__
#define SQL_H__

typedef struct sql sql_t;

int sql_open(sql_t **sql, const char *filepath);
int sql_insert(sql_t *sql, const char *filename, char unsigned hash[]);
const char *sql_errmsg(sql_t *sql);
void sql_close(sql_t *sql);

#endif
