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
    class Admin;
    class PopupWin;
}

namespace DBTREE
{
    class ArticleBase;
}

namespace XML
{
    class Dom;
}

namespace ARTICLE
{
    class DrawAreaBase;

    class ArticleViewBase : public SKELETON::View
    {
        // viewに表示するdatファイルのURL ( SKELETON::View::m_url はview自身のURLなのに注意すること )
        std::string m_url_article; 

        // ツールバーの板ボタンに表示する文字列
        std::string m_label_board; 

        // 高速化のため直接アクセス
        JDLIB::RefPtr_Lock< DBTREE::ArticleBase > m_article; 

        // widget
        DrawAreaBase* m_drawarea;

        // slot呼び出し時にURLのやりとりに使う一時変数
        std::string m_url_tmp; // url
        std::string m_str_num; // レス番号
        std::string m_jump_to; // ジャンプ先番号
        std::string m_jump_from; // ジャンプ元番号
        std::string m_id_name; // ID
        std::string m_name;    // 名前

        // ポップアップ
        SKELETON::PopupWin* m_popup_win;
        bool m_popup_shown; // 表示されているならtrue, falseでもdeleteしない限りは m_popup_win != NULLに注意
        int m_hidepopup_counter; // ポップアップを消すまでのカウンタ
        std::string m_popup_url; // 表示中のポップアップのアドレス

        // 検索用
        bool m_search_invert; // 逆方向検索モード
        std::string m_pre_query;  // 前回の検索で使ったクエリー

        // ポップアップメニュー表示のときにactivate_act_before_popupmenu()で使う変数
        bool m_enable_menuslot;

        // ブックマーク移動時の現在の位置(レス番号)
        int m_current_bm;

        // 書き込みマーク移動時の現在の位置(レス番号)
        int m_current_post;

        // 抽出ビューで荒らし報告用のURLを表示する
        bool m_show_url4report;

        // URLをステータスバーに表示しているかどうか
        bool m_url_show_status;

        // 実況モードが可能か
        bool m_enable_live;

        // ライブモードか
        bool m_live;

        // メニューのユーザコマンド、 create_usrcmd_menu() で作成
        std::string m_usrcmd;

    public:

        ArticleViewBase( const std::string& url, const std::string& url_article );
        virtual ~ArticleViewBase();

        const std::string& url_article() const { return m_url_article; }

        const std::string& get_label_board(){ return m_label_board; }

        // SKELETON::View の関数のオーバロード

        virtual void save_session(){}        

        virtual const std::string url_for_copy(); // コピーやURLバー表示用のURL
        virtual const int width_client();
        virtual const int height_client();
        virtual const int get_icon( const std::string& iconname );
        virtual const bool set_command( const std::string& command,
                                        const std::string& arg1 = std::string(),
                                        const std::string& arg2 = std::string()
            );

        virtual void clock_in();
        void clock_in_smooth_scroll();

        // キーを押した
        virtual const bool slot_key_press( GdkEventKey* event );

        virtual void write();
        virtual void reload();
        virtual void stop();
        virtual void redraw_view();
        virtual void focus_view();
        virtual void focus_out();
        virtual void close_view();
        virtual void delete_view();
        virtual void set_favorite();
        virtual const bool operate_view( const int control );
        virtual void goto_top();
        virtual void goto_bottom();
        virtual void goto_num( const int num_to, const int num_from );
        virtual void show_preference();

        // 進む、戻る
        virtual void back_viewhistory( const int count );
        virtual void forward_viewhistory( const int count );
 
        // 検索
        virtual void exec_search();
        virtual void up_search();
        virtual void down_search();
        virtual void operate_search( const std::string& controlid );
        void clear_highlight();  // ハイライト解除

        // 記事削除 & 再オープン
        void delete_open_view();

        // 実況モードが可能か
        const bool get_enable_live();

        // 実況モードか
        const bool get_live() const { return m_live; }

        DrawAreaBase* drawarea();

    protected:

        // Viewが所属するAdminクラス
        virtual SKELETON::Admin* get_admin();

        const JDLIB::RefPtr_Lock< DBTREE::ArticleBase >& get_article() const noexcept;

        // ポップアップメニューを表示する前にメニューのアクティブ状態を切り替える
        virtual void activate_act_before_popupmenu( const std::string& url );

        // ポップアップメニュー取得
        virtual Gtk::Menu* get_popupmenu( const std::string& url );

        // レスポップアップを隠す
        void hide_popup( const bool force = false );

        // ポップアップが表示されているか
        const bool is_popup_shown() const;

        // 初期設定
        void setup_view();

        // 新着に移動
        void goto_new();

        // レスを抽出して表示
        // num は "from-to"　の形式 (例) 3から10を抽出したいなら "3-10"
        // show_title == trueの時は 板名、スレ名を表示
        void show_res( const std::string& num, const bool show_title );

        // 名前 で抽出して表示
        // show_option = true の時は URL 表示などのオプションが表示される
        void show_name( const std::string& name, bool show_option );

