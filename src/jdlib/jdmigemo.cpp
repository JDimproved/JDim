// ライセンス: GPL2

//
// Thanks to 「テスト運用中」スレの18氏
//
// http://jd4linux.sourceforge.jp/cgi-bin/bbs/test/read.cgi/support/1149945056/18
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_MIGEMO_H

//#define _DEBUG
#include "jddebug.h"

#include "jdmigemo.h"

#include <stdlib.h>
#include <string>

migemo *migemo_object;


int jd_migemo_regcomp(regex_t *preg,const char *regex,int cflags)
{
    migemo *m;
    int retval,i;
    size_t len;
    unsigned char *p,*utmp;
    char *ctmp;
    m=migemo_object;
    if(!m){
        return -1;
    }
    //migemo_load(m,MIGEMO_DICTID_MIGEMO,JD_MIGEMO_DICTNAME);
    if(!migemo_is_enable(m)){
        return -1;
    }
    utmp=(unsigned char *)calloc(sizeof(unsigned char),strlen(regex)+1);
    memcpy(utmp,regex,strlen(regex));
    for(i=0;utmp[i]!='\0';i++){
        if(utmp[i]=='\n'){
            utmp[i]='\0';
            break;
        }
    }
    p=migemo_query(m,utmp);
    free(utmp);
    for(len=0;p[len]!='\0';len++);
    ctmp=(char *)calloc(sizeof(char),len+1);
    memcpy(ctmp,p,len);

#ifdef _DEBUG
    std::cout << "migemo comp:" << ctmp << std::endl;
#endif

    retval=regcomp(preg,ctmp,cflags);
    free(ctmp);
    if(retval!=0){
        regfree(preg);
    }
    migemo_release(m,p);
    return retval;
}



int jd_migemo_init(const char *filename)
{
#ifdef _DEBUG
	std::cout << "migemo-dict: " << filename << std::endl;
#endif
    migemo_object=migemo_open(filename);
    if(migemo_is_enable(migemo_object)){
        return 1;
    }else{
        migemo_close(migemo_object);
        migemo_object=NULL;
        return 0;
    }
}


int jd_migemo_close(void)
{
    migemo_close(migemo_object);
    migemo_object=NULL;
    return 1;
}

#endif


