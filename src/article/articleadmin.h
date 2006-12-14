// ライセンス: GPL2

//
// 記事の管理クラス
//
#ifndef _ARTICLEADMIN_H
#define _ARTICLEADMIN_H

#include "skeleton/admin.h"

#include <list>
#include <string>

#define ARTICLE_SIGN "_ARTICLE_"
#define RES_SIGN     "_RES_"
#define NAME_SIGN    "_NAME_"
#define ID_SIGN      "_ID_"
#define BOOKMK_SIGN  "_BM_"
#define URL_SIGN     "_URL_"
#define REFER_SIGN   "_REF_"
#define KEYWORD_SIGN "_KW_"
#define ORMODE_SIGN      "_OR_"
#define CENTER_SIGN  "_CENTER_"
#define TIME_SIGN    "_TIME_"

namespace ARTICLE
{
    class ArticleAdmin : public SKELETON::Admin
    {
      public:
        ArticleAdmin( const std::string& url );
        ~ArticleAdmin();

      protected:
        virtual COMMAND_ARGS get_open_list_args( const std::string& url );
        SKELETON::View* create_view( const COMMAND_ARGS& command );
        virtual void command_local( const COMMAND_ARGS& command );

        virtual void restore();
        virtual void switch_admin();
        virtual void set_tabicon( const std::string& url, const std::string& iconname );

      private:
        void delete_popup();

        virtual void slot_drag_begin( int page );
        virtual void slot_drag_end();
    };
    
    ARTICLE::ArticleAdmin* get_admin();
    void delete_admin();
}

#endif
