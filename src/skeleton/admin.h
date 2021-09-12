// ライセンス: GPL2

// Viewを管理するクラス
// 派生クラスはcreate_view()をオーバロードすること
// Adminへの命令はすべてset_command()を通じておこなうこと

#ifndef _ADMIN_H
#define _ADMIN_H

#include "dispatchable.h"

#include "command_args.h"

#include <gtkmm.h>

#include <list>
#include <memory>
#include <string>


namespace SKELETON
{
    class JDWindow;
    class View;
    class DragableNoteBook;
    class TabSwitchMenu;

    class Admin : public Dispatchable
    {
        std::string m_url;

        std::unique_ptr<JDWindow> m_win;

    protected:
        std::unique_ptr<DragableNoteBook> m_notebook;

    private:
        bool m_focus{};

        std::list< COMMAND_ARGS > m_list_command;

        // 右クリックメニュー用
        Glib::RefPtr< Gtk::ActionGroup > m_action_group;
        Glib::RefPtr< Gtk::UIManager > m_ui_manager;
        int m_clicked_page;

        // 移動サブメニュー
        Gtk::MenuItem* m_move_menuitem;
        std::unique_ptr<TabSwitchMenu> m_move_menu;

        // タブ切り替えメニュー
        std::unique_ptr<TabSwitchMenu> m_tabswitchmenu;

        // view履歴使用
        bool m_use_viewhistory{};
        std::string m_last_closed_url;

        // タブの切り替え履歴
        bool m_use_switchhistory{};
        std::list< std::string > m_list_switchhistory;

    public:

        explicit Admin( const std::string& url );
        ~Admin();

        // コピー禁止
        Admin( const Admin& ) = delete;
        Admin& operator=( const Admin& ) = delete;

        virtual void save_session();

        void setup_menu();

        virtual bool empty() const;
        const std::string& get_url() const{ return m_url; }
        virtual Gtk::Widget* get_widget();
        virtual Gtk::Window* get_win();

        // 起動中
        bool is_booting();

        // フォーカスされているか
        virtual bool has_focus() const { return m_focus; }

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
                          const std::string& arg6 = std::string(),
                          const std::string& arg7 = std::string(),
                          const std::string& arg8 = std::string()
            );

        // コマンド入力(即時実行)
        void set_command_immediately( const std::string& command,
                          const std::string& url = std::string(),
                          const std::string& arg1 = std::string(),
                          const std::string& arg2 = std::string(),
                          const std::string& arg3 = std::string(),
                          const std::string& arg4 = std::string(),
                          const std::string& arg5 = std::string(),
                          const std::string& arg6 = std::string(),
                          const std::string& arg7 = std::string(),
                          const std::string& arg8 = std::string()
            );

        // コマンドがセットされているか
        bool has_commands() const { return m_list_command.size(); }

        // 現在表示してるページ番号およびURL
        // 表示ページを指定したいときは "set_page" コマンドを使う
        virtual int get_current_page();
        std::string get_current_url();

        // urlで指定されるタブがロックされているか
        bool is_locked( const std::string& url );

        // urlで指定されるタブが存在するか
        bool exist_tab( const std::string& url );

        // 指定したページに表示切替え
        void set_current_page( const int page );

        // フォーカスしてから指定したページに表示切替え
        void set_current_page_focus( const int page );

        virtual View* get_current_view();

    protected:

        void set_use_viewhistory( const bool use ){ m_use_viewhistory = use; }
        bool get_use_viewhistory() const { return m_use_viewhistory; }

        void set_use_switchhistory( const bool use ){ m_use_switchhistory = use; }

        JDWindow* get_jdwin(){ return m_win.get(); }
        void set_jdwin( std::unique_ptr<JDWindow> win );
        void delete_jdwin();

        // URLやステータスを更新
        void update_status( View* view, const bool force );

        DragableNoteBook* get_notebook(){ return m_notebook.get(); }

        // コマンド入力
        // immediately = true のときディスパッチャを呼ばずにすぐさま実行
        void set_command_impl( const bool immediately, const COMMAND_ARGS& command_arg );

        void callback_dispatch() override;

        // admin共通コマンド実行
        virtual void exec_command();

        // 派生クラス固有のコマンドを実行
        virtual void command_local( const COMMAND_ARGS& command ) = 0;

        // 起動時に前回の状態を回復
        virtual void restore( const bool only_locked ) = 0;

        // url から Viewを開くための COMMAND_ARGS を取得する
        // restore() などで使用する
        virtual COMMAND_ARGS url_to_openarg( const std::string& url, const bool tab, const bool lock ) = 0; 

        // COMMAND_ARGS からビューの URL を取得する
        virtual std::string command_to_url( const COMMAND_ARGS& command ){ return command.url; }

        // view_modeに該当するページを探す
        virtual int find_view( const std::string& view_mode ){ return -1; };

        virtual void open_view( const COMMAND_ARGS& command );
        virtual void switch_admin() = 0;  // CORE::core_set_command( "switch_*" )　を送る
        virtual void switch_view( const std::string& url );
        void reload_view( const std::string& url );

        // タブ左右移動
        // updated == true の時は更新されたタブに移動
        virtual void tab_left( const bool updated );
        virtual void tab_right( const bool updated );

