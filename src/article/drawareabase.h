// ライセンス: 最新のGPL

// スレ表示部のベースクラス

#ifndef _DRAWAREABASE_H
#define _DRAWAREABASE_H

#include "caret.h"
#include "scrollinfo.h"

#include "jdlib/refptr_lock.h"

#include "colorid.h"
#include "control.h"

#include <gtkmm.h>
#include <string>

namespace ARTICLE
{
    struct LAYOUT;
    class LayoutTree;

    // マウスボタンプレスとリリースのシグナル。リリース時にマウスがリンクの上にある時そのURLを渡す
    typedef sigc::signal< bool, GdkEventButton* > SIG_BUTTON_PRESS;
    typedef sigc::signal< bool, std::string, int, GdkEventButton* > SIG_BUTTON_RELEASE;

    typedef sigc::signal< bool, GdkEventCrossing* > SIG_LEAVE_NOTIFY;
    typedef sigc::signal< bool, GdkEventMotion* > SIG_MOTION_NOTIFY;
    typedef sigc::signal< bool, GdkEventKey* > SIG_KEY_PRESS;
    typedef sigc::signal< bool, GdkEventKey* > SIG_KEY_RELEASE;
    typedef sigc::signal< bool, GdkEventScroll* > SIG_SCROLL_EVENT;

    typedef sigc::signal< void, std::string, int > SIG_ON_URL;
    typedef sigc::signal< void > SIG_LEAVE_URL;
    
    // 範囲選択用
    struct SELECTION
    {
        bool select;
        CARET_POSITION caret_from;
        CARET_POSITION caret_to;
        std::string str;
    };


    ///////////////////////////////////


    class DrawAreaBase : public Gtk::HBox
    {
        SIG_BUTTON_PRESS m_sig_button_press;
        SIG_BUTTON_RELEASE m_sig_button_release;
        SIG_SCROLL_EVENT m_sig_scroll_event;
        SIG_LEAVE_NOTIFY m_sig_leave_notify;
        SIG_MOTION_NOTIFY m_sig_motion_notify;
        SIG_KEY_PRESS m_sig_key_press;
        SIG_KEY_RELEASE m_sig_key_release;
        SIG_ON_URL m_sig_on_url;
        SIG_LEAVE_URL m_sig_leave_url;

        std::string m_url;

        // スピードを稼ぐためデータベースに直接アクセスする
        JDLIB::RefPtr_Lock< DBTREE::ArticleBase > m_article; 

        // HBoxに張り付けるウイジット
        Gtk::DrawingArea m_view;
        Gtk::EventBox* m_event;
        Gtk::VScrollbar* m_vscrbar;

        // レイアウトツリー
        LayoutTree* m_layout_tree;

        // 描画領域の幅、高さ( != ウィンドウサイズ )
        int m_width_client; 
        int m_height_client;

        // 描画用
        Glib::RefPtr< Gdk::Window > m_window;
        Glib::RefPtr< Gdk::GC > m_gc;
        Glib::RefPtr< Gdk::Pixmap > m_backscreen;
        Glib::RefPtr< Pango::Layout > m_pango_layout;

        // キャレット情報
        CARET_POSITION m_caret_pos;           // 現在のキャレットの位置(クリックやドラッグすると移動)
        CARET_POSITION m_caret_pos_pre;       // 移動前のキャレットの位置(選択範囲の描画などで使用する)
        CARET_POSITION m_caret_pos_current;   // 現在のマウスポインタの下のキャレットの位置(マウスを動かすと移動)
        CARET_POSITION m_caret_pos_dragstart; // ドラッグを開始したキャレット位置

        // 色
        Gdk::Color m_color[ COLOR_NUM ];

        // 新着セパレータの位置(レス番号),  0 なら表示しない
        int m_separator_new; 

        // 範囲選択
        SELECTION m_selection;

        // 検索結果のハイライト範囲
        std::list< SELECTION > m_multi_selection;
      
        // レイアウト用
        int m_underline_pos; // 下線の位置(文字の上からのピクセル値)
        int m_br_size; // 改行量 (ピクセル値)
        int m_mrg_left; // 左マージン幅 (ピクセル値)
        int m_mrg_right; // 右マージン幅 (ピクセル値)
        int m_down_size; // 字下げ幅(ピクセル)

        // スクロール情報
        SCROLLINFO m_scrollinfo;
        guint32 m_wheel_scroll_time; // 前回ホイールを回した時刻
        int m_goto_num_reserve; // 初期化時のジャンプ予約(レス番号)
        int m_pre_pos_y; // スクロールした時の差分量(dy)

        // 状態
        int m_x_pointer, m_y_pointer;  // 現在のマウスポインタの位置
        bool m_key_press; // キーを押している
        bool m_drugging;  // ドラッグ中
        bool m_r_drugging; // 右ドラッグ中
        std::string m_link_current; // 現在マウスポインタの下にあるリンクの文字列
        LAYOUT* m_layout_current; // 現在マウスポインタの下にあるlayoutノード(下が空白ならNULL)

        // 入力コントローラ
        CONTROL::Control m_control;

      public:

