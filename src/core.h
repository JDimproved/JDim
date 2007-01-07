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
        
        Gtk::VBox m_vbox_main;
        SKELETON::JDHPaned m_hpaned;
        Gtk::VPaned m_vpaned; // 3ペーンモード時の右側ペーン
        Gtk::HPaned m_hpaned_r; // 縦3ペーンモードの時の右側ペーン
        Gtk::VBox m_vbox;
        Gtk::Notebook m_notebook;
        bool m_imagetab_shown;

        Gtk::Tooltips m_tooltip;

        Gtk::ScrolledWindow m_urlbar;
        Gtk::HBox m_urlbar_vbox;
        Gtk::Entry m_entry_url;
        SKELETON::ImgButton m_button_go;
        SKELETON::ImgButton m_button_search_cache;
        SKELETON::ImgButton m_button_sidebar;

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

        Glib::RefPtr< Gtk::ActionGroup > m_action_group;
        Glib::RefPtr< Gtk::UIManager > m_ui_manager;
        HistoryMenuThread* m_histmenu_thread;
        HistoryMenuBoard* m_histmenu_board;
        bool m_enable_menuslot;

        // 起動中
        bool m_boot;

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
        void slot_toggle_urlbar();
        void slot_toggle_sidebar();
        void slot_search_cache();
        void slot_show_hide_leftpane( bool show );
        void slot_toggle_2pane();
        void slot_toggle_3pane();
        void slot_toggle_v3pane();
        void slot_toggle_oldarticle();
        void slot_toggle_tabbutton();
        void slot_toggle_popupwarpmode();
        void slot_toggle_restore_views();

        // coreが自前でするコマンド処理
        void exec_command();

        // 起動完了直後に実行する処理
        void exec_command_after_boot();

        bool slot_timeout( int timer_number );
        void slot_switch_page( GtkNotebookPage*, guint page );
        bool slot_focus_out_event( GdkEventFocus* ev );
        bool slot_focus_in_event( GdkEventFocus* ev );
        void slot_active_url();

        // あるadminがemptyになったので他のadminにスイッチ
        void empty_page( const std::string& url );

        // あるadminののnotebookのページがスイッチした
        void switch_page( const std::string& url );

        void switch_article();
        void switch_board();
        void switch_bbslist();
        void switch_image();
        void toggle_article();
        void switch_leftview();
        void switch_rightview();
        void open_by_browser( const std::string& url );

        void set_history_article( const std::string& url );
        void set_history_board( const std::string& url );
    };

    Core* get_instance();
}


#endif
