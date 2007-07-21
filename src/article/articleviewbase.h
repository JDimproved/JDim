// ライセンス: GPL2

// スレビュークラスの基本クラス

#ifndef _ARTICLEVIEWBASE_H
#define _ARTICLEVIEWBASE_H

#include "skeleton/view.h"

#include "jdlib/refptr_lock.h"

#include <gtkmm.h>
#include <list>


namespace SKELETON
{
    class PopupWin;
}

namespace DBTREE
{
    class ArticleBase;
}

namespace ARTICLE
{
    class DrawAreaBase;
    class ArticleToolBar;

    class ArticleViewBase : public SKELETON::View
    {
        // viewに表示するdatファイルのURL ( SKELETON::View::m_url はview自身のURLなのに注意すること )
        std::string m_url_article; 

        // 高速化のため直接アクセス
        JDLIB::RefPtr_Lock< DBTREE::ArticleBase > m_article; 

        // widget
        DrawAreaBase* m_drawarea;
        ArticleToolBar *m_toolbar;

        // slot呼び出し時にURLのやりとりに使う一時変数
        std::string m_url_tmp; // url
        std::string m_str_num; // レス番号
        std::string m_id_name; // ID
        std::string m_name;    // 名前

        // ポップアップ
        SKELETON::PopupWin* m_popup_win;
        bool m_popup_shown; // 表示されているならtrue, falseでもdeleteしない限りは m_popup_win != NULLに注意

        // 検索用
        bool m_search_invert; // 逆方向検索モード
        std::string m_query;  // 前回の検索で使ったクエリー

        // ポップアップメニュー表示のときにactivate_act_before_popupmenu()で使う変数
        bool m_enable_menuslot;

        // ブックマーク移動時の現在の位置(レス番号)
        int m_current_bm;

        // 抽出ビューで荒らし報告用のURLを表示する
        bool m_show_url4report;

    public:

        ArticleViewBase( const std::string& url );
        virtual ~ArticleViewBase();

        const std::string& url_article() const { return m_url_article; }
        virtual const std::string url_for_copy();

        // SKELETON::View の関数のオーバロード
        virtual const int width_client();
        virtual const int height_client();
        virtual const int get_icon( const std::string& iconname );
        virtual bool set_command( const std::string& command, const std::string& arg = std::string() );

        virtual void clock_in();
        virtual void reload();
        virtual void stop();
        virtual void redraw_view();
        virtual void focus_view();
        virtual void focus_out();
        virtual void close_view();
        virtual void delete_view();
        virtual void operate_view( const int& control );
        virtual void goto_top();
        virtual void goto_bottom();
        virtual void goto_num( int num );
        virtual void toggle_toolbar();

        // 記事削除 & 再オープン
        void delete_open_view();

    protected:

        DrawAreaBase* drawarea();
        ArticleToolBar* toolbar() { return m_toolbar; }
        JDLIB::RefPtr_Lock< DBTREE::ArticleBase >& get_article();

        // ポップアップメニューを表示する前にメニューのアクティブ状態を切り替える
        virtual void activate_act_before_popupmenu( const std::string& url );

        // ポップアップメニュー取得
        virtual Gtk::Menu* get_popupmenu( const std::string& url );

        // レスポップアップを隠す
        void hide_popup( bool force = false );

        // 初期設定
        void setup_view();

        // 新着に移動
        void goto_new();

        // レスを抽出して表示
        // num は "from-to"　の形式 (例) 3から10を抽出したいなら "3-10"
        // show_title == trueの時は 板名、スレ名を表示
        void show_res( const std::string& num, bool show_title );

        // 名前 で抽出して表示
        // show_option = true の時は URL 表示などのオプションが表示される
        void show_name( const std::string& name, bool show_option );

        // ID で抽出して表示
        // show_option = true の時は URL 表示などのオプションが表示される
        void show_id( const std::string& id_name, bool show_option );

        // ブックマークを抽出して表示
        void show_bm();

        // URLを含むレスを抽出して表示
        void show_res_with_url();

        // num 番のレスを参照してるレスを抽出して表示
        void show_refer( int num );

