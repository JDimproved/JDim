// ライセンス: GPL2

//
// コアクラス
//

#ifndef _CORE_H
#define _CORE_H

#include <gtkmm.h>
#include <list>
#include <string>

#include "skeleton/imgbutton.h"
#include "skeleton/hpaned.h"
#include "skeleton/vpaned.h"
#include "skeleton/notebook.h"
#include "skeleton/vbox.h"

#include "command_args.h"

class WinMain;


namespace BOARD
{
    class BoardAdmin;
}


namespace BBSLIST
{
    class BBSListAdmin;
}


namespace ARTICLE
{
    class ArticleAdmin;
}

namespace IMAGE
{
    class ImageAdmin;
}


namespace MESSAGE
{
    class MessageAdmin;
}


namespace CORE
{
    class DND_Manager;
    class HistoryMenuThread;
    class HistoryMenuBoard;

    class Core
    {
        Glib::Dispatcher m_disp;
        std::list< COMMAND_ARGS > m_list_command;
        sigc::connection m_sigc_switch_page;

        WinMain& m_win_main;

        // フォーカス制御用カウンタ
        int m_count_focusout;

        // アイコン化されている
        bool m_iconified;
        
        SKELETON::JDVBox m_vbox_main;
        SKELETON::JDHPaned m_hpaned;

        // サイドバー
        Gtk::Widget* m_sidebar;

        // (縦/横) 3ペーンモード時の右側ペーン
        SKELETON::JDVPaned m_vpaned_r; 
        SKELETON::JDHPaned m_hpaned_r;

        // 右ペーンで使用するwidget
        SKELETON::JDVBox m_vbox_article;
        SKELETON::JDVBox m_vbox_board;
        SKELETON::JDVBox m_vbox_articleboard;
        SKELETON::JDNotebook m_notebook;
        bool m_imagetab_shown;
        SKELETON::JDVPaned m_vpaned_message; // 埋め込み書き込みビュー用

        // ツールバー
        Gtk::ScrolledWindow m_toolbar;
        Gtk::Tooltips m_tooltip;
        Gtk::HBox m_toolbar_vbox;
        Gtk::Entry m_entry_url;
        SKELETON::ImgButton m_button_go;
        SKELETON::ImgButton m_button_search_cache;
        SKELETON::ImgToggleButton m_button_bbslist;
        SKELETON::ImgToggleButton m_button_favorite;
        SKELETON::ImgToggleButton m_button_board;
        SKELETON::ImgToggleButton m_button_thread;
        SKELETON::ImgToggleButton m_button_image;
        Gtk::VSeparator m_vspr_toolbar_1;
        Gtk::VSeparator m_vspr_toolbar_2;

        // タイトルに表示する文字列
        // set_maintitle() 参照
        std::string m_title;

        // ステータスバー
#if GTKMMVER <= 240
        Gtk::Statusbar m_statbar;
#else
        Gtk::HBox m_statbar;
        Gtk::Label m_label_stat;
#endif
        Gtk::Label m_mginfo;
        Gtk::ScrolledWindow m_stat_scrbar;

        Gtk::MenuBar* m_menubar;

        Glib::RefPtr< Gtk::ActionGroup > m_action_group;
        Glib::RefPtr< Gtk::UIManager > m_ui_manager;
        HistoryMenuThread* m_histmenu_thread;
        HistoryMenuBoard* m_histmenu_board;
        bool m_enable_menuslot;

        // 起動中
        bool m_boot;

        // 初期設定中
        bool m_init;

    public:

        Core( WinMain& win_main );
        virtual ~Core();

        Gtk::Widget* get_toplevel();

        // init = true なら初回起動
        void run( bool init );        

        void set_command( const COMMAND_ARGS& command );

        // SIGHUPを受け取った時の処理
        void shutdown();

    private:

        void pack_widget( bool unpack );
        void create_toolbar();

        // 初回起動時のセットアップ
        void first_setup();
        void show_setupdiag( const std::string& msg );

        void set_maintitle();

        void slot_activate_menubar();

        void slot_toggle_save_postlog();
        void slot_toggle_strict_charwidth();
        void slot_toggle_use_mosaic();
        void slot_delete_all_images();

        void slot_changefont_tree();
        void slot_changefont_tree_board();
        void slot_changefont_main();
        void slot_changefont_popup();
        void slot_changefont_message();

        void slot_changecolor_char();
        void slot_changecolor_separator();
        void slot_changecolor_back();
        void slot_changecolor_back_popup();
        void slot_changecolor_back_tree();

        bool open_color_diag( std::string title, const int* rgb, int* rgb_out );

        void slot_setup_proxy();
        void slot_setup_passwd();
        void slot_setup_browser();
        void slot_setup_abone();
        void slot_setup_abone_thread();
        void slot_toggle_abone_transp_chain();

        void slot_show_hp();
        void slot_show_bbs();
        void slot_show_old2ch();
        void slot_show_manual();
        void slot_show_about();
        void slot_quit();
        void slot_reload_list();
        void slot_save_favorite();
        void slot_toggle_online();
        void slot_toggle_login2ch();
        void slot_toggle_toolbar();
        void slot_toggle_sidebar();
        void slot_search_cache();
        void slot_show_hide_leftpane( bool show );
        void slot_toggle_toolbarpos( int pos );
        void slot_toggle_2pane();
        void slot_toggle_3pane();
        void slot_toggle_v3pane();
        void slot_toggle_embedded_mes();
        void slot_toggle_embedded_img();
        void slot_toggle_oldarticle();
        void slot_toggle_tabbutton();
        void slot_toggle_popupwarpmode();
        void slot_toggle_emacsmode();
        void slot_toggle_restore_views();

        // coreが自前でするコマンド処理
        void exec_command();

        // 起動完了直後に実行する処理
        void exec_command_after_boot();

        // フォーカス回復
        void restore_focus( bool force );

        // メインタイマー
        bool slot_timeout( int timer_number );

        // 右ペーンのnotebookのタブの切替え
        void slot_switch_page( GtkNotebookPage*, guint page );

        // 右ペーンのnotebookのページ番号
        int get_right_current_page();

        // 右ペーンのnotebookのページをセット
        void set_right_current_page( int page );

        bool slot_focus_out_event( GdkEventFocus* ev );
        bool slot_focus_in_event( GdkEventFocus* ev );
        bool slot_window_state_event( GdkEventWindowState* event );
        void slot_active_url();

        // あるadminがemptyになったので他のadminにスイッチ
        void empty_page( const std::string& url );

        // あるadminののnotebookのページがスイッチした
        void switch_page( const std::string& url );

        void set_toggle_view_button();
        void set_sensitive_view_button();
        void toggle_maximize_rightpane();

        void switch_article();
        void switch_board();
        void switch_bbslist();
        void switch_favorite();
        void switch_sidebar( const std::string& url = std::string() );
        void switch_image();
        void switch_message();
        void toggle_article();
        void switch_leftview();
        void switch_rightview();
        void open_by_browser( const std::string& url );

        void set_history_article( const std::string& url );
        void set_history_board( const std::string& url );

        // 画像インジケータ表示/非表示
        void show_imagetab();
        void hide_imagetab();
    };

    Core* get_instance();
}


#endif
