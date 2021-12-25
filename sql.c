#include "sql.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <sqlite3.h>

#include "sha256.h"
#include "util.h"

#define INSERT_LIM 1024
#define SQL_TRY(exp) if ((errcode = exp)) return errcode

struct SQL {
    sqlite3 *database;
    sqlite3_stmt *stmt;
    int insertc;
};

int
sql_open(SQL **sql, const char *path)
{
    const char *create_cmd = "CREATE TABLE IF NOT EXISTS resources("
                            "  res_path TEXT NOT NULL UNIQUE,      "
                            "  res_hash BLOB NOT NULL              "
                            ")                                     ";

    const char *insert_cmd = "REPLACE INTO resources VALUES(?, ?)";
    int errcode;

    *sql = ecalloc(1, sizeof(struct SQL));

    SQL_TRY(sqlite3_open(path, &(*sql)->database));
    SQL_TRY(sqlite3_exec((*sql)->database, create_cmd, NULL, NULL, NULL));
    SQL_TRY(sqlite3_prepare_v2((*sql)->database, insert_cmd, -1, &(*sql)->stmt, NULL));
    SQL_TRY(sqlite3_exec((*sql)->database, "BEGIN", NULL, NULL, NULL));

    return 0;
}

int
sql_insert(SQL *sql, const char *filename, char unsigned hash[])
{
    int errcode;

    sqlite3_bind_text(sql->stmt, 1, filename, -1, NULL);
    sqlite3_bind_blob(sql->stmt, 2, hash, SHA256_LENGTH, NULL);

    sqlite3_step(sql->stmt);
    SQL_TRY(sqlite3_reset(sql->stmt));

    if (++sql->insertc >= INSERT_LIM) {
        SQL_TRY(sqlite3_exec(sql->database, "COMMIT;BEGIN", NULL, NULL, NULL));
        sql->insertc = 0;
    }

    return 0;
}

const char *
sql_errmsg(SQL *sql)
{
    return sqlite3_errmsg(sql->database);
}

void
sql_close(SQL *sql)
{
    sqlite3_exec(sql->database, "COMMIT", NULL, NULL, NULL);
    if (sql->stmt) sqlite3_finalize(sql->stmt);
    if (sql->database) sqlite3_close(sql->database);
    free(sql);
}
