// ライセンス: GPL2

// スレ表示部のベースクラス

#ifndef _DRAWAREABASE_H
#define _DRAWAREABASE_H

#include "caret.h"
#include "scrollinfo.h"

#include "jdlib/refptr_lock.h"

#include "control/control.h"

#include "colorid.h"
#include "cssmanager.h"

#include <gtkmm.h>

#include <chrono>
#include <memory>


namespace ARTICLE
{
    struct LAYOUT;
    class LayoutTree;
    class EmbeddedImage;

    // マウスボタンプレスとリリースのシグナル。リリース時にマウスがリンクの上にある時そのURLを渡す
    typedef sigc::signal< bool, std::string, int, GdkEventButton* > SIG_BUTTON_PRESS;
    typedef sigc::signal< bool, std::string, int, GdkEventButton* > SIG_BUTTON_RELEASE;

    typedef sigc::signal< bool, GdkEventCrossing* > SIG_LEAVE_NOTIFY;
    typedef sigc::signal< bool, GdkEventMotion* > SIG_MOTION_NOTIFY;
    typedef sigc::signal< bool, GdkEventKey* > SIG_KEY_PRESS;
    typedef sigc::signal< bool, GdkEventKey* > SIG_KEY_RELEASE;
    typedef sigc::signal< bool, GdkEventScroll* > SIG_SCROLL_EVENT;

    typedef sigc::signal< void, std::string, std::string, int > SIG_ON_URL;
    typedef sigc::signal< void > SIG_LEAVE_URL;

    struct URLINFO
    {
        std::string url;
        int res_number;
    };

    // 範囲選択用
    struct SELECTION
    {
        /// @brief クリックして選択を変更したときの位置情報
        enum class ClickPosition : char
        {
            unknown,          ///< @brief クリックした位置を記憶していない
            before_first_hit, ///< @brief クリックした位置が先頭のヒットより前
            between_hits,     ///< @brief クリックした位置がヒットとヒットの間
            on_hit,           ///< @brief クリックした位置がヒットの上
        };

        bool select;
        ClickPosition click_pos;
        CARET_POSITION caret_from;
        CARET_POSITION caret_to;
        std::string str;      // 現在の選択文字列
        std::string str_pre;  // 一つ前の選択文字列
        std::vector< URLINFO > imgurls;// 選択範囲に含まれる画像URL
    };

    // 描画情報
    struct DRAWINFO
    {
        bool draw;
        int y;
        int height;
    };

    // フォント情報
    struct FONTINFO
    {
        std::string fontname;
        Pango::FontDescription pfd;
        int ascent;
        int descent;
        int height;
        int underline_pos; // 下線の位置(文字の上からのピクセル値)
        int br_size;       // 改行量 (ピクセル値)
        int mrg_right;     // wrap計算のときに使用する右マージン幅 (ピクセル値)
    };

    // 描画領域
    struct CLIPINFO
    {
        int width_view;
        int pos_y;
        int upper; // 画面上、小さい値
        int lower; // 画面下、大きい値
    };

    /// @brief 文字を描写する関数は変数が多いため構造体を使って参照を渡す
    struct RenderTextArguments
    {
        cairo_t* text_cr;
        struct RECTANGLE* rect; // NOTE: rect->width は更新されることがある
        const char* text;
        int n_ustr;
        int x;
        int y;
        int color;
        int color_back;
        int width_line;
        int byte_to;
        bool bold;
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
        Gtk::EventBox* m_event{};
        Gtk::Scrollbar* m_vscrbar{};

        // レイアウトツリー
        std::unique_ptr<LayoutTree> m_layout_tree;

        // 描画領域の幅、高さ( != ウィンドウサイズ )
        int m_width_client{};
        int m_height_client{};

        //現在見ているレスの番号
        int m_seen_current{};

        // 描画用
        Glib::RefPtr< Gdk::Window > m_window;

        // cairomm 1.12.0 がメモリリークを起こしたので
        // C API を使うことで問題を回避する
        cairo_t* m_cr{};
        std::unique_ptr< cairo_surface_t, void ( * )( cairo_surface_t* ) > m_backscreen;

