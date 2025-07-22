#ifndef __DBMANAGER_H__
#define __DBMANAGER_H__

#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

enum {
    RECYCLE_COUNT = 100,
    DEFAULT_MAX_SIZE = 5000,
};

enum {
    EVT_ROW_COUNT = 0,
    EVT_LAST_ROWID,
    CALLBACK_SIZE = 16,
};




int InitDB();
void CloseDB();

#ifdef __cplusplus
}
#endif	/* __cplusplus */
#endif /* __DBMANAGER_H__ */