        virtual void tab_num( const std::string& str_num );
        virtual void tab_head();
        void tab_head_focus();
        virtual void tab_tail();
        void tab_tail_focus();
        virtual void redraw_view( const std::string& url );
        virtual void redraw_current_view();
        virtual void relayout_current_view();
        virtual void redraw_views( const std::string& url );
        virtual void update_view( const std::string& url );
        virtual void update_item( const std::string& url,  const std::string& id );
        virtual void update_finish( const std::string& url );        
        virtual void close_view( const std::string& url );
        virtual void unlock_all_view( const std::string& url );
        virtual void close_all_view( const std::string& url );
        virtual void close_other_views( const std::string& url );
        virtual void close_current_view();
        virtual void restore_lasttab(){}
        virtual void set_title( const std::string& url, const std::string& title, const bool force );
        virtual void set_url( const std::string& url, const std::string& url_show, const bool force );
        virtual void set_status( const std::string& url, const std::string& stat, const bool force );
        void set_status_color( const std::string& url, const std::string& color, const bool force );
        virtual void focus_view( int page );
        virtual void focus_current_view();
        virtual void restore_focus();
        virtual void focus_out();
        virtual void set_tablabel( const std::string& url, const std::string& str_label );
        virtual void relayout_all();
        virtual void open_window(){}
        virtual void close_window(){}
        virtual void toggle_tab();
        virtual void toggle_icon( const std::string& url );

        // オートリロードのモード設定
        virtual bool set_autoreload_mode( const std::string& url, int mode, int sec );

        // ポップアップを隠す(インスタンスは削除しない)
        void hide_popup();
        
        // タブをお気に入りにドロップした時にお気に入りがデータ送信を要求してきた
        virtual void slot_drag_data_get( Gtk::SelectionData& selection_data, const int page ) = 0;

        void open_list( const COMMAND_ARGS& command_list );
        virtual COMMAND_ARGS get_open_list_args( const std::string& url, const COMMAND_ARGS& command_list ){ return COMMAND_ARGS(); }
        virtual View* create_view( const COMMAND_ARGS& command ){ return nullptr; };
        virtual View* get_view( const std::string& url );

        // url を含むビュークラスをリストで取得 ( url と一致するビューでは無い )
        std::list< View* > get_list_view( const std::string& url );

        // 全てのビュークラスをリストで取得
        std::list< View* > get_list_view();

        // ツールバー
        virtual void show_toolbar(){}
        virtual void toggle_toolbar(){}
        void focus_toolbar_search();
        void redraw_toolbar();
        void update_toolbar_button();
        virtual void open_searchbar(){}
        virtual void close_searchbar(){}

        // タブの更新チェック
        void check_update_all_tabs( const bool open );
        void check_update_all_tabs( const int from_page, const bool open );

        // タブの再読み込み
        void reload_all_tabs();
        void reload_all_tabs( const int from_page );

        // notebookのタブが切り替わったときに呼ばれるslot
        void slot_switch_page( Gtk::Widget*, guint page );

        // タブをクリックした
        void slot_tab_clicked( const int page );

        // タブの上でホイールを回した
        void slot_tab_scrolled( GdkEventScroll* event );

        // タブを閉じる
        void slot_tab_close( const int page );

        // タブ再読み込み
        void slot_tab_reload( const int page );

        // タブメニュー表示
        virtual void slot_tab_menu( int page, int x, int y );

        // タブ切り替えメニュー表示
        void slot_show_tabswitchmenu();

        // 右クリックメニュー
        virtual void slot_close_tab();
        virtual void slot_lock();
        virtual void slot_close_other_tabs();
        virtual void slot_close_left_tabs();
        virtual void slot_close_right_tabs();
        virtual void slot_close_all_tabs();
        virtual void slot_close_same_icon_tabs();
        virtual void slot_check_update_all_tabs();
        virtual void slot_check_update_reload_all_tabs();
        virtual void slot_reload_all_tabs();
        virtual void slot_cancel_reload_all_tabs();
        virtual void slot_open_by_browser();
        virtual void slot_copy_url();
        virtual void slot_copy_title_url();

        // ページがロックされているかリストで取得
        virtual std::list< bool > get_locked();

        // タブのロック/アンロック
        virtual bool is_lockable( const int page );
        virtual bool is_locked( const int page );
        virtual void lock( const int page );
        virtual void unlock( const int page );

        // プロパティ表示
        void show_preference();

        // View履歴戻る / 進む
        bool back_viewhistory( const std::string& url, const int count );
        void back_clicked_viewhistory( const int count );

        bool forward_viewhistory( const std::string& url, const int count );
        void forward_clicked_viewhistory( const int count );

        // View履歴削除
        void clear_viewhistory();

        // タブの切り替え履歴
        const std::list< std::string >& get_switchhistory() const { return m_list_switchhistory; }
        void set_switchhistory( const std::list< std::string >& hist ){ m_list_switchhistory = hist; }

      private:

        void slot_popupmenu_deactivate();

        bool back_forward_viewhistory( const std::string& url, const bool back, const int count );

        // 移転などでviewのurlを更新
        void update_url( const std::string& url_old, const std::string& url_new );

        // urlを含むviewの板名を更新
        void update_boardname( const std::string& url );

        // タブの切り替え履歴を更新
        void append_switchhistory( const std::string& url );
        void remove_switchhistory( const std::string& url );
        std::string get_valid_switchhistory();

        void close_view( View* view );
    };
}

#endif
