// ライセンス: GPL2

//
// 書き込みビューの管理クラス
//

#ifndef _MESSAGEADMIN_H
#define _MESSAGEADMIN_H

#include "skeleton/admin.h"

namespace MESSAGE
{
    class MessageAdmin : public SKELETON::Admin
    {
      public:

        MessageAdmin( const std::string& url );
        virtual ~MessageAdmin();

      protected:

        virtual void command_local( const COMMAND_ARGS& command );

      private:

        bool delete_message( SKELETON::View * view );

        // 復元をしない
        virtual void restore(){}
        virtual COMMAND_ARGS url_to_openarg( const std::string& url, const bool tab, const bool lock ){
            COMMAND_ARGS ret;
            return ret;
        }

        virtual void open_view( const COMMAND_ARGS& command );
        virtual void switch_admin();
        virtual void tab_left();
        virtual void tab_right();
        virtual void close_view( const std::string& url );
        virtual void open_window();
        virtual void close_window();

        // タブの D&D 処理をしない
        virtual void slot_drag_begin( int page ){}
        virtual void slot_drag_end(){}
    };
    
    MESSAGE::MessageAdmin* get_admin();
    void delete_admin();
}

#endif
