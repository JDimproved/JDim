// ライセンス: GPL2

#ifndef _VIEW_H
#define _VIEW_H

#include <gtkmm.h>
#include <string>
#include <set>

#include "control/control.h"

namespace SKELETON
{
    class Admin;

    // 自分がポップアップviewの時に(ポップアップウィンドウ( SKELETON::PopupWin ) 経由で)
    // 親widgetにhideを依頼するシグナル。PopupWin::PopupWin()でPopupWin::slot_hide_popup()にコネクトされる。
    typedef sigc::signal< void > SIG_HIDE_POPUP;    

    // 自分がポップアップviewでリサイズしたときに、明示的にポップアップウィンドウ( SKELETON::PopupWin )
    // にリサイズを依頼するシグナル。PopupWin::PopupWin()でPopupWin::slot_resize_popup()にコネクトされる。
    typedef sigc::signal< void > SIG_RESIZE_POPUP;
    
    class View : public Gtk::VBox
    {
        SIG_HIDE_POPUP m_sig_hide_popup;
        SIG_RESIZE_POPUP m_sig_resize_popup;

        std::string m_url;
        Gtk::Window* m_parent_win;

        // クライアント領域の幅、高さ
        int m_width_client;
        int m_height_client;

        // 入力コントローラ
        CONTROL::Control m_control;

        // ポップアップメニュー
        Glib::RefPtr< Gtk::ActionGroup > m_action_group;
        Glib::RefPtr< Gtk::UIManager > m_ui_manager;
        std::set< std::string > m_url_popup;

        // ツールバーに表示する文字列
        std::string m_label;

        // メインウィンドウのタイトルに表示する文字
        std::string m_title;

        // メインウィンドウのステータスバーに表示する文字
        std::string m_status;

        // true ならマウスジェスチャ使用
        bool m_enable_mg; 

        // オートリロード
        bool m_enable_autoreload; // true ならオートリロード可能(デフォルト:off)
        int m_autoreload_mode; // モード
        int m_autoreload_sec; // 何秒おきにリロードするか
        int m_autoreload_counter; // オートリロード用のカウンタ

        // キーボード数字入力ジャンプ用
        int m_keyjump_counter;
        int m_keyjump_num;

        // ロック可能か
        bool m_lockable;

        // ロック状態
        bool m_locked;

        // 書き込み可能か
        bool m_writeable;

        // ツールバーのID
        int m_id_toolbar;

        // 検索文字列
        std::string m_search_query;

        // ポップアップ時に全ての領域を表示できないならカーソルの上に表示
        bool m_popup_upside;

        // ロード時にキャッシュを削除してからviewを再読み込みする
        bool m_reget;

      protected:

        // url_new に URL を変更
        void set_url( const std::string& url_new );

        // Viewが所属するAdminクラス
        virtual Admin* get_admin() = 0;

        // UI
        Glib::RefPtr< Gtk::ActionGroup >& action_group(){ return m_action_group; }
        Glib::RefPtr< Gtk::UIManager >& ui_manager(){ return m_ui_manager; }

        // コントローラ
        CONTROL::Control& get_control(){ return m_control; }

        // ツールバーに表示するラベル
        void set_label( const std::string& label ){ m_label = label; }

        // メインウィンドウのタイトルに表示する文字列
        void set_title( const std::string& title ){ m_title = title; }

        // メインウィンドウのステータスバーに表示する文字
        void set_status( const std::string& status ){ m_status = status; }

        // マウスジェスチャ
        void set_enable_mg( bool mg ){ m_enable_mg = mg; }
        bool enable_mg() const { return m_enable_mg; }

        // オートリロードのカウンタをインクリメント
        // 指定秒数を越えたら true を返す
        bool inc_autoreload_counter();

        // オートリロード可能/不可能切替え
        void set_enable_autoreload( bool autoreload ){ m_enable_autoreload = autoreload; }

        // オートリロードのカウンタをリセット
        void reset_autoreload_counter(); 

        // オートリロード間隔設定
        int get_autoreload_sec() const { return m_autoreload_sec; }
        void set_autoreload_sec( const int sec ){ m_autoreload_sec = sec; }

        // オートリロード用のカウンタ
        int get_autoreload_counter() const { return m_autoreload_counter; }
        void set_autoreload_counter( const int counter ) { m_autoreload_counter = counter; }

        // 数字入力ジャンプカウンタのインクリメント
        // 指定秒数を越えたら true を返す
        bool inc_keyjump_counter();

        // 数字入力ジャンプカウンタのリセット
        void reset_keyjump_counter(); 

        // 数字入力ジャンプ用に sig_key_press() から呼び出す
        bool release_keyjump_key( int key );

        // ポップアップメニュー表示
        void show_popupmenu( const std::string& url, bool use_slot = false );

        // ポップアップメニュー表示時に表示位置を決めるスロット
        void slot_popup_menu_position( int& x, int& y, bool& push_in );

        // ポップアップメニューがmapした時に呼び出されるスロット
        void slot_map_popupmenu();

        // ポップアップメニューがhideした時に呼び出されるスロット
        void slot_hide_popupmenu();

        // ポップアップメニューを表示する前にメニューのアクティブ状態を切り替える
        virtual void activate_act_before_popupmenu( const std::string& url ){}

        //  ポップアップメニュー取得
        virtual Gtk::Menu* get_popupmenu( const std::string& url ){ return NULL; }