        Glib::RefPtr< Pango::Layout > m_pango_layout;
        Glib::RefPtr< Pango::Context > m_context;
        RECTANGLE m_rect_backscreen{}; // バックスクリーンが描画されている範囲
        bool m_enable_draw;
        DRAWINFO m_drawinfo{};

        // キャレット情報
        CARET_POSITION m_caret_pos;           // 現在のキャレットの位置(クリックやドラッグすると移動)
        CARET_POSITION m_caret_pos_pre;       // 移動前のキャレットの位置(選択範囲の描画などで使用する)
        CARET_POSITION m_caret_pos_dragstart; // ドラッグを開始したキャレット位置

        // 色
        int m_colorid_text{}; // デフォルトの文字色
        int m_colorid_back{}; // デフォルトの背景色
        std::vector< Gdk::RGBA > m_color;

        // 枠
        bool m_draw_frame{};  // 枠を描画する
        // 枠線に隠れる部分のバックアップを分ける
        std::unique_ptr< cairo_surface_t, void ( * )( cairo_surface_t* ) > m_back_frame_top;
        std::unique_ptr< cairo_surface_t, void ( * )( cairo_surface_t* ) > m_back_frame_bottom;
        bool m_ready_back_frame{};

        // 範囲選択
        SELECTION m_selection{};

        // 検索結果のハイライト範囲
        std::list< SELECTION > m_multi_selection;
      
        // レイアウト用
        CORE::CSS_PROPERTY m_css_body; // body の cssプロパティ
        int m_fontid{};
        int m_defaultfontid{};
        int m_mailfontid{};
        int m_defaultmailfontid{};
        FONTINFO *m_font{};       // カレントフォント情報
        FONTINFO m_defaultfont{}; // デフォルトフォント情報
        FONTINFO m_aafont{};      // AA用フォント情報
        FONTINFO m_mailfont{};    // メールフォント情報
        bool m_aafont_initialized{};
        bool m_mailfont_initialized{};

        // スレビューで文字幅の近似を厳密にするか
        bool m_strict_of_char{};

        // ビューのリサイズ用
        bool m_configure_reserve{}; // true の時は再描画する前に再レイアウトする
        int m_configure_width{};
        int m_configure_height{};

        // スクロール情報
        double m_smooth_dy{}; // GDK_SCROLL_SMOOTH のスクロール変化量
        SCROLLINFO m_scrollinfo{};
        guint32 m_wheel_scroll_time{}; // 前回ホイールを回した時刻
        int m_goto_num_reserve{}; // 初期化時のジャンプ予約(レス番号)
        bool m_goto_bottom_reserve{}; // 初期化時のジャンプ予約(底)
        int m_pre_pos_y; // ひとつ前のスクロールバーの位置。スクロールした時の差分量計算に使用する
        std::vector< int > m_jump_history;  // ジャンプ履歴
        bool m_cancel_change_adjust{}; // adjust の値変更イベントをキャンセル

        std::unique_ptr< cairo_surface_t, void ( * )( cairo_surface_t* ) > m_back_marker;
        RECTANGLE m_clip_marker{};
        bool m_ready_back_marker{};
        using Monotonic = std::chrono::steady_clock;
        Monotonic::duration m_wait_scroll{}; // 処理落ちした時にスクロールにウエイトを入れる
        Monotonic::time_point m_scroll_time; // ウエイト時に最後にスクロールした時刻

        // 状態
        int m_x_pointer{};
        int m_y_pointer{};   // 現在のマウスポインタの位置
        bool m_key_press{};  // キーを押している
        bool m_key_locked{}; // 同じキーを押したままにしている
        guint m_keyval{};
        bool m_clicked{};
        bool m_drugging{};   // ドラッグ中
        bool m_r_drugging{}; // 右ドラッグ中
        std::string m_link_current; // 現在マウスポインタの下にあるリンクの文字列
        LAYOUT* m_layout_current{}; // 現在マウスポインタの下にあるlayoutノード(下が空白ならnullptr)
        Glib::ustring m_cursor_type; // カーソルの形状

        // 入力コントローラ
        CONTROL::Control m_control;

