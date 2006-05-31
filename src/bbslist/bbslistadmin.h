// ライセンス: 最新のGPL

//
// 板の管理クラス
//
#ifndef _BBSLISTADMIN_H
#define _BBSLISTADMIN_H

#include "skeleton/admin.h"

#include <string>

namespace BBSLIST
{
    class BBSListAdmin : public SKELETON::Admin
    {
      public:
        BBSListAdmin( const std::string& url );
        ~BBSListAdmin();

      protected:
        SKELETON::View* create_view( const COMMAND_ARGS& command );
        virtual void command_local( const COMMAND_ARGS& command );

        virtual void restore();

        // bbslistはクローズしない
        virtual void close_view( const std::string& url ){}
        virtual void close_all_view( const std::string& url ){}

        // タブメニュー表示キャンセル
        virtual void slot_tab_menu( int page, int x, int y ){}
    };
    
    BBSLIST::BBSListAdmin* get_admin();
    void delete_admin();
}

#endif