    public:

        SIG_HIDE_POPUP sig_hide_popup(){ return m_sig_hide_popup; }
        SIG_RESIZE_POPUP sig_resize_popup(){ return m_sig_resize_popup; }
        
        View( const std::string& url, const std::string& arg1 = std::string(), const std::string& arg2 = std::string() );
        ~View() noexcept {}

        virtual void save_session() = 0;

        virtual const std::string& get_url(){ return m_url; }
        const std::string& get_url_admin();

        virtual void set_parent_win( Gtk::Window* parent_win ){ m_parent_win = parent_win; }
        virtual Gtk::Window* get_parent_win(){ return m_parent_win; }

        // 移転があったときなどにadminから呼び出される
        virtual void update_url( const std::string& url_old, const std::string& url_new );

        // 検索文字列
        const std::string& get_search_query(){ return m_search_query; }

        // ツールバーのID
        // タブの切り替えのときに Admin から参照される
        // コンストラクタであらかじめ指定しておくこと
        void set_id_toolbar( const int id ) { m_id_toolbar = id; }
        int get_id_toolbar() const { return m_id_toolbar; }

        // ロック/アンロック
        bool is_lockable() const { return m_lockable; }
        void set_lockable( const bool lockable ){ m_lockable = lockable; }
        bool is_locked() const { return m_locked; }
        virtual void lock(){ m_locked = true; }
        virtual void unlock(){ m_locked = false; }

        // 書き込み可能/不可能
        bool is_writeable() const { return m_writeable; }
        void set_writeable( const bool writeable ){ m_writeable = writeable; }

        // ポップアップ時に全ての領域を表示できないならカーソルの上に表示
        bool get_popup_upside() const { return m_popup_upside; }
        void set_popup_upside( const bool upside ){ m_popup_upside = upside; }

        // view 上にマウスポインタがあれば true
        bool is_mouse_on_view();

        // 各view個別のコマンド
        virtual bool set_command( const std::string& command,
                                  const std::string& arg1 = {},
                                  const std::string& arg2 = {} ) { return true; }

        // コピー用のURL
        virtual std::string url_for_copy(){ return m_url; }

        // ツールバーのラベルに表示する文字列
        const std::string& get_label(){ return m_label; }

        // メインウィンドウのタイトルバーに表示する文字列
        virtual const std::string& get_title(){ return m_title; }

        // メインウィンドウのステータスバーに表示する文字列
        virtual const std::string& get_status(){ return m_status; }

        // クライアント領域の幅、高さ
        virtual int width_client(){ return m_width_client; }
        virtual int height_client(){ return m_height_client; }
        void set_width_client( int val ){ m_width_client = val; }
        void set_height_client( int val ){ m_height_client = val; }

        // オートリロード可能か
        bool get_enable_autoreload() const { return m_enable_autoreload; }

        // オートリロードのモード設定
        void set_autoreload_mode( int mode, int sec );

        // 現在のオートリロードのモード取得
        int get_autoreload_mode() const { return m_autoreload_mode; }

        // ロード時にキャッシュを削除してからviewを再読み込みする
        bool get_reget() const{ return m_reget; }
        void set_reget( const bool reget ){ m_reget = reget; }

        // アイコンのID取得
        virtual int get_icon( const std::string& iconname ){ return -1; }

        // ロード中
        virtual bool is_loading() const { return false; }

        // 更新した
        virtual bool is_updated(){ return false;}

        // 更新チェックして更新可能か
        virtual bool is_check_update(){ return false;}

        // 古いデータか
        virtual bool is_old(){ return false;}

        // 壊れているか
        virtual bool is_broken(){ return false; }

        // ラベルやステータスバーの色
        std::string get_color();

        // キーを押した
        virtual bool slot_key_press( GdkEventKey* event ){ return false; }

        // クロック入力
        // clock_in()はビューがアクティブのときに呼び出される
        // clock_in_always()はviewの種類に依らず常に呼び出されるので重い処理を含めてはいけない
        virtual void clock_in(){};
        virtual void clock_in_always();

        virtual void write(){}
        virtual void reload(){}
        virtual void stop(){}
        virtual void show_view(){}
        virtual void redraw_view(){}
        virtual void redraw_scrollbar(){}
        virtual void relayout(){}
        virtual void update_view(){}
        virtual void update_finish(){}        
        virtual void focus_view(){}
        virtual void focus_out(){}
        virtual void close_view(){}
        virtual void delete_view(){}
        virtual void set_favorite(){}
        virtual void update_item( const std::string& url, const std::string& id ){}
        virtual bool operate_view( const int ) = 0;
        virtual void goto_top(){}
        virtual void goto_bottom(){}
        virtual void goto_num( const int num_to, const int num_from ){}
        virtual void scroll_up(){}
        virtual void scroll_down(){}
        virtual void scroll_left(){}
        virtual void scroll_right(){}
        virtual void show_preference(){}
        virtual void update_boardname(){}

        // 進む、戻る
        virtual void back_viewhistory( const int count ){}
        virtual void forward_viewhistory( const int count ){}

        // 検索
        virtual void exec_search(){}
        virtual void up_search(){}
        virtual void down_search(){}
        virtual void operate_search( const std::string& controlid ){}
        virtual void set_search_query( const std::string& query ){ m_search_query = query; }
    };
}

#endif
