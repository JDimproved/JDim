// ライセンス: 最新のGPL

// Viewを管理するクラス
// 派生クラスはcreate_view()をオーバロードすること
// Adminへの命令はすべてset_command()を通じておこなうこと

#ifndef _ADMIN_H
#define _ADMIN_H

#include <gtkmm.h>
#include <string>
#include <list>

#include "dragnote.h"

struct COMMAND_ARGS;

namespace SKELETON
{
    class View;

    class Admin
    {
        std::string m_url;
        DragableNoteBook m_notebook;

        bool m_focus;

        bool m_adjust_reserve; // adjust予約
        int m_pre_width;

        Glib::Dispatcher m_disp;
        std::list< COMMAND_ARGS > m_list_command;

        // 右クリックメニュー用
        Glib::RefPtr< Gtk::ActionGroup > m_action_group;
        Glib::RefPtr< Gtk::UIManager > m_ui_manager;
        int m_clicked_page;

    public:

        Admin( const std::string& url );
        virtual ~Admin();

        virtual bool empty();
        const std::string& get_url() const{ return m_url; }
        Gtk::Notebook& get_gtknotebook(){ return m_notebook; }

        // フォーカスされているか
        const bool has_focus() const { return m_focus; }

        // 含まれているページのURLのリスト取得
        virtual std::list< std::string > get_URLs();

        // Core からのクロック入力。
        // Coreでタイマーをひとつ動かして全体の同期を取るようにしているので
        // 一定時間毎に clock_in() が Core から呼び出される
        virtual void clock_in();

        // コマンド入力
        virtual void set_command( const std::string& command,
                                  const std::string& url = std::string(),
                                  const std::string& arg1 = std::string(),
                                  const std::string& arg2 = std::string(),
                                  const std::string& arg3 = std::string(),
                                  const std::string& arg4 = std::string(),
                                  const std::string& arg5 = std::string(),
                                  const std::string& arg6 = std::string()
            );

        // 現在表示してるページ番号
        // 表示ページを指定したいときは "set_page" コマンドを使う
        virtual int get_current_page();

        // SIGHUPを受け取ったときの処理
        virtual void shutdown();

    protected:

        DragableNoteBook& get_notebook(){ return m_notebook; }

        virtual void exec_command();

        // 派生クラス固有のコマンドを実行
        virtual void command_local( const COMMAND_ARGS& command ){}

        virtual void restore(){} // 起動時に前回の状態を回復
        virtual void open_view( const COMMAND_ARGS& command );
        virtual void switch_admin(){} // CORE::core_set_command( "switch_*" )　を送る
        virtual void switch_view( const std::string& url );
        virtual void tab_left();
        virtual void tab_right();
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
        virtual void focus_view( int page );
        virtual void focus_current_view();
        virtual void restore_focus();
        virtual void focus_out();
        virtual void set_tablabel( const std::string& url, const std::string& str_label, bool fix );
        virtual void adjust_tabwidth( bool force );
        virtual void set_autoreload_mode( const std::string& url, int mode, int sec );

        // D&D
        // 派生クラス別にD&Dの処理を行う
        virtual void slot_drag_begin( int page ){}
        virtual void slot_drag_motion(){}
        virtual void slot_drag_drop( int page ){}
        virtual void slot_drag_end(){}

        virtual void open_list( const std::string& str_list ){};
        virtual View* create_view( const COMMAND_ARGS& command ){ return NULL; };
        virtual View* get_view( const std::string& url, bool use_find = false );
        std::list< View* > get_list_view( const std::string& url );
        std::list< View* > get_list_view();
        virtual View* get_current_view();
        virtual void set_current_page( int page );

        // notebookのタブが切り替わったときに呼ばれるslot
        virtual void slot_switch_page( GtkNotebookPage*, guint page );

        // タブを閉じる
        virtual void slot_tab_close( int page );

        // タブ再読み込み
        virtual void slot_tab_reload( int page );

        // タブメニュー表示
        virtual void slot_tab_menu( int page, int x, int y );

        // 右クリックメニュー
        virtual void slot_close_tab();
        virtual void slot_close_other_tabs();
        virtual void slot_close_all_tabs();
        virtual void slot_open_by_browser();
        virtual void slot_copy_url();
    };
}

#endif
