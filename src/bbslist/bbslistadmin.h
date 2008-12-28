// ライセンス: GPL2

//
// 板の管理クラス
//
#ifndef _BBSLISTADMIN_H
#define _BBSLISTADMIN_H

#include "skeleton/admin.h"

#include <string>

namespace SKELETON
{
    class UNDO_BUFFER;
}

namespace BBSLIST
{
    class BBSListToolBar;

    class BBSListAdmin : public SKELETON::Admin
    {
        BBSListToolBar* m_toolbar;

      public:
        BBSListAdmin( const std::string& url );
        ~BBSListAdmin();

      protected:

        SKELETON::View* create_view( const COMMAND_ARGS& command );

        // ツールバー
        virtual void show_toolbar();
        virtual void toggle_toolbar();

        virtual void command_local( const COMMAND_ARGS& command );

        virtual void restore( const bool only_locked );
        virtual COMMAND_ARGS url_to_openarg( const std::string& url, const bool tab, const bool lock );

        virtual void switch_admin();

        // bbslistはクローズしない
        virtual void close_view( const std::string& url ){}
        virtual void close_all_view( const std::string& url ){}

        // (お気に入りの)アイコン表示切り替え
        virtual void toggle_icon( const std::string& url );

        // タブの D&D 処理をしない
        virtual void slot_drag_data_get( Gtk::SelectionData& selection_data, const int page ){}

        // タブメニュー表示キャンセル
        virtual void slot_tab_menu( int page, int x, int y ){}
    };
    
    BBSLIST::BBSListAdmin* get_admin();
    void delete_admin();

    SKELETON::UNDO_BUFFER* get_undo_buffer_favorite();
    void delete_undo_buffer_favorite();
}

#endif
