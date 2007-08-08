// ライセンス: GPL2

// Viewを管理するクラス
// 派生クラスはcreate_view()をオーバロードすること
// Adminへの命令はすべてset_command()を通じておこなうこと

#ifndef _ADMIN_H
#define _ADMIN_H

#include "dispatchable.h"

#include <gtkmm.h>
#include <string>
#include <list>

#include "command_args.h"

namespace SKELETON
{
    class JDWindow;
    class View;
    class DragableNoteBook;

    class Admin : public Dispatchable
    {
        std::string m_url;

        JDWindow * m_win;
        DragableNoteBook* m_notebook;

        bool m_focus;

        std::list< COMMAND_ARGS > m_list_command;

        // 右クリックメニュー用
        Glib::RefPtr< Gtk::ActionGroup > m_action_group;
        Glib::RefPtr< Gtk::UIManager > m_ui_manager;
        int m_clicked_page;

        // 移動サブメニュー用
        Gtk::Menu* m_move_menu;
        std::vector< Gtk::MenuItem* > m_vec_movemenu_items;
        std::vector< bool > m_vec_movemenu_append;

    public:

        Admin( const std::string& url );
        virtual ~Admin();

        void setup_menu( const bool enable_checkupdate );

        virtual bool empty();
        const std::string& get_url() const{ return m_url; }
        virtual Gtk::Widget* get_widget();
        virtual Gtk::Window* get_win();

        // 起動中
        const bool is_booting();

        // フォーカスされているか
        virtual const bool has_focus() const { return m_focus; }

        // タブの数
        virtual int get_tab_nums();

        // 含まれているページのURLのリスト取得
        virtual std::list< std::string > get_URLs();

        // Core からのクロック入力。
        // Coreでタイマーをひとつ動かして全体の同期を取るようにしているので
        // 一定時間毎に clock_in() が Core から呼び出される
        virtual void clock_in();

        // コマンド入力(通常)
        void set_command( const std::string& command,
                          const std::string& url = std::string(),
                          const std::string& arg1 = std::string(),
                          const std::string& arg2 = std::string(),
                          const std::string& arg3 = std::string(),
                          const std::string& arg4 = std::string(),
                          const std::string& arg5 = std::string(),
                          const std::string& arg6 = std::string()
            );

        // コマンド入力(即時実行)
        void set_command_immediately( const std::string& command,
                          const std::string& url = std::string(),
                          const std::string& arg1 = std::string(),
                          const std::string& arg2 = std::string(),
                          const std::string& arg3 = std::string(),
                          const std::string& arg4 = std::string(),
                          const std::string& arg5 = std::string(),
                          const std::string& arg6 = std::string()
            );

        // コマンドがセットされているか
        const bool has_commands() const { return ( m_list_command.size() ); }

        // 現在表示してるページ番号およびURL
        // 表示ページを指定したいときは "set_page" コマンドを使う
        virtual int get_current_page();
        std::string get_current_url();

        // SIGHUPを受け取ったときの処理
        virtual void shutdown();

    protected:

        JDWindow* get_jdwin(){ return m_win; }
        void set_jdwin( JDWindow* win ){ m_win = win; }
        void delete_jdwin();

        // 移転などでviewのホスト名を更新
        void update_host( const std::string& oldhost, const std::string& newhost );

        // URLやステータスを更新
        void update_status( View* view, const bool force );

        DragableNoteBook* get_notebook(){ return m_notebook; }

        // コマンド入力
        // immediately = true のときディスパッチャを呼ばずにすぐさま実行
        void set_command_impl( const bool immediately,
                          const std::string& command,
                          const std::string& url = std::string(),
                          const std::string& arg1 = std::string(),
                          const std::string& arg2 = std::string(),
                          const std::string& arg3 = std::string(),
                          const std::string& arg4 = std::string(),
                          const std::string& arg5 = std::string(),
                          const std::string& arg6 = std::string()
            );

        virtual void callback_dispatch();

        // admin共通コマンド実行
        virtual void exec_command();

        // 派生クラス固有のコマンドを実行
        virtual void command_local( const COMMAND_ARGS& command ){}

        virtual void restore(){} // 起動時に前回の状態を回復
        virtual void open_view( const COMMAND_ARGS& command );
        virtual void switch_admin(){} // CORE::core_set_command( "switch_*" )　を送る
        virtual void switch_view( const std::string& url );
        virtual void tab_left();
        virtual void tab_right();
        virtual void tab_head();
        virtual void tab_tail();
        virtual void redraw_view( const std::string& url );
        virtual void redraw_current_view();
        virtual void redraw_views( const std::string& url );
        virtual void update_view( const std::string& url );
        virtual void update_item( const std::string& url,  const std::string& id );
        virtual void update_finish( const std::string& url );        
        virtual void close_view( const std::string& url );
        virtual void close_view( View* view );
        virtual void close_all_view( const std::string& url );
        virtual void close_current_view();
        virtual void set_title( const std::string& url, const std::string& title, const bool force );
        virtual void set_url( const std::string& url, const std::string& url_show, const bool force );
        virtual void set_status( const std::string& url, const std::string& stat, const bool force );
        virtual void focus_view( int page );
        virtual void focus_current_view();
        virtual void restore_focus();
        virtual void focus_out();
        virtual void set_tablabel( const std::string& url, const std::string& str_label );
        virtual void relayout_all();
        virtual void open_window(){}
        virtual void close_window(){}
        virtual void toggle_toolbar();
        virtual void toggle_icon( const std::string& url );

        // オートリロードのモード設定
        virtual bool set_autoreload_mode( const std::string& url, int mode, int sec );

        // D&D
        // 派生クラス別にD&Dの処理を行う
        virtual void slot_drag_begin( int page ){}
        virtual void slot_drag_end(){}

        void open_list( const std::string& str_list );
        virtual COMMAND_ARGS get_open_list_args( const std::string& url ){ return COMMAND_ARGS(); }
        virtual View* create_view( const COMMAND_ARGS& command ){ return NULL; };
        virtual View* get_view( const std::string& url );
        std::list< View* > get_list_view( const std::string& url );
        std::list< View* > get_list_view();
        virtual View* get_current_view();

        // タブの更新チェック
        void check_update_all_tabs( const int from_page );

        // タブの再読み込み
        void reload_all_tabs();
        void reload_all_tabs( const int from_page );

        // 指定したページに表示切替え
        virtual void set_current_page( int page );

        // notebookのタブが切り替わったときに呼ばれるslot
        void slot_switch_page( GtkNotebookPage*, guint page );

        // タブをクリック
        virtual void slot_tab_click( int page );

        // タブを閉じる
        virtual void slot_tab_close( int page );

        // タブ再読み込み
        virtual void slot_tab_reload( int page );

        // タブメニュー表示
        virtual void slot_tab_menu( int page, int x, int y );

        // 右クリックメニュー
        virtual void slot_close_tab();
        virtual void slot_close_other_tabs();
        virtual void slot_close_left_tabs();
        virtual void slot_close_right_tabs();
        virtual void slot_close_all_tabs();
        virtual void slot_check_update_all_tabs();
        virtual void slot_check_update_reload_all_tabs();
        virtual void slot_reload_all_tabs();
        virtual void slot_cancel_reload_all_tabs();
        virtual void slot_open_by_browser();
        virtual void slot_copy_url();
        virtual void slot_copy_title_url();
    };
}

#endif
