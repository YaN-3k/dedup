typedef struct SQL SQL;

int sql_open(SQL **sql, const char *filepath);
int sql_insert(SQL *sql, const char *filename, const uchar hash[]);
const char *sql_errmsg(const SQL *sql);
void sql_close(SQL *sql);