        SIG_BUTTON_PRESS sig_button_press(){ return m_sig_button_press; }
        SIG_BUTTON_RELEASE sig_button_release(){ return m_sig_button_release; }
        SIG_SCROLL_EVENT sig_scroll_event(){ return m_sig_scroll_event; }
        SIG_LEAVE_NOTIFY sig_leave_notify(){ return m_sig_leave_notify; }
        SIG_MOTION_NOTIFY sig_motion_notify(){ return m_sig_motion_notify; }
        SIG_KEY_PRESS sig_key_press(){ return m_sig_key_press; }
        SIG_KEY_RELEASE sig_key_release(){ return m_sig_key_release; }
        SIG_ON_URL sig_on_url(){ return m_sig_on_url; }
        SIG_LEAVE_URL sig_leave_url(){ return m_sig_leave_url; }

        DrawAreaBase( const std::string& url );
        ~DrawAreaBase();

        const std::string& get_url() const { return m_url; }
        const int& width_client() const { return m_width_client; }
        const int& height_client() const { return m_height_client; }
        int get_separator_new(){ return m_separator_new; }
        void set_separator_new( int val ){ m_separator_new = val; }

        void clock_in();
        void focus_view();
        void focus_out();

        const std::string str_selection(); // 範囲選択中の文字列
        bool is_mouse_on_drawarea(); // マウスが描画領域の上にあるかどうか
        int seen_current(); // 現在見ているレスの番号
        int max_number();   // 表示されている最後のレスの番号

        // レスをappendして再レイアウト
        void append_res( int from_num, int to_num );

        // リストで指定したレスをappendして再レイアウト
        void append_res( std::list< int >& list_resnum );

        // リストで指定したレスをappendして再レイアウト( 連結情報付き )
        // list_joint で連結指定したレスはヘッダを取り除いて前のレスに連結する
        void append_res( std::list< int >& list_resnum, std::list< bool >& list_joint );

        // html をappendして再レイアウト
        void append_html( const std::string& html );

        // datをappendして再レイアウト
        void append_dat( const std::string& dat, int num );

        // 全画面消去
        void clear_screen();

        // バックスクリーンを描き直して再描画予約(queue_draw())する。再レイアウトはしない
        void redraw_view();  

        // スクロール方向指定
        bool set_scroll( const int& control );

        // マウスホイールの処理
        void wheelscroll( GdkEventScroll* event );

        // ジャンプ
        void goto_num( int num );
        void goto_new();
        void goto_top();
        void goto_bottom();

        // 検索
        bool search( std::list< std::string >& list_query, bool reverse );
        void search_move( bool reverse );
        void clear_highlight();

      protected:

        // 共通セットアップ
        void setup( bool show_abone, bool show_scrbar );

        // レイアウト処理
        virtual void layout();
        void layout_impl( bool nowrap, int offset_y, int right_mrg );

        // バックスクリーンをDrawAreaにコピー
        virtual bool draw_drawarea();

        // DrawAreaに枠を描画
        void draw_frame();

      private:

        // 背景色
        virtual const int* rgb_color_back();

        // フォント
        virtual const std::string& fontname(); 

        // フォントモード
        virtual const int fontmode();

        // 初期化関係
        void clear();
        void create_scrbar();
        void init_color();
        void init_font();

        // 描画、レイアウト関係
        void layout_one_node( LAYOUT* node, int& x, int& y, int width_win, int& mrg_level ); 
        bool draw_backscreen( bool redraw_all = false ); // バックスクリーン描画
        int get_width_of_one_char( const char* str, int& byte );
        void draw_one_node( LAYOUT* layout, const int& width_win, const int& pos_y );
        void draw_one_text_node( LAYOUT* layout, const int& width_win, const int& pos_y );
        void layout_draw_one_node( LAYOUT* node, int& x, int& y, int width_win, bool do_draw, bool bold = false, 
                                   int color = COLOR_CHAR, int color_bac = COLOR_BACK, int byte_from = 0, int byte_to = 0 );

        // スクロール関係
        void exec_scroll( bool redraw_all ); // スクロールやジャンプを実行して再描画
        int get_vscr_val();

        // キャレット関係
        LAYOUT* set_caret( CARET_POSITION& caret_pos, int x, int y );
        bool set_carets_dclick( CARET_POSITION& caret_left, CARET_POSITION& caret_right,  int x, int y );

        // 範囲選択関係
        bool set_selection( CARET_POSITION& caret_left, CARET_POSITION& caret_right );
        bool set_selection( CARET_POSITION& caret_pos, bool redraw = true );
        bool set_selection_str();
        std::string get_selection_as_url( const CARET_POSITION& caret_pos );

        // マウスが動いた時の処理
        bool motion_mouse();

        // スロット
        void slot_change_adjust();
        bool slot_configure_event( GdkEventConfigure* event );
        bool slot_expose_event( GdkEventExpose* event );
        bool slot_scroll_event( GdkEventScroll* event );
        bool slot_leave_notify_event( GdkEventCrossing* event );
        void slot_realize();

        bool slot_button_press_event( GdkEventButton* event );
        bool slot_button_release_event( GdkEventButton* event );
        bool slot_motion_notify_event( GdkEventMotion* event );

        bool slot_key_press_event( GdkEventKey* event );
        bool slot_key_release_event( GdkEventKey* event );
    };
}

#endif

