// ライセンス: GPL2

//
// 板の管理クラス
//
#ifndef _BBSLISTADMIN_H
#define _BBSLISTADMIN_H

#include "skeleton/admin.h"

#include "type.h"
#include "data_info.h"

#include <string>
#include <vector>

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

        void save_session() override;
        
        // 履歴を DATA_INFO_LIST 型で取得
        void get_history( const std::string& url, CORE::DATA_INFO_LIST& info_list );

        // サイドバーの指定したidのディレクトリに含まれるスレのアドレスを取得
        void get_threads( const std::string& url, const int dirid, std::vector< std::string >& list_url );

        // サイドバーの指定したidのディレクトリの名前を取得
        const std::string get_dirname( const std::string& url, const int dirid );

      protected:

        SKELETON::View* create_view( const COMMAND_ARGS& command ) override;

        // ツールバー
        void show_toolbar() override;
        void toggle_toolbar() override;

        void command_local( const COMMAND_ARGS& command ) override;

        void restore( const bool only_locked ) override;
        COMMAND_ARGS url_to_openarg( const std::string& url, const bool tab, const bool lock ) override;

        void switch_admin() override;

        // bbslistはクローズしない
        void close_view( const std::string& url ) override {}
        void close_all_view( const std::string& url ) override {}

        // タブの D&D 処理をしない
        void slot_drag_data_get( Gtk::SelectionData& selection_data, const int page ) override {}

        // タブメニュー表示キャンセル
        void slot_tab_menu( int page, int x, int y ) override {}
    };

    
    BBSLIST::BBSListAdmin* get_admin();
    void delete_admin();

    SKELETON::UNDO_BUFFER* get_undo_buffer_favorite();
    void delete_undo_buffer_favorite();
}

#endif
