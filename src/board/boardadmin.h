// ライセンス: GPL2

//
// 個別の板の管理クラス
//
#ifndef _BOARDADMIN_H
#define _BOARDADMIN_H

#include "skeleton/admin.h"

#include <string>

namespace BOARD
{
    class BoardAdmin : public SKELETON::Admin
    {
      public:
        BoardAdmin( const std::string& url );
        ~BoardAdmin();

      protected:
        virtual COMMAND_ARGS get_open_list_args( const std::string& url );
        SKELETON::View* create_view( const COMMAND_ARGS& command );
        virtual void command_local( const COMMAND_ARGS& command );

        virtual void restore();
        virtual void switch_admin();

      private:
        virtual void slot_drag_begin( int page );
        virtual void slot_drag_end();
    };
    
    BoardAdmin* get_admin();
    void delete_admin();
}

#endif