        // 埋め込み画像を保持するリスト
        std::list<EmbeddedImage> m_eimgs;

        // ブックマークアイコン
        Glib::RefPtr< Gdk::Pixbuf > m_pixbuf_bkmk;

        // 書き込みマークアイコン
        Glib::RefPtr< Gdk::Pixbuf > m_pixbuf_post;

        // 自分の書き込みに対するレスマークアイコン
        Glib::RefPtr< Gdk::Pixbuf > m_pixbuf_refer_post;

        // マウスのクリックとタッチスクリーンのタップ
        Glib::RefPtr< Gtk::GestureMultiPress > m_gesture_multipress;

        // タッチスクリーンのスクロール
        Glib::RefPtr< Gtk::GesturePan > m_gesture_pan;
        Glib::RefPtr< Gtk::GestureSwipe > m_gesture_swipe;
        double m_drag_start_y{};

        // 慣性スクロール
        struct DecelerationInfo
        {
            double elapsed; // 換算された経過時間
            double initial_dy; // スケーリングされた初速度(pixcels/frame)
            gint64 last_time; // 前回コールバックが呼び出された時間(frame)
            guint id; // コールバックのID
        } m_deceleration{};

        /// @brief 文字を描画するメンバー関数の関数ポインター
        void (DrawAreaBase::*m_render_text)( RenderTextArguments& args );

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
        auto sig_view_map() { return m_view.signal_map(); }

        explicit DrawAreaBase( const std::string& url );
        ~DrawAreaBase() override;

        const std::string& get_url() const { return m_url; }
        int width_client() const { return m_width_client; }
        int height_client() const { return m_height_client; }

        void clock_in();
        void clock_in_smooth_scroll();
        void focus_view();
        void focus_out();

        void set_enable_draw( const bool enable ){ m_enable_draw = enable; }

        // フォントID( fontid.h にある ID を指定)
        int get_fontid() const { return m_fontid; }
        void set_fontid( int id ){ m_fontid = id; m_defaultfontid = id; }
        int get_mailfontid() const { return m_fontid; }
        void set_mailfontid( int id ){ m_mailfontid = id; m_defaultmailfontid = id; }

        // 新着セパレータのあるレス番号の取得とセット
        int get_separator_new() const;
        void set_separator_new( int num );
        void hide_separator_new();

        // 現在のポインタの下にあるレス番号取得
        int get_current_res_num() const;

        // 全選択
        void select_all(); 

        // 範囲選択中の文字列
        std::string str_selection() const;
        const std::string& str_pre_selection() const { return m_selection.str_pre; }  // 一つ前の選択文字列

        // 範囲選択を開始したレス番号
        int get_selection_resnum_from() const;

        // 範囲選択を終了したレス番号
        int get_selection_resnum_to() const;

        // 範囲選択に含まれる画像URLのリスト
        const std::vector< URLINFO >& get_selection_imgurls() const { return m_selection.imgurls; }

        int get_seen_current() const { return m_seen_current; } // 現在見ているレスの番号
        int get_goto_num_reserve() const { return m_goto_num_reserve; } // 初期化時のジャンプ予約(レス番号)

        int max_number() const;   // 表示されている最後のレスの番号

        // レスをappendして再レイアウト
        void append_res( const int from_num, const int to_num );

        // リストで指定したレスをappendして再レイアウト
        void append_res( const std::list< int >& list_resnum );

        // リストで指定したレスをappendして再レイアウト( 連結情報付き )
        // list_joint で連結指定したレスはヘッダを取り除いて前のレスに連結する
        void append_res( const std::list< int >& list_resnum, const std::list< bool >& list_joint );

        // html をappendして再レイアウト
        void append_html( const std::string& html );

        // datをappendして再レイアウト
        void append_dat( const std::string& dat, int num );

        // 全画面消去
        void clear_screen();

        // 再描画
        // 再レイアウトはしないが configureの予約がある場合は再レイアウトしてから再描画する
        void redraw_view_force();
        void redraw_view();

        // スクロール方向指定
        bool set_scroll( const int control );

        // マウスホイールの処理
        void wheelscroll( GdkEventScroll* event );

