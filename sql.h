typedef struct sql sql_t;

int sql_open(sql_t **sql, const char *filepath);
int sql_insert(sql_t *sql, const char *filename, const char unsigned hash[]);
const char *sql_errmsg(const sql_t *sql);
void sql_close(sql_t *sql);
