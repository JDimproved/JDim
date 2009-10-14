/* HTTP コード */

#ifndef _HTTPCODE_H
#define _HTTPCODE_H

enum
{
    HTTP_ERR = -1,
    HTTP_INIT = 0,
    HTTP_CANCEL = 1,
    HTTP_OLD = 2, // offlaw や 過去ログ倉庫からの読み込み

    HTTP_OK = 200,
    HTTP_PARTIAL_CONTENT = 206,
    HTTP_REDIRECT = 302,
    HTTP_NOT_MODIFIED = 304,
    HTTP_NOT_FOUND = 404,
    HTTP_TIMEOUT = 408,
    HTTP_RANGE_ERR = 416
};

#endif