        // ジャンプ
        void set_jump_history( const int num ); // ジャンプ履歴にスレ番号を登録
        void goto_num( int num );
        void goto_next_res();
        void goto_pre_res();
        void goto_new();
        void goto_top();
        void goto_bottom();
        void goto_back();

        // 検索
        int search( const std::list< std::string >& list_query, const bool reverse );
        int search_move( const bool reverse );
        void clear_highlight();
        void reset_caret_position();

        // 実況モード
        void live_start();
        void live_stop();
        void update_live_speed( const int sec );

      protected:

        // リアライズしたか
        // Gtk::Widget::is_realized() はうまく動作しない
        bool is_drawarea_realized() const noexcept { return static_cast<bool>(m_window); }

        // 文字色のID( colorid.h にある ID を指定)
        int get_colorid_text() const { return m_colorid_text; }
        void set_colorid_text( int id ){ m_colorid_text = id; }

        // 背景色のID( colorid.h にある ID を指定)
        int get_colorid_back() const;
        void set_colorid_back( int id ){ m_colorid_back = id; }

        // 共通セットアップ
        // show_abone : あぼーんされたスレも表示
        // show_scrbar : スクロールバーを最初から表示
        // show_multispace : 連続空白も表示
        void setup( const bool show_abone, const bool show_scrbar, const bool show_multispace );

        // レイアウト処理
        virtual bool exec_layout();
        bool exec_layout_impl( const bool is_popup, const int offset_y );

        // DrawAreaに枠を描画する
        void set_draw_frame( bool draw_frame ){ m_draw_frame = draw_frame; }

        // リサイズした
        virtual bool slot_configure_event( GdkEventConfigure* event );

        Gtk::DrawingArea* get_view(){ return &m_view; }
        Gtk::Scrollbar* get_vscrbar(){ return m_vscrbar; }

      private:

        // 初期化関係
        void clear();
        void create_scrbar();
        void init_color();
        void init_font();
        void init_fontinfo( FONTINFO& fi, std::string& fontname );

        // レイアウト処理
        void adjust_layout_baseline( LAYOUT* header );
        void set_align( LAYOUT* div, int id_end, int align );
        void set_align_line( LAYOUT* div, LAYOUT* layout_from, LAYOUT* layout_to, RECTANGLE* rect_from, RECTANGLE* rect_to,
                             int width_line, int align );
        void layout_one_text_node( LAYOUT* layout, int& x, int& y, int& br_size, const int width_view );
        void layout_one_img_node( LAYOUT* layout, int& x, int& y, int& br_size, const int width_view,
                                  const bool init_popupwin, const int mrg_right, const int mrg_bottom );

        // 文字の幅などの情報
        int get_width_of_one_char( const char* utfstr, int& byte, char& pre_char, bool& wide_mode, const int mode );
        bool set_init_wide_mode( const char* str, const int pos_start, const int pos_to );
        bool is_wrapped( const int x, const int border, const char* str ) const;

        // スクリーン描画
        // y から height の高さ分だけ描画する
        // height == 0 ならスクロールした分だけ描画( y は無視 )
        bool draw_screen( const int y, const int height );
        void exec_draw_screen( const int y_redraw, const int height_redraw );

        bool draw_one_node( LAYOUT* layout, const CLIPINFO& ci );
        void draw_div( LAYOUT* layout_div, const CLIPINFO& ci );
        bool get_selection_byte( const LAYOUT* layout, const SELECTION& selection, size_t& byte_from,
                                 size_t& byte_to ) const;
        void draw_one_text_node( LAYOUT* layout, const CLIPINFO& ci );
        void draw_string( LAYOUT* node, const CLIPINFO& ci,
                          const int color, const int color_back, const int byte_from, const int byte_to );
        void render_text_glyphstring( RenderTextArguments& args );
        void render_text_pangolayout( RenderTextArguments& args );
        bool draw_one_img_node( LAYOUT* layout, const CLIPINFO& ci );
        int get_layout_fontid( LAYOUT *layout ) const;
        void set_node_font( LAYOUT* layout );

        // drawarea がリサイズ実行
        void configure_impl();

        // 整数 -> 文字変換してノードに発言数をセット
        int set_num_id( LAYOUT* layout );

