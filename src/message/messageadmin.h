// ライセンス: GPL2

//
// 書き込みビューの管理クラス
//

#ifndef _MESSAGEADMIN_H
#define _MESSAGEADMIN_H

#include "skeleton/admin.h"

namespace MESSAGE
{

#define ID_OF_NEWTHREAD "0000000000"

    enum
    {
        TOOLBAR_MESSAGE = 0,
        TOOLBAR_PREVIEW = 1
    };

    class MessageToolBar;
    class MessageToolBarPreview;

    class MessageAdmin : public SKELETON::Admin
    {
        MessageToolBar* m_toolbar;
        MessageToolBarPreview* m_toolbar_preview;

      public:

        MessageAdmin( const std::string& url );
        virtual ~MessageAdmin();

        void show_entry_new_subject( bool show );
        std::string get_new_subject();

      protected:

        virtual void set_status( const std::string& url, const std::string& stat, const bool force );

        // ツールバー
        virtual void show_toolbar();

        virtual void command_local( const COMMAND_ARGS& command );

      private:

        bool delete_message( SKELETON::View * view );

        // 復元をしない
        virtual void restore( const bool only_locked ){}
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
