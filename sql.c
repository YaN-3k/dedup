#include <openssl/sha.h>
#include <sqlite3.h>
#include "libc.h"
#include "sha256.h"
#include "sql.h"

#define INSERT_LIM 1024
#define SQL_TRY(cmd) if((errcode = cmd)) return errcode

struct SQL {
	sqlite3 *db;
	sqlite3_stmt *stmt;
	pthread_mutex_t mtx;
	int insertc;
};

int
sql_open(SQL **sql, const char *path)
{
	const char *createcmd, *insertcmd;
	int openflags, errcode;

	createcmd = "CREATE TABLE IF NOT EXISTS resources("
	            "  res_path TEXT NOT NULL UNIQUE,     "
	            "  res_hash BLOB NOT NULL             "
	            ")                                    ";
	insertcmd = "REPLACE INTO resources VALUES(?, ?)";
	openflags = SQLITE_OPEN_READWRITE
	          | SQLITE_OPEN_CREATE
	          | SQLITE_OPEN_NOMUTEX;
	*sql = xcalloc(1, sizeof **sql);

	pthread_mutex_init(&(*sql)->mtx, NULL);
	SQL_TRY(sqlite3_open_v2(path, &(*sql)->db, openflags, NULL));
	SQL_TRY(sqlite3_exec((*sql)->db, createcmd, NULL, NULL, NULL));
	SQL_TRY(sqlite3_prepare_v2((*sql)->db, insertcmd, -1, &(*sql)->stmt, NULL));
	SQL_TRY(sqlite3_exec((*sql)->db, "BEGIN", NULL, NULL, NULL));
	return 0;
}

int
sql_insert(SQL *sql, const char *filename, const uchar hash[])
{
	int errcode;

	pthread_mutex_lock(&sql->mtx);
	sqlite3_bind_text(sql->stmt, 1, filename, -1, NULL);
	sqlite3_bind_blob(sql->stmt, 2, hash, SHA256_LEN, NULL);
	sqlite3_step(sql->stmt);
	SQL_TRY(sqlite3_reset(sql->stmt));
	if(++sql->insertc >= INSERT_LIM) {
		SQL_TRY(sqlite3_exec(sql->db, "END;BEGIN", NULL, NULL, NULL));
		sql->insertc = 0;
	}
	pthread_mutex_unlock(&sql->mtx);
	return 0;
}

const char *
sql_errmsg(const SQL *sql)
{
	return sqlite3_errmsg(sql->db);
}

void
sql_close(SQL *sql)
{
	sqlite3_exec(sql->db, "END", NULL, NULL, NULL);
	if(sql->stmt) sqlite3_finalize(sql->stmt);
	if(sql->db) sqlite3_close(sql->db);
	pthread_mutex_destroy(&sql->mtx);
	free(sql);
}
