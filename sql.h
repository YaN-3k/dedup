#ifndef SQL_H__
#define SQL_H__

typedef struct SQL SQL;

int sql_open(SQL **sql, const char *filepath);
int sql_insert(SQL *sql, const char *filename, char unsigned hash[]);
const char *sql_errmsg(SQL *sql);
void sql_close(SQL *sql);

#endif
