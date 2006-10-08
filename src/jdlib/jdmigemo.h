// ライセンス: 最新のGPL

//
// Thanks to 「テスト運用中」スレの18氏
//
// http://jd4linux.sourceforge.jp/cgi-bin/bbs/test/read.cgi/support/1149945056/18
//

#ifndef __JD_MIGEMO_H__
#define __JD_MIGEMO_H__

#ifdef HAVE_MIGEMO_H

#include <migemo.h>
#include <regex.h>

#ifdef __cplusplus
extern "C" {
#endif

    int jd_migemo_regcomp(regex_t *preg,const char *regex,int cflags);
    int jd_migemo_init(const char *filename);
    int jd_migemo_close(void);

#ifdef __cplusplus
}
#endif

#define JD_MIGEMO_DICTNAME ("/usr/share/migemo/utf-8/migemo-dict")

#endif /* #ifdef HAVE_MIGEMO_H */
#endif /* #ifndef __JD_MIGEMO_H__ */
