#include "dbmanager.h"
#include <string.h>
#include <pthread.h>
#include "utils/utils.h"

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */


#if defined(DEBUG_ON)
#define _INIT_DB
#define	_CLOSE_DB
#define DBG     printf
#else
#define DBG(...);
#endif

static sqlite3* g_db;

#if defined(DB_MANAGER_FOR_CGI)
#else
#define _INIT_DB
#define _CLOSE_DB
#endif	/* DB_MANAGER_FOR_CGI */

int row_count = 0;
int last_rowid = 0;

int handlerNum = 0;
typedef int (*callback_func) (void *, int, char**, char**);
callback_func callArr [CALLBACK_SIZE];

int row_count_callback(void *data, int argc, char **argv, char**azColName)
{
	row_count = atoi (argv[0]);
	return 0;
}

int last_rowid_callback(void *data, int argc, char **argv, char**azColName)
{
	last_rowid = atoi (argv[0]);
	return 0;
}

void register_callback_func (callback_func handler)
{
	if (handlerNum >= CALLBACK_SIZE + 1) {
		DBG ("[%s:%d] callback array is full\n", __FILE__, __LINE__);
		return;
	}
	callArr [handlerNum] = handler;
	handlerNum++;
}

int execute_sql(const char* query, callback_func cb)
{
	char* errmsg = 0;
	int ret = sqlite3_exec(g_db, query, cb, NULL, &errmsg);
	if (ret != SQLITE_OK) {
	}
	return ret;
}


#ifdef __cplusplus
}
#endif	/* __cplusplus */
