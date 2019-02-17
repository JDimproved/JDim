// ライセンス: GPL2

//
// 個別の板の管理クラス
//
#ifndef _BOARDADMIN_H
#define _BOARDADMIN_H

#include "skeleton/admin.h"

#include "sign.h"

#include <string>

namespace BOARD
{
    class BoardToolBar;

    class BoardAdmin : public SKELETON::Admin
    {
        BoardToolBar* m_toolbar;

      public:
        BoardAdmin( const std::string& url );
        ~BoardAdmin();

        void save_session() override;

      protected:

        COMMAND_ARGS get_open_list_args( const std::string& url, const COMMAND_ARGS& command_list ) override;
        SKELETON::View* create_view( const COMMAND_ARGS& command ) override;

        // view_modeに該当するページを探す
        int find_view( const std::string& view_mode ) override;

        // ツールバー
        void show_toolbar() override;
        void toggle_toolbar() override;
        void open_searchbar() override;
        void close_searchbar() override;

        void command_local( const COMMAND_ARGS& command ) override;

        void restore( const bool only_locked ) override;
        COMMAND_ARGS url_to_openarg( const std::string& url, const bool tab, const bool lock ) override;
        std::string command_to_url( const COMMAND_ARGS& command ) override;

        void switch_admin() override;

        void restore_lasttab() override;

      private:

        // タブをお気に入りにドロップした時にお気に入りがデータ送信を要求してきた
        void slot_drag_data_get( Gtk::SelectionData& selection_data, const int page ) override;
    };
    
    BoardAdmin* get_admin();
    void delete_admin();
}

#endif