        // ID で抽出して表示
        // show_option = true の時は URL 表示などのオプションが表示される
        void show_id( const std::string& id_name, const bool show_option );

        // ブックマークを抽出して表示
        void show_bm();

        // 書き込みを抽出して表示
        void show_post();

        // 書き込みログを表示
        void show_postlog( const int num );

        // URLを含むレスを抽出して表示
        void show_res_with_url();

        // num 番のレスを参照してるレスを抽出して表示
        void show_refer( int num );

        // 前回の検索で使ったクエリー
        void set_pre_query( const std::string& query ){  m_pre_query = query; }
        const std::string& get_pre_query() const{ return m_pre_query; }

        // キーワードで抽出して表示
        // mode_or = true の時は or 検索
        // show_option = true の時は URL 表示などのオプションが表示される
        void drawout_keywords( const std::string& query, bool mode_or, bool show_option );

        // HTML追加
        void append_html( const std::string& html );

        // dat追加
        virtual void append_dat( const std::string& dat, int num );

        // リストで指定したレスを表示
        void append_res( const std::list< int >& list_resnum );

        // リストで指定したレスを表示(連結情報付き)
        void append_res( const std::list< int >& list_resnum, const std::list< bool >& list_joint );

        // 画像プロパティ表示
        void slot_preferences_image();

        void slot_copy_selection_str();
        void slot_select_all();

        // 実況用
        void set_enable_live( const bool enable ){ m_enable_live = enable; }
        void set_live( const bool live );
        virtual void live_start(){}
        virtual void live_stop(){}

    private:

        virtual DrawAreaBase* create_drawarea();        

        void setup_action();

        // 通常の右クリックメニューの作成
        const std::string create_context_menu();
        const char* get_menu_item( const int item );

        virtual void exec_reload();
        void reload_article();

        void exec_delete();

        // 荒らし報告用のURLリストをHTML形式で取得
        const std::string get_html_url4report( const std::list< int >& list_resnum );
        
        // drawarea の signal を受け取る slots
        virtual bool slot_button_press( std::string url, int res_number, GdkEventButton* event );
        bool slot_button_release( std::string url, int res_number, GdkEventButton* event );
        bool slot_motion_notify( GdkEventMotion* event );
        bool slot_key_release( GdkEventKey* event );
        bool slot_scroll_event( GdkEventScroll* event );
        bool slot_leave_notify( GdkEventCrossing* ev );

        // レスポップアップ関係

        // ポップアップが表示されていてかつマウスがその上にあるか
        const bool is_mouse_on_popup();

        void show_popup( SKELETON::View* view, const int mrg_x, const int mrg_y );
        bool slot_popup_leave_notify_event( GdkEventCrossing* event );
        void slot_hide_popup();
        void delete_popup(); // ポップアップ強制削除
        void warp_pointer_to_popup(); // マウスポインタをポップアップの上に移動する
        
        void slot_bookmark();
        void slot_postedmark();
        void slot_open_browser();
        void slot_open_cache_browser();
        void slot_write_res();
        void slot_quote_res();
        void slot_quote_selection_res();
        void slot_copy_current_url();
        void slot_copy_name();
        void slot_copy_id();
        void slot_drawout_selection_str();
        void slot_search_cacheall();
        void slot_search_cachelocal();
        void slot_search_next();
        void slot_search_web();
        void slot_search_title();
        void slot_usrcmd( int num );
        void slot_copy_res( bool ref );
        void slot_copy_title_url();
        void slot_drawout_res();
        void slot_drawout_around();
        void slot_drawout_tmp();
        void slot_drawout_name();
        void slot_drawout_id();
        void slot_drawout_bm();
        void slot_drawout_post();
        void slot_drawout_refer();
        void slot_drawout_url();
        void slot_abone_res();
        void slot_abone_selection_res();
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
        void slot_pre_post();
        void slot_next_post();
        void slot_jump();
        void slot_save_dat();
        void slot_copy_article_info();

        // あぼーん設定
        void slot_setup_abone();
        void slot_setup_abone_board();
        void slot_setup_abone_all();

        // リンクの処理
        virtual void slot_on_url( std::string url, std::string imgurl, int res_number );
        void slot_leave_url();
        bool click_url( std::string url, int res_number, GdkEventButton* event );
        void open_image( const std::string& url, const int res_number,
                         const bool open_imageview, const bool open_browser, const bool mosaic, const bool switch_image );

        // 画像ポップアップメニュー用
        void slot_cancel_mosaic();
        void slot_show_image_with_mosaic();
        void slot_show_selection_images();
        void slot_delete_selection_images();
        void slot_abone_selection_images();
        void slot_show_large_img();
        void slot_deleteimage();
        void slot_toggle_protectimage();
        void slot_saveimage();
        void slot_abone_img();

        // 検索バーを開く
        void open_searchbar( bool invert );
    };

}


#endif