        // キーワードで抽出して表示
        // mode_or = true の時は or 検索
        // show_option = true の時は URL 表示などのオプションが表示される
        void drawout_keywords( const std::string& query, bool mode_or, bool show_option );

        // HTML追加
        void append_html( const std::string& html );

        // dat追加
        virtual void append_dat( const std::string& dat, int num = 0 );

        // リストで指定したレスを表示
        void append_res( std::list< int >& list_resnum );

        // リストで指定したレスを表示(連結情報付き)
        void append_res( std::list< int >& list_resnum, std::list< bool >& list_joint );

        // ツールバーのボタンを押したときのスロット
        void slot_push_close_search();
        void slot_push_reload();
        void slot_push_write();
        void slot_push_delete();
        void slot_push_open_board();
        void slot_push_preferences();
        void slot_preferences_image();
        void slot_push_open_search();
        void slot_push_up_search();
        void slot_push_down_search();
        void slot_push_drawout_and();
        void slot_push_drawout_or();
        void slot_push_claar_hl();

    private:

        virtual DrawAreaBase* create_drawarea();        

        virtual void pack_widget();
        void setup_action();

        // 荒らし報告用のURLリストをHTML形式で取得
        std::string get_html_url4report( std::list< int >& list_resnum );
        
        // drawarea の signal を受け取る slots
        virtual bool slot_button_press( std::string url, int res_number, GdkEventButton* event );
        bool slot_button_release( std::string url, int res_number, GdkEventButton* event );
        bool slot_motion_notify( GdkEventMotion* event );
        bool slot_key_press( GdkEventKey* event );
        bool slot_key_release( GdkEventKey* event );
        bool slot_scroll_event( GdkEventScroll* event );
        bool slot_leave_notify( GdkEventCrossing* ev );

        // レスポップアップ関係

        // ポップアップが表示されているか
        const bool is_popup_shown() const;

        // ポップアップが表示されていてかつマウスがその上にあるか
        const bool is_mouse_on_popup();

        void show_popup( SKELETON::View* view, int margin );
        bool slot_popup_leave_notify_event( GdkEventCrossing* event );
        void slot_hide_popup();
        void delete_popup(); // ポップアップ強制削除
        void warp_pointer_to_popup(); // マウスポインタをポップアップの上に移動する
        
        void slot_bookmark();
        void slot_open_browser();
        void slot_write_res();
        void slot_quote_res();
        void slot_quote_selection_res();
        void slot_copy_current_url();
        void slot_copy_name();
        void slot_copy_id();
        void slot_copy_selection_str();
        void slot_drawout_selection_str();
        void slot_search_cacheall();
        void slot_search_cachelocal();
        void slot_usrcmd( int num );
        void slot_copy_res( bool ref );
        void slot_favorite();
        virtual void slot_drawout_res();
        virtual void slot_drawout_around();
        virtual void slot_drawout_tmp();
        virtual void slot_drawout_name();
        virtual void slot_drawout_id();
        virtual void slot_drawout_bm();
        virtual void slot_drawout_refer();
        virtual void slot_drawout_url();
        void slot_abone_res();
        void slot_abone_id();
        void slot_abone_name();
        void slot_abone_word();
        void slot_abone_name_board();
        void slot_abone_word_board();
        void slot_global_abone_name();
        void slot_global_abone_word();
        void slot_toggle_abone_transp();
        void slot_toggle_abone_transp_chain();
        void slot_pre_bm();
        void slot_next_bm();
        void slot_jump();
        void slot_save_dat();

        // リンクの処理
        void slot_on_url( std::string url, int res_number );
        void slot_leave_url();
        bool click_url( std::string url, int res_number, GdkEventButton* event );

        // 画像ポップアップメニュー用
        void slot_cancel_mosaic();
        void slot_deleteimage();
        void slot_toggle_protectimage();
        void slot_saveimage();
        void slot_abone_img();

        // 検索
        virtual void open_searchbar( bool invert );
        virtual void slot_active_search();
        virtual void slot_entry_operate( int controlid );
    };

}


#endif