        // スクロール関係
        void exec_scroll(); // スクロールやジャンプを実行して再描画
        int get_vscr_val() const;
        int get_vscr_maxval() const;

        // キャレット関係
        bool is_pointer_on_rect( const RECTANGLE* rect, LAYOUT* layout, const char* text, const int pos_start, const int pos_to,
                                 const int x, const int y,
                                 int& pos, int& width_line, int& char_width, int& byte_char );
        LAYOUT* set_caret( CARET_POSITION& caret_pos, int x, int y );
        bool set_carets_dclick( CARET_POSITION& caret_left, CARET_POSITION& caret_right,  const int x, const int y, const bool triple );

        // 範囲選択関係
        bool set_selection( const CARET_POSITION& caret_left, const CARET_POSITION& caret_right );
        bool set_selection( const CARET_POSITION& caret_pos );
        bool set_selection( const CARET_POSITION& caret_pos, RECTANGLE* rect );
        bool set_selection_str();
        bool is_caret_on_selection( const CARET_POSITION& caret_pos ) const;
        std::string get_selection_as_url( const CARET_POSITION& caret_pos ) const;

        void update_search_start_position( const CARET_POSITION& caret_pos );

        // マウスが動いた時の処理
        bool motion_mouse();

        // 現在のポインターの下のノードからカーソルのタイプを決定する
        Glib::ustring get_cursor_type() const;

        // カーソルの形状の変更
        void change_cursor( const Glib::ustring& cursor_type );

        // スクロールマーカの描画
        void draw_marker();

        // 枠の描画
        void draw_frame();

        // バックスクリーンを矩形で塗りつぶす
        void fill_backscreen( const int colorid, int x, int y, int width, int height );

        // Pixbufの内容をバックスクリーンに貼り付ける
        void paint_backscreen( const Glib::RefPtr< Gdk::Pixbuf >& pixbuf,
                               int src_x, int src_y, int dest_x, int dest_y, int width, int height );

        // スロット
        void slot_change_adjust();
        bool slot_draw( const Cairo::RefPtr< Cairo::Context >& cr );
        bool slot_scroll_event( GdkEventScroll* event );
        bool slot_leave_notify_event( GdkEventCrossing* event );
        void slot_realize();

        bool slot_button_press_event( GdkEventButton* event );
        bool slot_button_release_event( GdkEventButton* event );
        bool slot_motion_notify_event( GdkEventMotion* event );

        bool slot_key_press_event( GdkEventKey* event );
        bool slot_key_release_event( GdkEventKey* event );

        void setup_event_controller();

        void slot_multipress_pressed( int n_press, double x, double y );
        void slot_multipress_released( int n_press, double x, double y );

        void slot_pan_begin( double start_x, double start_y );
        void slot_pan_update( double offset_x, double offset_y );
        void slot_gesture_end( GdkEventSequence* sequence );

        void slot_swipe( double velocity_x, double velocity_y );

        // 慣性スクロール
        static gboolean deceleration_tick_cb( GtkWidget* cwidget, GdkFrameClock* clock, gpointer );
        gboolean deceleration_tick_impl( GdkFrameClock* clock );
        void cancel_deceleration();
    };


    //
    // 文字列を wrap するか判定する関数
    //
    // str != nullptr なら禁則処理も考える
    //
    inline bool DrawAreaBase::is_wrapped( const int x, const int border, const char* str ) const
    {
        const unsigned char* tmpchar = reinterpret_cast<const unsigned char*>( str );

        if( x < border ) return false;
        if( ! tmpchar ) return true;

        // 禁則文字
        if(    ( tmpchar[ 0 ] == ',' )
            || ( tmpchar[ 0 ] == '.' )

            // UTF-8で"。"
            || ( tmpchar[ 0 ] == 0xe3 && tmpchar[ 1 ] == 0x80 && tmpchar[ 2 ] == 0x82 )

            // UTF-8で"、"
            || ( tmpchar[ 0 ] == 0xe3 && tmpchar[ 1 ] == 0x80 && tmpchar[ 2 ] == 0x81 )
            ) return false;

        return true;
    }
}

#endif

