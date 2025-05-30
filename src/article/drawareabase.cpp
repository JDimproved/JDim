// ライセンス: GPL2

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//#define _DEBUG
//#define _DEBUG_CARETMOVE
//#define _DRAW_CARET
#include "jddebug.h"
#include "gtkmmversion.h"

#include "drawareabase.h"
#include "layouttree.h"
#include "font.h"
#include "embeddedimage.h"

#include "jdlib/jdregex.h"
#include "jdlib/misccharcode.h"
#include "jdlib/miscgtk.h"
#include "jdlib/miscmsg.h"
#include "jdlib/miscutil.h"

#include "dbtree/articlebase.h"
#include "dbtree/node.h"
#include "dbtree/interface.h"

#include "dbimg/imginterface.h"
#include "dbimg/img.h"

#include "config/globalconf.h"

#include "icons/iconmanager.h"

#include "control/controlid.h"

#include "global.h"
#include "httpcode.h"
#include "colorid.h"
#include "fontid.h"
#include "cache.h"
#include "cssmanager.h"
#include "session.h"

#include <pangomm/glyphstring.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>
#include <sstream>


using namespace ARTICLE;


static constexpr int AUTOSCR_CIRCLE = 24; ///< オートスクロールの時のサークルの大きさ

enum
{
    BIG_HEIGHT = 100000000,

    LAYOUT_MIN_HEIGHT = 2, // viewの高さがこの値よりも小さい時はリサイズしていないと考える

    EIMG_ICONSIZE = 32,  // 埋め込み画像のアイコンサイズ
    EIMG_MRG = 8,       // 埋め込み画像のアイコンの間隔

    EIMG_SSSP_MRG = 2,       // SSSP画像のアイコンの間隔 ( 上下マージンのみ )
    EIMG_SSSP_FULL = EIMG_SSSP_MRG * 2,

    WIDTH_FRAME = 1, // フレーム幅

    SPACE_TAB =  4, // 水平タブをどれだけ空けるか ( フォントの高さ * SPACE_TAB )

    COUNTER_NOMOTION = 16, // ホイールスクロール直後にモーションイベントをキャンセルする時の遊び量

    SEARCH_BUFFER_SIZE = 64 * 1024   // 検索のバッファサイズ
};


namespace {
namespace cursor_names {

// カーソル名の入力ミスを予防するため文字列リテラルのかわりに定数を使う
// 参考: https://developer.gnome.org/gdk3/stable/gdk3-Cursors.html#gdk-cursor-new-from-name
constexpr const char kAllScroll[] = "all-scroll";
constexpr const char kDefault[] = "default";
constexpr const char kPointer[] = "pointer";
constexpr const char kText[] = "text";

} // namespace cursor_names
} // namespace


#define SCROLLSPEED_FAST ( m_vscrbar ? \
                           m_vscrbar->get_adjustment()->get_page_size() \
                           - m_vscrbar->get_adjustment()->get_step_increment()*CONFIG::get_key_fastscroll_size() \
                           : 0 )
#define SCROLLSPEED_MID  ( m_vscrbar ? m_vscrbar->get_adjustment()->get_page_size()/2 : 0 )
#define SCROLLSPEED_SLOW ( m_vscrbar ? m_vscrbar->get_adjustment()->get_step_increment()*CONFIG::get_key_scroll_size() : 0 )

#define IS_ALPHABET( chr ) ( ( chr >= 'a' && chr <= 'z' ) || ( chr >= 'A' && chr <= 'Z' ) )

#define HAS_TEXT(layout) ( ( layout->type == DBTREE::NODE_TEXT || layout->type == DBTREE::NODE_IDNUM || layout->type == DBTREE::NODE_LINK ) && layout->text )


// 検索で使用
struct LAYOUT_TABLE
{
    LAYOUT* layout;
    size_t offset;
};


//////////////////////////////////////////////////////////



DrawAreaBase::DrawAreaBase( const std::string& url )
    : m_url( url )
    , m_backscreen( nullptr, cairo_surface_destroy )
    , m_enable_draw{ true }
    , m_back_frame_top( nullptr, cairo_surface_destroy )
    , m_back_frame_bottom( nullptr, cairo_surface_destroy )
    , m_pre_pos_y{ -1 }
    , m_back_marker( nullptr, cairo_surface_destroy )
    , m_cursor_type( cursor_names::kDefault )
    , m_render_text{ CONFIG::get_text_rendering_method() == 0 ? &DrawAreaBase::render_text_glyphstring
                                                              : &DrawAreaBase::render_text_pangolayout }
{
#ifdef _DEBUG
    std::cout << "DrawAreaBase::DrawAreaBase " << m_url << std::endl;
#endif

    // フォント設定
    set_fontid( FONT_MAIN );
    set_mailfontid( FONT_MAIL );

    // 文字色
    set_colorid_text( COLOR_CHAR );

    // 背景色
    set_colorid_back( COLOR_BACK );

    // bodyのcssプロパティ
    int classid = CORE::get_css_manager()->get_classid( "body" );
    m_css_body = CORE::get_css_manager()->get_property( classid );
}


DrawAreaBase::~DrawAreaBase()
{
#ifdef _DEBUG
    std::cout << "DrawAreaBase::~DrawAreaBase " << m_url << std::endl;
#endif

    cancel_deceleration();

    clear();
}


//
// セットアップ
//
// show_abone : あぼーんされたスレも表示
// show_scrbar : スクロールバーを最初から表示
// show_multispace : 連続空白も表示
//
void DrawAreaBase::setup( const bool show_abone, const bool show_scrbar, const bool show_multispace )
{
    clear();

    m_article = DBTREE::get_article( m_url );
    m_layout_tree = std::make_unique<LayoutTree>( m_url, show_abone, show_multispace );

    // デフォルトではoffになってるイベントを追加
    m_view.add_events( Gdk::BUTTON_PRESS_MASK );
    m_view.add_events( Gdk::BUTTON_RELEASE_MASK );
    m_view.add_events( Gdk::SCROLL_MASK );
    m_view.add_events( Gdk::SMOOTH_SCROLL_MASK );
    m_view.add_events( Gdk::POINTER_MOTION_MASK );
    m_view.add_events( Gdk::LEAVE_NOTIFY_MASK );
    m_view.add_events( Gdk::VISIBILITY_NOTIFY_MASK );

    // focus 可にセット
    m_view.set_can_focus( true );
    m_view.add_events( Gdk::KEY_PRESS_MASK );
    m_view.add_events( Gdk::KEY_RELEASE_MASK );

    // イベント接続
    m_view.signal_leave_notify_event().connect(  sigc::mem_fun( *this, &DrawAreaBase::slot_leave_notify_event ) );
    m_view.signal_realize().connect( sigc::mem_fun( *this, &DrawAreaBase::slot_realize ));
    m_view.signal_configure_event().connect(  sigc::mem_fun( *this, &DrawAreaBase::slot_configure_event ));
    m_view.signal_draw().connect( sigc::mem_fun( *this, &DrawAreaBase::slot_draw ) );
    m_view.signal_scroll_event().connect(  sigc::mem_fun( *this, &DrawAreaBase::slot_scroll_event ));
    setup_event_controller();
    m_view.signal_motion_notify_event().connect(  sigc::mem_fun( *this, &DrawAreaBase::slot_motion_notify_event ));
    m_view.signal_key_press_event().connect( sigc::mem_fun(*this, &DrawAreaBase::slot_key_press_event ));
    m_view.signal_key_release_event().connect( sigc::mem_fun(*this, &DrawAreaBase::slot_key_release_event ));

    pack_start( m_view );

    // pango layout 作成
    m_pango_layout = m_view.create_pango_layout( "" );
    m_pango_layout->set_width( -1 ); // no wrap

    // フォント初期化
    // 色の初期化は realize したとき
    init_font();

    // スクロールバー作成
    if( show_scrbar ) create_scrbar();

    show_all_children();
}


//
// 背景色のID( colorid.h にある ID を指定)
//
int DrawAreaBase::get_colorid_back() const
{
    if( m_css_body.bg_color >= 0 ) return m_css_body.bg_color;

    return m_colorid_back;
}



//
// 変数初期化
//
void DrawAreaBase::clear()
{
    m_scrollinfo.reset();

    m_selection.select = false;
    m_multi_selection.clear();
    m_layout_current = nullptr;
    m_width_client = 0;
    m_height_client = 0;
    m_clicked = false;
    m_drugging = false;
    m_r_drugging = false;
    m_pre_pos_y = -1;
    m_cancel_change_adjust = false;
    m_key_press = false;
    m_key_locked = false;
    m_keyval = 0;
    m_goto_num_reserve = 0;
    m_goto_bottom_reserve = false;
    m_wheel_scroll_time = 0;
    m_caret_pos = CARET_POSITION();
    m_caret_pos_pre = CARET_POSITION();
    m_caret_pos_dragstart = CARET_POSITION();

    m_drawinfo.draw = false;

    m_rect_backscreen.y = 0;
    m_rect_backscreen.height = 0;

    m_enable_draw = true;

    m_jump_history.clear();

    // 埋め込み画像削除
    m_eimgs.clear();
}



//
// スクロールバー作成とパック
//
void DrawAreaBase::create_scrbar()
{
    if( m_vscrbar ) return;

#ifdef _DEBUG
    std::cout << "DrawAreaBase::create_scrbar\n";
#endif

    // そのままHBoxにスクロールバーをパックすると、スクロールしたときに何故かHBox全体が
    // 再描画されて負荷が高くなるのでEventBoxを間に挟む
    m_vscrbar = Gtk::manage( new Gtk::Scrollbar( Glib::RefPtr<Gtk::Adjustment>{}, Gtk::ORIENTATION_VERTICAL ) );
    m_event = Gtk::manage( new Gtk::EventBox() );
    assert( m_vscrbar );
    assert( m_event );

    if( CONFIG::get_left_scrbar() ) remove( m_view );

    m_event->add( *m_vscrbar );
    pack_start( *m_event, Gtk::PACK_SHRINK );

    if( CONFIG::get_left_scrbar() ) pack_start( m_view );

    m_vscrbar->get_adjustment()->signal_value_changed().connect( sigc::mem_fun( *this, &DrawAreaBase::slot_change_adjust ) );

    show_all_children();
}




//
// 色初期化 ( colorid.h 参照 )
//
void DrawAreaBase::init_color()
{
    const std::vector< std::string >& colors = CORE::get_css_manager()->get_colors();
    const int usrcolor = colors.size();
    m_color.resize( END_COLOR_FOR_THREAD + usrcolor );

    Glib::RefPtr< Gtk::StyleContext > context = get_style_context();

    int i = COLOR_FOR_THREAD +1;
    for( ; i < END_COLOR_FOR_THREAD; ++i ){

        m_color[ i ] = Gdk::RGBA( CONFIG::get_color( i ) );
    }

    // スレビューの文字色、背景色、選択色でGTKテーマの設定を使用する
    if( CONFIG::get_use_select_gtkrc() ){
        bool fg_ok = context->lookup_color( "theme_text_color", m_color[ COLOR_CHAR ] );
        bool bg_ok = context->lookup_color( "theme_base_color", m_color[ COLOR_BACK ] );
        if( !fg_ok || !bg_ok ) {
            MISC::ERRMSG( "DrawAreaBase::init_color: Failed to retrieve GTK theme char colors." );
        }
        fg_ok = context->lookup_color( "theme_selected_fg_color", m_color[ COLOR_CHAR_SELECTION ] );
        bg_ok = context->lookup_color( "theme_selected_bg_color", m_color[ COLOR_BACK_SELECTION ] );
        if( !fg_ok || !bg_ok ) {
            MISC::ERRMSG( "DrawAreaBase::init_color: Failed to retrieve GTK theme selected color." );
        }
    }

    for( const auto& color : colors ) {
        m_color[ i ] = Gdk::RGBA( color );
        ++i;
    }
}



//
// フォント初期化
//
void DrawAreaBase::init_font()
{
    // スレビューで文字幅の近似を厳密にするか
    m_strict_of_char = CONFIG::get_strict_char_width();

    m_context = get_pango_context();
    assert( m_context );

    std::string fontname = CONFIG::get_fontname( m_defaultfontid );
    if( fontname.empty() ) return;

    init_fontinfo( m_defaultfont, fontname );

    if( CONFIG::get_aafont_enabled() ){
        m_aafont_initialized = true;
        std::string aafontname = CONFIG::get_fontname( FONT_AA );

        init_fontinfo( m_aafont, aafontname );

#ifdef _DEBUG
        std::cout << "DrawAreaBase::aa_fontname = " << aafontname << std::endl;
#endif
    }

    std::string mailfontname = CONFIG::get_fontname( m_defaultmailfontid );
    if( ! mailfontname.empty() && mailfontname != fontname ) {
        m_mailfont_initialized = true;
        init_fontinfo( m_mailfont, mailfontname );
    }
    else {
        // メモリ節約のためデフォルトとメール欄のフォントが同じときはデフォルトを利用する
        m_mailfont_initialized = false;
    }

    // layoutにフォントをセット
    m_font = &m_defaultfont;
    m_pango_layout->set_font_description( m_font->pfd );
    m_context->set_font_description( m_font->pfd );
}


//
// フォント情報初期化
//
void DrawAreaBase::init_fontinfo( FONTINFO& fi, std::string& fontname )
{
    fi.fontname = fontname;
    
    // layoutにフォントをセット
    fi.pfd = Pango::FontDescription( fontname );
    fi.pfd.set_weight( Pango::WEIGHT_NORMAL );
    m_pango_layout->set_font_description( fi.pfd );

    // フォント情報取得
    PangoFontMetrics* metrics = pango_context_get_metrics( m_context->gobj(), fi.pfd.gobj(), nullptr );
    fi.ascent = PANGO_PIXELS( pango_font_metrics_get_ascent( metrics ) );
    fi.descent = PANGO_PIXELS( pango_font_metrics_get_descent( metrics ) );
    fi.height = fi.ascent + fi.descent;

    // 改行高さ ( トップからの距離 )
    fi.br_size = static_cast<int>( fi.height * CONFIG::get_adjust_line_space() );

    const char* wstr = "あいうえお";
    m_pango_layout->set_text( wstr );

    // リンクの下線の位置 ( トップからの距離 )
    fi.underline_pos = PANGO_PIXELS( ( pango_font_metrics_get_ascent( metrics )
                                       - pango_font_metrics_get_underline_position( metrics ) )
                                     * CONFIG::get_adjust_underline_pos() );
    pango_font_metrics_unref( metrics );

    // 左右padding取得
    // マージン幅は真面目にやると大変そうなので文字列 wstr の平均を取る
    int width = m_pango_layout->get_pixel_ink_extents().get_width() / 5;

    fi.mrg_right = width /2 * 3;
}


//
// クロック入力
//
void DrawAreaBase::clock_in()
{
    if( m_scrollinfo.mode != SCROLL_NOT &&
        ! ( m_scrollinfo.mode == SCROLL_AUTO || m_scrollinfo.live ) ){

        // スクロール中にダイアログを開いた場合はスクロールされたままになるのでスクロールを止める
        if( SESSION::is_dialog_shown() ) focus_out();

        else exec_scroll();
    }
}


//
// スムーススクロール用クロック入力
//
void DrawAreaBase::clock_in_smooth_scroll()
{
    if( m_scrollinfo.mode == SCROLL_AUTO || m_scrollinfo.live ){

        // スクロール中にダイアログを開いた場合はスクロールされたままになるのでスクロールを止める
        if( ! m_scrollinfo.live && SESSION::is_dialog_shown() ){
            m_scrollinfo.mode = SCROLL_NOT;
            focus_out();
        }

        else exec_scroll();
    }
}


//
// フォーカス
//
void DrawAreaBase::focus_view()
{
    m_view.grab_focus();
}


//
// フォーカス解除
//
void DrawAreaBase::focus_out()
{
    // realize していない
    if( !m_window ) return;

#ifdef _DEBUG
    std::cout << "DrawAreaBase::focus_out\n";
#endif

    change_cursor( cursor_names::kDefault );

    m_key_press = false;
    m_key_locked = false;
    m_keyval = 0;

    if( m_scrollinfo.mode != SCROLL_AUTO ) m_scrollinfo.reset();

}


// 新着セパレータのあるレス番号の取得とセット
int DrawAreaBase::get_separator_new() const
{
    return m_layout_tree->get_separator_new();
}

void DrawAreaBase::set_separator_new( int num )
{
    m_layout_tree->set_separator_new( num );
}

// 新着セパレータを隠す
void DrawAreaBase::hide_separator_new()
{
    if( ! get_separator_new() ) return;

    m_layout_tree->set_separator_new( 0 );
    exec_layout();
}


// 現在のポインタの下にあるレス番号取得
int DrawAreaBase::get_current_res_num() const
{
    const int y = m_y_pointer + get_vscr_val();

    // 先頭のヘッダブロックから順に調べる
    const LAYOUT* header = m_layout_tree->top_header();
    const LAYOUT* next_header = header ? header->next_header : nullptr;
    while( next_header ){

        // y が含まれているブロックを探す
        if( next_header->rect->y >= y ) return header->res_number;

        // 次のブロックへ
        header = next_header;
        next_header = header->next_header;
    }

    return max_number();
}


// 範囲選択中の文字列
std::string DrawAreaBase::str_selection() const
{
    if( ! m_selection.select ) return std::string();

    return m_selection.str;
}


/**
 * @brief 範囲選択の開始位置に対応するレス番号を取得します。
 *
 * 選択範囲の開始位置が通常のレス位置、または「ここまで読んだ」セパレーター、
 * あるいはその親ヘッダーに含まれる場合に対応し、適切なレス番号を返します。
 *
 * @return 選択範囲の開始位置のレス番号。選択されていない場合や、
 * 有効なレス番号が取得できない場合は0を返します。
 */
int DrawAreaBase::get_selection_resnum_from() const
{
    if( ! m_selection.select ) return 0;
    const LAYOUT* layout = m_selection.caret_from.layout;
    if( ! layout ) return 0;

    // 範囲選択の開始位置がレス番号を持っているなら返す。
    if( const int res_number = layout->res_number; res_number ) return res_number;

    const LAYOUT* header = layout->header;
    // 範囲選択の開始位置の親ヘッダーが新着セパレータなら新着セパレータの位置(レス番号)を返す。
    if( header == m_layout_tree->get_separator() ) {
        const int res_number = m_layout_tree->get_separator_new();
        return res_number;
    }
    if( header ) {
        const int res_number = header->res_number;
        return res_number;
    }
    return 0;
}


/**
 * @brief 範囲選択の終了位置に対応するレス番号を取得します。
 *
 * 選択範囲の終了位置が通常のレス位置、または「ここまで読んだ」セパレーター、
 * あるいはその親ヘッダーに含まれる場合に対応し、適切なレス番号を返します。
 * セパレーターに含まれる場合はその直前のレス番号を返します。
 *
 * @return 選択範囲の終了位置のレス番号。選択されていない場合や、
 * 有効なレス番号が取得できない場合は0を返します。
 */
int DrawAreaBase::get_selection_resnum_to() const
{
    if( ! m_selection.select ) return 0;
    const LAYOUT* layout = m_selection.caret_to.layout;
    if( ! layout ) return 0;

    // 範囲選択の終了位置がレス番号を持っているなら返す。
    if( const int res_number = layout->res_number; res_number ) return res_number;

    const LAYOUT* header = layout->header;
    // 範囲選択の終了位置の親ヘッダーが新着セパレータなら新着セパレータの位置(レス番号)の一つ前を返す。
    if( header == m_layout_tree->get_separator() ) {
        const int res_number = m_layout_tree->get_separator_new() - 1;
        return res_number;
    }
    if( header ) {
        const int res_number = header->res_number;
        return res_number;
    }
    return 0;
}



//
// 表示されている最後のレスの番号
//
int DrawAreaBase::max_number() const
{
    assert( m_layout_tree );

    return m_layout_tree->max_res_number();
}


//
// from_num　から to_num までレスをappendして再レイアウト
//
void DrawAreaBase::append_res( const int from_num, const int to_num )
{
    assert( m_article );
    assert( m_layout_tree );

#ifdef _DEBUG
    std::cout << "DrawAreaBase::append_res from " << from_num << " to " << to_num << std::endl;
#endif

    // スクロールバーが一番下にある(つまり新着スレがappendされた)場合は少しだけスクロールする
    bool scroll = false;
    const int pos = get_vscr_val();
    if( ! m_layout_tree->get_separator_new() && pos && pos == get_vscr_maxval() && ! m_scrollinfo.live ){

#ifdef _DEBUG
        std::cout << "on bottom pos = " << pos << std::endl;
#endif
        scroll = true;
    }

    for( int num = from_num; num <= to_num; ++num ) m_layout_tree->append_node( m_article->res_header( num ), false );

    // クライアント領域のサイズをリセットして再レイアウト
    m_width_client = 0;
    m_height_client = 0;
    exec_layout();

    if( scroll ){

        const CORE::CSS_PROPERTY* css = m_layout_tree->get_separator()->css;
        const RECTANGLE* rect = m_layout_tree->get_separator()->rect;

        m_scrollinfo.reset();
        m_scrollinfo.dy = 0;
        if( css ) m_scrollinfo.dy += css->mrg_top + css->mrg_bottom;
        if( rect ) m_scrollinfo.dy += rect->height;
        if( m_scrollinfo.dy ){

#ifdef _DEBUG
            std::cout << "exec scroll dy = " << m_scrollinfo.dy << std::endl;
#endif
            m_scrollinfo.mode = SCROLL_NORMAL;
            exec_scroll();
        }
    }

}



//
// リストで指定したレスをappendして再レイアウト
//
void DrawAreaBase::append_res( const std::list< int >& list_resnum )
{
    std::list< bool > list_joint;
    append_res( list_resnum, list_joint );
}



//
// リストで指定したレスをappendして再レイアウト( 連結情報付き )
//
// list_joint で連結指定したレスはヘッダを取り除いて前のレスに連結する
//
void DrawAreaBase::append_res( const std::list< int >& list_resnum, const std::list< bool >& list_joint )
{
    assert( m_article );
    assert( m_layout_tree );

    if( list_resnum.size() == 0 ) return;

#ifdef _DEBUG
    std::cout << "DrawAreaBase::append_res" << std::endl;
#endif

    const bool use_joint = ( list_joint.size() == list_resnum.size() );

    std::list< bool >::const_iterator it_joint = list_joint.begin();
    for( const int num : list_resnum ) {

        bool joint = false;
        if( use_joint ){
            joint = ( *it_joint );
            ++it_joint;
        }

#ifdef _DEBUG
        std::cout << "append no. " << num << " joint = " << joint << std::endl;
#endif

        m_layout_tree->append_node( m_article->res_header( num ), joint );
    }

    // クライアント領域のサイズをリセットして再レイアウト
    m_width_client = 0;
    m_height_client = 0;
    exec_layout();
}



//
// html をappendして再レイアウト
//
void DrawAreaBase::append_html( const std::string& html )
{
    assert( m_layout_tree );

    if( html.empty() ) return;

#ifdef _DEBUG
    std::cout << "DrawAreaBase::append_html " << html << std::endl;
#endif

    m_layout_tree->append_html( html );

    // クライアント領域のサイズをリセットして再レイアウト
    m_width_client = 0;
    m_height_client = 0;
    exec_layout();
}



//
// datをappendして再レイアウト
//
void DrawAreaBase::append_dat( const std::string& dat, int num )
{
    assert( m_layout_tree );

    if( dat.empty() ) return;

#ifdef _DEBUG
    std::cout << "DrawAreaBase::append_dat " << dat << std::endl;
#endif

    m_layout_tree->append_dat( dat, num );

    // クライアント領域のサイズをリセットして再レイアウト
    m_width_client = 0;
    m_height_client = 0;
    exec_layout();
}


//
// 画面初期化
//
// 色、フォントを初期化して画面を消す
//
void DrawAreaBase::clear_screen()
{
    if( ! m_layout_tree ) return;
    m_layout_tree->clear();
    clear();
    init_color();
    init_font();
    if( exec_layout() ) redraw_view_force();
}



//
// 再描画
// 再レイアウトはしないが configureの予約がある場合は再レイアウトしてから再描画する
//
void DrawAreaBase::redraw_view()
{
    // 起動中とシャットダウン中は処理しない
    if( SESSION::is_booting() ) return;
    if( SESSION::is_quitting() ) return;

    // タブ操作中は処理しない
    if( SESSION::is_tab_operating( URL_ARTICLEADMIN ) ) return;

#ifdef _DEBUG
    std::cout << "DrawAreaBase::redraw_view " << m_url << std::endl;
#endif

    configure_impl();

    // 実行時の警告を修正する
    // Gtk-WARNING **: HH:MM:SS.sss: Drawing a gadget with negative dimensions. Did you forget to allocate a size?
    // (node tab owner gtkmm__GtkNotebook)
    // FIXME: 起動時に隠れているタブのスレビューを開くとこの警告が出る
    const auto alloc_view = m_view.get_allocation();
#ifdef _DEBUG
    std::cout << "DrawAreaBase::redraw_view "
              << " m_view.x = " << alloc_view.get_x()
              << " m_view.y = " << alloc_view.get_y()
              << " m_view.width = " << alloc_view.get_width()
              << " m_view.height = " << alloc_view.get_height()
              << std::endl;
#endif
    if( alloc_view.get_x() < 0 || alloc_view.get_y() < 0 ) return;

    m_view.queue_draw();
}

// 強制再描画
void DrawAreaBase::redraw_view_force()
{
#ifdef _DEBUG
    std::cout << "DrawAreaBase::redraw_view_force()\n";
#endif

    m_drawinfo.draw = false;
    m_rect_backscreen.y = 0;
    m_rect_backscreen.height = 0;
    redraw_view();
}



//
// レイアウト(ノードの座標演算)実行
//
bool DrawAreaBase::exec_layout()
{
    return exec_layout_impl( false, 0 );
}



//
// 先頭ノードから順に全ノードの座標を計算する(描画はしない)
//
// is_popup = true ならポップアップウィンドウの幅を親ビューの幅とする
// offset_y は y 座標の上オフセット行数
//
bool DrawAreaBase::exec_layout_impl( const bool is_popup, const int offset_y )
{
    // 起動中とシャットダウン中は処理しない
    if( SESSION::is_booting() ) return false;
    if( SESSION::is_quitting() ) return false;

    // タブ操作中は処理しない
    if( SESSION::is_tab_operating( URL_ARTICLEADMIN ) ) return true;

    // レイアウトがセットされていない
    if( ! m_layout_tree ) return false;
    if( ! m_layout_tree->top_header() ) return false;

    // drawareaのウィンドウサイズ
    int width_view = m_view.get_width();
    const int height_view = m_view.get_height();

    int width_base_vscrbar = 0;
    if( is_popup && SESSION::get_base_drawarea() && SESSION::get_base_drawarea()->get_vscrbar() ){
        width_base_vscrbar = SESSION::get_base_drawarea()->get_vscrbar()->get_width();
    }

#ifdef _DEBUG
    std::cout << "DrawAreaBase::exec_layout_impl : is_popup = " << is_popup << " url = " << m_url << std::endl;
#endif

    //表示はされてるがまだリサイズしてない状況
    if( height_view < LAYOUT_MIN_HEIGHT ){

        // ポップアップで無い場合は処理しない
        if( ! is_popup ){
#ifdef _DEBUG
            std::cout << "drawarea is not resized yet.\n";
#endif
            return false;
        }

        // ポップアップの場合、横幅が確定出来ないのでメインウィンドウのスレビューの
        // drawarea の幅を最大幅とする
        if( SESSION::get_base_drawarea() && SESSION::get_base_drawarea()->get_view() ){
            width_view = SESSION::get_base_drawarea()->get_view()->get_width();
        }

        // スレビューを表示していない場合はメインウィンドウサイズを使用
        else width_view = SESSION::get_width_win_main();
    }

#ifdef _DEBUG
    std::cout << "width_view = " << width_view << " height_view  = " << height_view << std::endl;
#endif

    // 新着セパレータの挿入
    // 実況時は新着セパレータを表示しない(スクロールがガタガタするから)
    if( ! m_scrollinfo.live ) m_layout_tree->move_separator();

    m_width_client = 0;
    m_height_client = 0;
    int y = 0;
    LAYOUT* header = m_layout_tree->top_header();

    // フォント設定
    set_node_font( header );

    const CORE::Css_Manager *cssmgr = CORE::get_css_manager();
    cssmgr->set_size( &m_css_body, m_font->height );

    y += offset_y * m_font->br_size;
    y += m_css_body.padding_top;

    while( header ){

        LAYOUT* layout = header->next_layout;
        if( ! layout ) break;

        // フォント設定
        set_node_font( header );

        // (注) header は div ノードであり、クラス名は "res"
        // 詳しくは LayoutTree::create_layout_header() を参照せよ
        cssmgr->set_size( header->css, m_font->height );

        // div の位置と幅を計算
        // 高さは子ノードのレイアウトが全て済んでから計算
        if( ! m_pixbuf_bkmk ) m_pixbuf_bkmk = ICON::get_pixbuf( ICON::BKMARK_THREAD ); // 最低でもブックマークのアイコン分だけ左のスペース開ける
        int x = MAX( 1 + m_pixbuf_bkmk->get_width() + 1, m_css_body.padding_left + header->css->mrg_left );
        y += header->css->mrg_top;

        if( ! header->rect ) header->rect = m_layout_tree->create_rect();
        header->rect->x = x;
        header->rect->y = y;
        header->rect->width = width_view - x - ( header->css->mrg_right + m_css_body.padding_right );

        // 内部ノードの開始座標
        x += header->css->padding_left;
        y += header->css->padding_top;

        LAYOUT* current_div = nullptr;
        int br_size = m_font->br_size; // 現在行の改行サイズ

        // 先頭の子ノードから順にレイアウトしていく
        while ( layout ){

            // フォント設定
            set_node_font( layout );

            switch( layout->type ){

                // div (注) div の中に div は作れない仕様になっている( header は除く )
                case DBTREE::NODE_DIV:

                    // 違うdivに切り替わったら以前のdivの高さを更新
                    if( current_div ){

                        y += br_size;
                        br_size = m_font->br_size; // 次の行の改行位置をリセット

                        y += current_div->css->padding_bottom;
                        current_div->rect->height = y - current_div->rect->y;

                        y += current_div->css->mrg_bottom;

                        // align 調整
                        if( current_div->css->align != CORE::ALIGN_LEFT ) set_align( current_div, layout->id, current_div->css->align );
                    }

                    current_div = layout;

                    // div の位置と幅を計算
                    // 高さは違う div に切り替わった時に計算
                    cssmgr->set_size( current_div->css, m_font->height );
                    x = header->rect->x + header->css->padding_left;
                    x += current_div->css->mrg_left;
                    y += current_div->css->mrg_top;

                    if( ! current_div->rect ) current_div->rect = m_layout_tree->create_rect();
                    current_div->rect->x = x;
                    current_div->rect->y = y;
                    current_div->rect->width = header->rect->width
                    - ( header->css->padding_left + current_div->css->mrg_left )
                    - ( header->css->padding_right + current_div->css->mrg_right );

                    // divの内部にあるノードの開始座標
                    x += current_div->css->padding_left;
                    y += current_div->css->padding_top;

                    break;

                    //////////////////////////////////////////

                case DBTREE::NODE_IDNUM: // 発言数ノード

                    if( ! set_num_id( layout ) ) break;
                    [[fallthrough]];

                case DBTREE::NODE_TEXT: // テキスト
                case DBTREE::NODE_LINK: // リンク

                    // ノードをレイアウトして次のノードの左上座標を計算
                    // x,y,br_size が参照なので更新された値が戻る
                    layout_one_text_node( layout, x, y, br_size, width_view );

                    break;

                    //////////////////////////////////////////

                case DBTREE::NODE_IMG: // img

                    // レイアウトして次のノードの左上座標を計算
                    // x,y, br_size が参照なので更新された値が戻る
                    layout_one_img_node( layout, x, y, br_size, m_width_client, is_popup, EIMG_MRG, EIMG_MRG );

                    break;

                    //////////////////////////////////////////

                case DBTREE::NODE_SSSP: // sssp アイコン

                    // テキストノードの途中にインラインで表示する
                    // x,y, br_size が参照なので更新された値が戻る
                    layout_one_img_node( layout, x, y, br_size, m_width_client, is_popup, 0, EIMG_SSSP_FULL );

                    // 上下マージンあわせた分の余白を、下マージン ( br_size ) に確保しておく
                    // 上マージンの位置は、後でベースライン合わせするときに再調整する
                    break;

                    //////////////////////////////////////////

                case DBTREE::NODE_HR: // 水平線

                    if( ! layout->rect ) layout->rect = m_layout_tree->create_rect();
                    layout->rect->x = layout->div ? layout->div->rect->x + layout->div->css->padding_left : 0;
                    layout->rect->y = y + br_size;
                    layout->rect->width = layout->div ? layout->div->rect->width - layout->div->css->padding_left : width_view;
                    layout->rect->height = 1;

                    y += 2; // 水平線1px + 余白1px

                    [[fallthrough]];

                    //////////////////////////////////////////

                case DBTREE::NODE_BR: // 改行

                    x = 0;
                    if( layout->div ) x = layout->div->rect->x + layout->div->css->padding_left;
                    y += br_size;
                    br_size = m_font->br_size; // 次の行の改行位置をリセット

                    break;

                    //////////////////////////////////////////

                case DBTREE::NODE_ZWSP: // 幅0スペース
                    break;

                    //////////////////////////////////////////

                case DBTREE::NODE_HTAB: // 水平タブ
                    x += m_font->height * SPACE_TAB;
                    break;
            }

            // クライアント領域の幅を更新
            if( layout->type != DBTREE::NODE_DIV && layout->rect ){

                int width_tmp = layout->rect->x + layout->rect->width;
                width_tmp += ( header->css->padding_right + header->css->mrg_right );

                // divの中なら右スペースの分も足す
                if( layout->div ) width_tmp += ( layout->div->css->padding_right + layout->div->css->mrg_right );

                width_tmp += m_font->mrg_right + m_css_body.padding_right;
                if( width_tmp > width_view ) width_tmp = width_view;

                width_tmp += width_base_vscrbar;

                if( width_tmp > m_width_client ) m_width_client = width_tmp;
            }

            layout = layout->next_layout;
        }

        y += br_size;

        // 同じ行にあるノードのベースラインを調整
        adjust_layout_baseline( header );

        // 属している div の高さ確定
        if( current_div ){
            y += current_div->css->padding_bottom;
            current_div->rect->height = y - current_div->rect->y;
            y += current_div->css->mrg_bottom;

            // align 調整
            if( current_div->css->align != CORE::ALIGN_LEFT ) set_align( current_div, 0, current_div->css->align );
        }

        // ヘッダブロック高さ確定
        y += header->css->padding_bottom;
        header->rect->height = y - header->rect->y;
        if( header->next_header ) y += header->css->mrg_bottom;

        // align 調整
        if( header->css->align != CORE::ALIGN_LEFT ) set_align( header, 0, header->css->align );

        header = header->next_header;
    }

    y += m_css_body.padding_bottom;

    // クライアント領域の高さ確定
    m_height_client = y;

#ifdef _DEBUG
    std::cout << "virtual size of drawarea : m_width_client = " << m_width_client
              << " m_height_client = " << m_height_client << std::endl;
#endif

    // 実際に画面に表示されてない
    if( ! is_drawarea_realized() ){
#ifdef _DEBG
        std::cout << "windows is not shown yet\n";
#endif
        return false;
    }

    // 表示はされてるがまだリサイズしてない状況
    if( height_view < LAYOUT_MIN_HEIGHT ){
#ifdef _DEBG
        std::cout << "windows is not resized yet\n";
#endif
        return false;
    }

    // ポップアップなどでスクロールバーが表示されていないならここで作成
    // (注) メインウィンドウのスレビューなどは DrawAreaBase::setup() の show_scrbar
    // が true 指定されているので、はじめからスクロールバーが表示されている
    if( ! m_vscrbar && m_height_client > height_view ) create_scrbar();

    // adjustment 範囲変更
    auto adjust = m_vscrbar ? m_vscrbar->get_adjustment()
                            : decltype( m_vscrbar->get_adjustment() )( nullptr );
    if( adjust ){

        const double current = adjust->get_value();
        const double newpos = MAX( 0, MIN( m_height_client - height_view , current ) );

        m_pre_pos_y = -1;

        adjust->set_lower( 0 );
        adjust->set_upper( m_height_client );
        adjust->set_page_size( height_view );
        adjust->set_step_increment( m_font->br_size );
        adjust->set_page_increment(  height_view / 2 );

        m_cancel_change_adjust = true;
        adjust->set_value( newpos );
        m_cancel_change_adjust = false;
    }

    // 裏描画画面作成と初期描画
#ifdef _DEBUG
    std::cout << "create backscreen : width = " << m_view.get_width() << " height = " << m_view.get_height() << std::endl;
#endif

    m_backscreen.reset( gdk_window_create_similar_surface(
        m_window->gobj(), CAIRO_CONTENT_COLOR, m_view.get_width(), m_view.get_height() ) );

    m_rect_backscreen.y = 0;
    m_rect_backscreen.height = 0;

    m_back_frame_top.reset( gdk_window_create_similar_surface(
        m_window->gobj(), CAIRO_CONTENT_COLOR, m_view.get_width(), WIDTH_FRAME ) );
    m_back_frame_bottom.reset( gdk_window_create_similar_surface(
        m_window->gobj(), CAIRO_CONTENT_COLOR, m_view.get_width(), WIDTH_FRAME ) );

    m_ready_back_frame = false;

    // 予約されているならジャンプ予約を実行
    if( m_goto_num_reserve ) goto_num( m_goto_num_reserve );
    if( m_goto_bottom_reserve ) goto_bottom();

    return true;
}


//
// 同じ行にあるノードのベースラインを調整
//
void DrawAreaBase::adjust_layout_baseline( LAYOUT* header )
{
    int top_y = -1, max_height = 0;
    LAYOUT* line_layout = nullptr;
    bool line_onsssp = false;

    LAYOUT* layout = header->next_layout;
    while ( layout ){

        switch( layout->type ){
        case DBTREE::NODE_IDNUM: // 発言数ノード
        case DBTREE::NODE_TEXT: // テキスト
        case DBTREE::NODE_LINK: // リンク
        case DBTREE::NODE_SSSP: // sssp アイコン
            break;
        default:
            layout = layout->next_layout;
            continue;
        }

        RECTANGLE* rect = layout->rect;
        while ( rect ){

            // 行ごとにベースラインを調整する
            if( top_y < rect->y ){

                if( line_onsssp ){
                    max_height += EIMG_SSSP_MRG; // ssspアイコンの上マージンを加える
                }

                while ( line_layout ){

                    switch( line_layout->type ){
                    case DBTREE::NODE_IDNUM: // 発言数ノード
                    case DBTREE::NODE_TEXT: // テキスト
                    case DBTREE::NODE_LINK: // リンク
                    case DBTREE::NODE_SSSP: // sssp アイコン
                        break;
                    default:
                        line_layout = line_layout->next_layout;
                        continue;
                    }

                    RECTANGLE* line_rect = line_layout->rect;
                    while ( line_rect ){
                        if( line_rect == rect ){
                            goto NEXTLINE;
                        }
                        if( line_rect->y == top_y ){
                            line_rect->y = top_y + max_height - line_rect->height;
                        }
                        line_rect = line_rect->next_rect;
                    }
                    line_layout = line_layout->next_layout;
                }
NEXTLINE:
                top_y = rect->y;
                max_height = rect->height;
                line_layout = layout; // 行ごとにノードの先頭を覚えておく
                line_onsssp = false;
            }
            if( max_height < rect->height ){
                max_height = rect->height;
            }
            if( layout->type == DBTREE::NODE_SSSP ){ // sssp アイコン
                line_onsssp = true;
            }
            rect = rect->next_rect;
        }
        layout = layout->next_layout;
    }

    // 最終行の調整を行う
    if( line_onsssp ){
        max_height += EIMG_SSSP_MRG; // ssspアイコンの上マージンを加える
    }

    while ( line_layout ){

        switch( line_layout->type ){
        case DBTREE::NODE_IDNUM: // 発言数ノード
        case DBTREE::NODE_TEXT: // テキスト
        case DBTREE::NODE_LINK: // リンク
        case DBTREE::NODE_SSSP: // sssp アイコン
            break;
        default:
            line_layout = line_layout->next_layout;
            continue;
        }

        RECTANGLE* line_rect = line_layout->rect;
        while ( line_rect ){
            if( line_rect->y == top_y ){
                line_rect->y = top_y + max_height - line_rect->height;
            }
            line_rect = line_rect->next_rect;
        }
        line_layout = line_layout->next_layout;
    }
}


//
// ブロック要素のalign設定
//
// id_end == 0 の時は最後のノードまでおこなう
//
void DrawAreaBase::set_align( LAYOUT* div, int id_end, int align )
{
#ifdef _DEBUG
//    std::cout << "DrawAreaBase::set_align width = " << div->rect->width << std::endl;
#endif

    LAYOUT* layout_from = nullptr;
    LAYOUT* layout_to = nullptr;
    RECTANGLE* rect_from = nullptr;
    RECTANGLE* rect_to = nullptr;

    LAYOUT* layout = div->next_layout;
    int width_line = 0;
    while( layout && ( ! id_end || layout->id != id_end ) ){

        RECTANGLE* rect = layout->rect;
        while( rect ){

            if( ! layout_from ){
                layout_from = layout;
                rect_from = rect;
                width_line = 0;
            }

            layout_to = layout;
            rect_to = rect;
            width_line += rect->width;

#ifdef _DEBUG
//            std::cout << "id = " << layout->id << " w = " << width_line << std::endl;
#endif

            if( rect->end ) break;

            // wrap
            set_align_line( div, layout_from, layout_to, rect_from, rect_to, width_line, align );
            layout_from = nullptr;

            rect = rect->next_rect;
        }

        // 改行
        if( layout_from && ( ! layout->next_layout || layout->type == DBTREE::NODE_BR || layout->id == id_end -1 ) ){
            set_align_line( div, layout_from, layout_to, rect_from, rect_to, width_line, align );
            layout_from = nullptr;
        }

        layout = layout->next_layout;
    }
}

void DrawAreaBase::set_align_line( LAYOUT* div, LAYOUT* layout_from, LAYOUT* layout_to, RECTANGLE* rect_from, RECTANGLE* rect_to,
                                   int width_line, int align )
{
    int padding = div->rect->width - div->css->padding_left - div->css->padding_right - width_line;

    if( align == CORE::ALIGN_CENTER ) padding /= 2;

#ifdef _DEBUG
//    std::cout << "from = " << layout_from->id << " padding = " << padding << std::endl;
#endif

    for(;;){

        bool break_for = ( layout_from == layout_to && rect_from == rect_to );

        if( rect_from ){
            rect_from->x += padding;
            rect_from = rect_from->next_rect;
        }
        if( ! rect_from ){
            layout_from = layout_from->next_layout;
            if( layout_from ) rect_from = layout_from->rect;
        }

        if( break_for ) break;
    }
}



//
// テキストノードの座標を計算する関数
//
// x,y (参照)  : ノードの初期座標(左上)を渡して、次のノードの左上座標が入って返る
// br_size : 改行量
// width_view : 描画領域の幅
//
void DrawAreaBase::layout_one_text_node( LAYOUT* layout, int& x, int& y, int& br_size, const int width_view )
{
    if( ! layout->lng_text ) layout->lng_text = strlen( layout->text );

    int byte_to = layout->lng_text;
    LAYOUT* div = layout->div;

    // wrap 処理用の閾値計算
    // x が border よりも右に来たら wrap する
    int border = 0;
    if( div ) border = div->rect->x + div->rect->width - div->css->padding_right;
    else border = width_view;
    border -= m_font->mrg_right;

    // 先頭の RECTANGLE型のメモリ確保
    // wrapが起きたらまたRECTANGLE型のメモリを確保してリストの後ろに繋ぐ
    bool head_rect = true;
    RECTANGLE* rect = layout->rect;

    int pos_start = 0;
    for(;;){

        // 横に何文字並べるか計算
        char pre_char = 0;
        bool draw_head = true; // 先頭は最低1文字描画
        int pos_to = pos_start;
        int width_line = 0;
        int n_byte = 0;
        int n_ustr = 0; // utfで数えたときの文字数

        // この文字列の全角/半角モードの初期値を決定
        bool wide_mode = set_init_wide_mode( layout->text, pos_start, byte_to );

        // 右端がはみ出るまで文字を足していく
        while( pos_to < byte_to
                && ( ! is_wrapped( x + PANGO_PIXELS( width_line ), border, layout->text + pos_to )
                     || draw_head  ) ) {

            int byte_char;
            width_line += get_width_of_one_char( layout->text + pos_to, byte_char, pre_char, wide_mode, get_layout_fontid( layout ) );
            pos_to += byte_char;
            n_byte += byte_char;
            ++n_ustr;
            draw_head = false;
        }

        // 幅確定
        width_line = PANGO_PIXELS( width_line );
        if( ! width_line ) break;
        if( layout->bold ) ++width_line;

        // RECTANGLEのメモリ確保
        if( head_rect ){ // 先頭
            if( ! rect ) rect = layout->rect = m_layout_tree->create_rect();
            head_rect = false;
        }
        else{ // wrap したので次のRECTANGLEを確保してリストで繋ぐ
            rect->end = false;
            if( ! rect->next_rect ) rect->next_rect = m_layout_tree->create_rect();
            rect = rect->next_rect;
        }

        // 座標情報更新
        rect->end = true;
        rect->x = x;
        rect->y = y;
        rect->width = width_line;
        rect->height = m_font->br_size;
        rect->pos_start = pos_start;
        rect->n_byte = n_byte;
        rect->n_ustr = n_ustr;

#ifdef _DEBUG
//        std::cout << do_draw << " " << layout->id_header << " " << layout->id
//                  << " x = " << layout->rect->x << " y = " << layout->rect->y
//                  << " w = " << layout->rect->width << " h = " << layout->rect->height << std::endl;
#endif

        // 一文字でも書いたので、改行位置をフォントにあわせて更新
        br_size = m_font->br_size;

        x += rect->width;
        if( pos_to >= byte_to ) break;

        // wrap 処理
        x = div ? div->rect->x + div->css->padding_left : 0;
        y += br_size;

        pos_start = pos_to;
    }

    // 次のノードが改行でなければ wrap するか確認する
    if( layout->next_layout && layout->next_layout->type != DBTREE::NODE_BR
            && is_wrapped( x, border, " " ) ){
        x = div ? div->rect->x + div->css->padding_left : 0;
        y += br_size;
        br_size = m_font->br_size; // 次の行の改行位置をリセット
    }
}



//
// 画像ノードの座標を計算する関数
//
// x,y (参照)  : ノードの初期座標(左上)を渡して、次のノードの左上座標が入って返る
// br_size : 現在行での改行量
// width_view : 描画領域の幅
// init_popupwin : ポップアップウィンドウの初期サイズ計算をおこなう
// mrg_right : 右マージン
// mrg_bottom : 下マージン
//
void DrawAreaBase::layout_one_img_node( LAYOUT* layout, int& x, int& y, int& br_size, const int width_view,
                                        const bool init_popupwin, const int mrg_right, const int mrg_bottom )
{
#ifdef _DEBUG
    std::cout << "DrawAreaBase::layout_one_img_node link = " << layout->link << std::endl;
#endif

    DBTREE::NODE* node = layout->node;
    if( ! node ) return;

    // 座標とサイズのセット
    RECTANGLE* rect = layout->rect;
    if( ! rect ) rect = layout->rect = m_layout_tree->create_rect();
    rect->x = x;
    rect->y = y;
    rect->width = EIMG_ICONSIZE + 2; // +2 は枠の分
    rect->height = EIMG_ICONSIZE + 2; // +2 は枠の分

    // 既に表示済みの場合
    DBIMG::Img* img = node->linkinfo->img;
    if( !img && init_popupwin ) img = node->linkinfo->img = DBIMG::get_img( layout->link );
    if( img && img->is_cached() ){
        rect->width = img->get_width_emb() + 2; // +2 は枠の分
        rect->height = img->get_height_emb() + 2; // +2 は枠の分
    }

#ifdef _DEBUG
    std::cout << "x = " << rect->x << " y = " << rect->y << " w = " << rect->width << " h = " << rect->height << std::endl;
#endif

    // wrap 処理用の閾値計算
    // x が border よりも右に来たら wrap する
    LAYOUT* div = layout->div;
    int border = 0;
    if( div ) border = div->rect->x + div->rect->width - div->css->padding_right;
    else border = width_view;

    // wrap
    if( x + ( rect->width + EIMG_MRG ) >= border ){

        x = 0;
        if( div ) x = div->rect->x + div->css->padding_left;
        y += br_size;
        br_size = m_font->br_size; // 次の行の改行位置をリセット

        rect->x = x;
        rect->y = y;
    }

    x += rect->width + mrg_right;
    if( br_size < rect->height + mrg_bottom ){
        br_size = rect->height + mrg_bottom;
    }
}



//
// 文字列の全角/半角モードの初期値を決定する関数
//
bool DrawAreaBase::set_init_wide_mode( const char* str, const int pos_start, const int pos_to )
{
    if( ! m_strict_of_char ) return false;

    bool wide_mode = true;
    int i = pos_start;
    while( i < pos_to ){

        const int byte_tmp = MISC::utf8bytes( str + i );

        // 文字列に全角が含まれていたら全角モードで開始
        if( byte_tmp != 1 ) break;

        // アルファベットが含まれていたら半角モードで開始
        if( IS_ALPHABET( str[ i ] ) ){
            wide_mode = false;
            break;
        }

        // 数字など、全てアルファベットと全角文字以外の文字で出来ていたら
        // 全角モードにする
        i += byte_tmp;
    }

    return wide_mode;
}


//
// 一文字の幅を取得
//
// utfstr : 入力文字 (UTF-8)
// byte   : 長さ(バイト) utfstr が ascii なら 1, UTF-8 なら 2 or 3 or 4 を入れて返す
// pre_char : 前の文字( 前の文字が全角なら = 0 )
// wide_mode :  全角半角モード( アルファベット以外の文字ではモードにしたがって幅を変える )
// mode : fontid.h で定義されているフォントのID
//
int DrawAreaBase::get_width_of_one_char( const char* utfstr, int& byte, char& pre_char, bool& wide_mode, const int mode )
{
    int width = 0;
    int width_wide = 0;

    const char32_t code = MISC::utf8toutf32( utfstr, byte );

    if( ! byte ){
#ifdef _DEBUG
        std::cout << "DrawAreaBase::get_width_of_one_char "
                  << "invalid char " << static_cast<unsigned char>(utfstr[ 0 ])
                  << " " << static_cast<unsigned char>(utfstr[ 1 ])
                  << " " << static_cast<unsigned char>(utfstr[ 2 ])
                  << " " << static_cast<unsigned char>(utfstr[ 3 ]) << std::endl;
#endif
        byte = 1;
        return 0;
    }

    // キャッシュに無かったら幅を調べてキャッシュに登録
    if( ! ARTICLE::get_width_of_char( code, pre_char, width, width_wide, mode ) ){

        const std::string tmpchar( utfstr, byte );

#ifdef _DEBUG
        std::cout << "no cache [" << tmpchar << "] " << byte <<" byte ";
        if( pre_char == 0 ) std::cout << " p =[0] ";
        else if( pre_char > 0 ) std::cout << " p =[" << pre_char << "] ";
#endif

        // 厳密な幅計算をしない場合
        if( ! m_strict_of_char ){

            m_pango_layout->set_text( tmpchar );
            width = width_wide = m_pango_layout->get_logical_extents().get_width();
        }

        // 厳密に幅計算する場合
        else{

            // 全角モードでの幅
            if( ! width_wide ){

                const std::string str_dummy( "ぁ" );
                m_pango_layout->set_text( str_dummy + str_dummy );
                int width_dummy = m_pango_layout->get_logical_extents().get_width() / 2;

                m_pango_layout->set_text( str_dummy + tmpchar  );
                width_wide = m_pango_layout->get_logical_extents().get_width() - width_dummy;

                if( byte != 1 ) width = width_wide;
            }

            // 半角モードでの幅
            // 半角モードではひとつ前の文字によって幅が変わることに注意する
            if( ! width ){

                std::string char_dummy( "a" );
                if( pre_char && IS_ALPHABET( pre_char ) ) char_dummy[ 0 ] = pre_char;

                const std::string str_dummy( char_dummy + char_dummy );
                m_pango_layout->set_text( str_dummy );
                int width_dummy = m_pango_layout->get_logical_extents().get_width() / 2;

                const std::string str_tmp( char_dummy + tmpchar );
                m_pango_layout->set_text( str_tmp );
                width = m_pango_layout->get_logical_extents().get_width() - width_dummy;

#ifdef _DEBUG
                std::cout << " dummy = " << str_dummy << " dw = " << PANGO_PIXELS( width_dummy );
                std::cout << " str = " << str_tmp << " dw = " << PANGO_PIXELS( m_pango_layout->get_logical_extents().get_width() );
#endif
            }
        }

#ifdef _DEBUG
        std::cout << " w = " << PANGO_PIXELS( width ) << " wide = " << PANGO_PIXELS( width_wide ) << std::endl;
#endif

        // フォントが無い
        if( width_wide <= 0 ){

#ifdef _DEBUG
            std::cout << "DrawAreaBase::get_width_of_one_char "
                      << "byte = " << byte
                      << " code = " << MISC::utf32tostr( code )
                      << " [" << tmpchar << "]\n";
#endif

            if( ( code < 0xE000 || code > 0xF8FF ) // 基本面私用領域ではない
                && ( code < 0xF0000 || code > 0x10FFFF ) // 私用面ではない
              ){
                std::stringstream ss_err;
                ss_err << "unknown font byte = " << byte << " utf32 = " << MISC::utf32tostr( code )
                       << " width = " << width;

                MISC::ERRMSG( ss_err.str() );
            }

            ARTICLE::set_width_of_char( code, pre_char, -1, -1, mode );
            width = width_wide = 0;
        }
        else ARTICLE::set_width_of_char( code, pre_char, width, width_wide, mode );
    }

    int ret = 0;

    // 厳密に計算しない場合
    if( ! m_strict_of_char ) ret = width_wide;

    else{

        // 全角文字
        if( byte != 1 ){

            ret = width_wide;
            pre_char = 0;
            wide_mode = true;
        }

        // 半角文字
        else{

            ret = width;
            pre_char = utfstr[ 0 ];

            // アルファベットならモードを半角モードに変更
            if( IS_ALPHABET( utfstr[ 0 ] ) ) wide_mode = false;

            // アルファベット以外の文字では現在の全角/半角モードに
            // したがって幅を変える。モードは変更しない
            else if( wide_mode ) ret = width_wide;
        }
    }

    return ret;
}


//
// スクリーン描画
//
// y から height の高さ分だけ描画する
// height == 0 ならスクロールした分だけ描画( y は無視 )
//
bool DrawAreaBase::draw_screen( const int y, const int height )
{
    if( ! m_enable_draw ) return false;
    if( ! m_backscreen ) return false;
    if( ! m_layout_tree ) return false;
    if( ! m_layout_tree->top_header() ) return false;
    if( ! m_window ) return false;
    if( m_view.get_height() < LAYOUT_MIN_HEIGHT ) return false; // まだ画面に表示されていない

    // スクロールしていない
    if( ! height && m_pre_pos_y != -1 ){

        const int pos_y = get_vscr_val();
        const int dy = pos_y - m_pre_pos_y;
        if( ! dy ) return false;
    }

    // キューに expose イベントが溜まっている時は全画面再描画
    auto* const updated = gdk_window_get_update_area( m_window->gobj() );
    if( updated ) {
        cairo_region_destroy( updated );
        redraw_view_force();
        return true;
    }

#ifdef _DEBUG
    std::cout << "DrawAreaBase::draw_screen "
              << " y = " << y << " height " << height << std::endl;
#endif

    m_drawinfo.draw = true;
    m_drawinfo.y = y;
    m_drawinfo.height = height;

    // expose イベント経由で exec_draw_screen() を呼び出す
    // gtk2.18以降は expose イベント内で描画処理しないと正しく描画されない様なので注意
    m_view.queue_draw();
    return true;
}


void DrawAreaBase::exec_draw_screen( const int y_redraw, const int height_redraw )
{
    const int width_view = m_view.get_width();
    const int height_view = m_view.get_height();
    const int pos_y = get_vscr_val();

    // 全画面再描画

    // 画面上の描画開始領域のy座標と高さ
    constexpr int y_screen = 0;
    const int height_screen = height_view;

    // 描画範囲の上限、下限
    const int upper = pos_y;
    const int lower = pos_y + height_screen;

#ifdef _DEBUG
    std::cout << "DrawAreaBase::exec_draw_screen "
              << " y_redraw = " << y_redraw
              << " height_redraw " << height_redraw << std::endl
              << "pos_y = " << pos_y
              << " y_screen = " << y_screen << " h_screen = " << height_screen
              << " upper = " << upper << " lower = " << lower
              << " scrollmode = " << m_scrollinfo.mode
              << " url = " << m_url
              << std::endl;
#endif

    m_pre_pos_y = pos_y;

    m_rect_backscreen.x = 0;
    m_rect_backscreen.y = y_screen;
    m_rect_backscreen.width = width_view;
    m_rect_backscreen.height = height_screen;

    // 一番最後のレスが半分以上表示されていたら最後のレス番号をm_seen_currentにセット
    m_seen_current = 0;
    const int max_res_number = m_layout_tree->max_res_number();
    int num = max_res_number;
    const LAYOUT* lastheader = m_layout_tree->get_header_of_res_const( num );

    // あぼーんなどで表示されていないときは前のレスを調べる
    if( !lastheader ){
        while( ! m_layout_tree->get_header_of_res_const( num ) && num-- > 0 );
        lastheader = m_layout_tree->get_header_of_res_const( num );
    }
    if( lastheader && lastheader->rect
            && lastheader->rect->y + lastheader->rect->height/2 < pos_y + height_view ){

        m_seen_current = m_layout_tree->max_res_number();
    }

    const auto tv_before = Monotonic::now();

    // 2分探索で画面に表示されているノードの先頭を探す
    LAYOUT* header = nullptr;
    int top = 1;
    int back = max_res_number;
    while( top <= back ){

        const int pivot = ( top + back )/2;

        header = m_layout_tree->get_header_of_res( pivot );
        if( ! header ) break;

        if( ! header->rect ) {
            top = pivot + 1;
            header = nullptr;
            continue;
        }

/*
        std::cout << "top = " << top << " back = " << back << " pivot = " << pivot
                  << " pos_y = " << pos_y << " y = " << header->rect->y
                  << " y + height = " << header->rect->y + header->rect->height
                  << std::endl;
*/

        if( header->next_header ){

            if( header->rect->y <= pos_y && header->next_header->rect->y >= pos_y )  break;
        }
        else{ // 最後のレス

            if( header->rect->y <= pos_y ) break;
        }

        if( header->rect->y > pos_y ) back = pivot -1;
        else top = pivot + 1;

        header = nullptr;
    }
    if( ! header ){
#ifdef _DEBUG
        std::cout << "not found\n";
#endif
        header = m_layout_tree->top_header();
    }

    // バックスクリーンの背景クリア
    fill_backscreen( get_colorid_back(), 0, y_screen, width_view, height_screen );

    // 描画ループ
    CLIPINFO ci = { width_view, pos_y, upper, lower }; // 描画領域
    bool relayout = false;
    while( header ) {
        if( ! header->rect ) {
            header = header->next_header;
            continue;
        }
        if( header->rect->y >= pos_y + height_view ) {
            break;
        }

        // フォント設定
        set_node_font( header );

        // 現在みているレス番号取得
        if( ! m_seen_current ){

            if( ! header->next_header // 最後のレス
                || ( header->next_header->rect->y >= ( pos_y + m_font->br_size ) // 改行分下にずらす
                     && header->rect->y <= ( pos_y + m_font->br_size ) ) ){

                m_seen_current = header->res_number;
            }
        }

        // ヘッダが描画範囲に含まれてるなら描画
        if( header->rect->y + header->rect->height > upper && header->rect->y < lower ){

            // ノードが描画範囲に含まれてるなら描画
            LAYOUT* layout  = header;
            while ( layout ){

                // フォント設定
                set_node_font( layout );

                RECTANGLE* rect = layout->rect;
                while( rect ){

                    if( rect->y + rect->height > upper && rect->y < lower ){
                        if( draw_one_node( layout, ci ) ) relayout = true;
                        break;
                    }

                    rect = rect->next_rect;
                }

                layout = layout->next_layout;
            }
        }

        header = header->next_header;
    }

    // 処理落ちが起きていないかチェックする
    // exec_scroll()を参照せよ
    if( m_wait_scroll == Monotonic::duration::zero() ) {

        const auto tv_after = Monotonic::now();
        const auto passed = tv_after - tv_before;
        if( passed > std::chrono::milliseconds{ TIMER_TIMEOUT } ) {

            m_scroll_time = tv_after;
            m_wait_scroll = std::chrono::milliseconds{ TIMER_TIMEOUT };

#ifdef _DEBUG
            std::cout << "DrawAreaBase::draw_screen_core : passed = " << passed.count()
                      << " wait = " << m_wait_scroll.count() << std::endl;
#endif
        }
    }

    // 再レイアウト & 再描画
    if( relayout ){

#ifdef _DEBUG
        std::cout << "relayout\n";
#endif

        if( exec_layout() ){
            redraw_view_force();
            return;
        }
    }

    // 前回描画したオートスクロールマーカを消す
    if( m_ready_back_marker ){

        m_ready_back_marker = false;

        cairo_save( m_cr );
        cairo_rectangle( m_cr, m_clip_marker.x, m_clip_marker.y, m_clip_marker.width, m_clip_marker.height );
        cairo_clip( m_cr );
        cairo_set_source_surface( m_cr, m_back_marker.get(), m_clip_marker.x, m_clip_marker.y );
        cairo_paint( m_cr );
        cairo_restore( m_cr );
    }

    // 前回描画したフレームを消す
    if( m_ready_back_frame ){

        m_ready_back_frame = false;

        cairo_save( m_cr );
        cairo_rectangle( m_cr, 0.0, 0.0, width_view, height_view );
        cairo_clip( m_cr );
        cairo_set_source_surface( m_cr, m_back_frame_top.get(), 0.0, 0.0 );
        cairo_paint( m_cr );
        cairo_set_source_surface( m_cr, m_back_frame_bottom.get(), 0.0, height_view - WIDTH_FRAME );
        cairo_paint( m_cr );
        cairo_restore( m_cr );
    }

    // バックスクリーンをウィンドウにコピー
    cairo_save( m_cr );
    cairo_rectangle( m_cr, 0.0, y_screen, width_view, height_screen );
    cairo_clip( m_cr );
    cairo_set_source_surface( m_cr, m_backscreen.get(), 0.0, 0.0 );
    cairo_paint( m_cr );
    cairo_restore( m_cr );

    // オートスクロールマーカと枠の描画
    draw_marker();
    draw_frame();
}


//
// ノードひとつを描画する関数
//
// width_view : 描画領域の幅
// pos_y : 描画領域の開始座標
//
// 戻り値 : true なら描画後に再レイアウトを実行する
//
bool DrawAreaBase::draw_one_node( LAYOUT* layout, const CLIPINFO& ci )
{
    bool relayout = false;

    if( ! m_article ) return relayout;

    // ノード種類別の処理
    switch( layout->type ){

        // div
        case DBTREE::NODE_DIV:
            draw_div( layout, ci );
            break;


            //////////////////////////////////////////


            // リンクノード
        case DBTREE::NODE_LINK:

            // 画像リンクの場合、実際にリンクが表示される段階でノードツリーに DBIMG::Img
            // のポインタと色をセットする。
            //
            // 結合度が激しく高くなるがスピードを重視
            //
            if( layout->node && layout->node->linkinfo->image ){

                DBTREE::NODE* node = layout->node;

                // 画像クラスのポインタ取得してノードツリーにセット
                if( ! node->linkinfo->img ){
                    node->linkinfo->img = DBIMG::get_img( layout->node->linkinfo->link );
                }

                // 画像クラスが取得されてたら色を指定
                DBIMG::Img* img = node->linkinfo->img;
                if( img ){

                    if( img->is_loading() ) node->color_text = COLOR_IMG_LOADING;
                    else if( img->is_wait() ) node->color_text = COLOR_IMG_LOADING;
                    else if( img->get_code() == HTTP_OK ) node->color_text = COLOR_IMG_CACHED;
                    else if( img->get_code() == HTTP_INIT ){
                        if( img->get_abone() ) node->color_text = COLOR_IMG_ERR;
                        else node->color_text = COLOR_IMG_NOCACHE;
                    }
                    else node->color_text = COLOR_IMG_ERR;
                }
            }
            [[fallthrough]];


            //////////////////////////////////////////


            // テキストノード
        case DBTREE::NODE_TEXT:

            draw_one_text_node( layout, ci );
            break;


            //////////////////////////////////////////


            // 発言回数ノード
        case DBTREE::NODE_IDNUM:

            if( set_num_id( layout ) ) draw_one_text_node( layout, ci );
            break;


            //////////////////////////////////////////


        // ヘッダ
        case DBTREE::NODE_HEADER:

            draw_div( layout, ci );

            if( layout->res_number ){

                const int y_org = layout->rect->y + layout->css->padding_top;
                int y = y_org;

                // ブックマークのマーク描画
                if( m_article->is_bookmarked( layout->res_number ) ){

                    if( ! m_pixbuf_bkmk ) m_pixbuf_bkmk = ICON::get_pixbuf( ICON::BKMARK_THREAD );
                    const int height_bkmk = m_pixbuf_bkmk->get_height();

                    y += ( m_font->height - height_bkmk ) / 2;

                    const int s_top = MAX( 0, ci.upper - y );
                    const int s_bottom = MIN( height_bkmk, ci.lower - y );
                    const int height = s_bottom - s_top;

                    if( height > 0 ) {
                        paint_backscreen( m_pixbuf_bkmk, 0, s_top, 1, y - ci.pos_y + s_top,
                                          m_pixbuf_bkmk->get_width(), height );
                    }
                    y += height_bkmk;
                }

                if( CONFIG::get_show_post_mark() ){

                    // 書き込みのマーク表示
                    if( m_article->is_posted( layout->res_number ) ){

                        if( ! m_pixbuf_post ) m_pixbuf_post = ICON::get_pixbuf( ICON::POST );
                        const int height_post = m_pixbuf_post->get_height();

                        if( y == y_org ) y += ( m_font->height - height_post ) / 2;

                        const int s_top = MAX( 0, ci.upper - y );
                        const int s_bottom = MIN( height_post, ci.lower - y );
                        const int height = s_bottom - s_top;

                        if( height > 0 ) {
                            paint_backscreen( m_pixbuf_post, 0, s_top, 1, y - ci.pos_y + s_top,
                                              m_pixbuf_post->get_width(), height );
                        }
                        y += height_post;
                    }

                    // 自分の書き込みに対するレスのマーク表示
                    if( m_article->is_refer_posted( layout->res_number ) ){

                        if( ! m_pixbuf_refer_post ) m_pixbuf_refer_post = ICON::get_pixbuf( ICON::POST_REFER );
                        const int height_refer_post = m_pixbuf_refer_post->get_height();

                        if( y == y_org ) y += ( m_font->height - height_refer_post ) / 2;

                        const int s_top = MAX( 0, ci.upper - y );
                        const int s_bottom = MIN( height_refer_post, ci.lower - y );
                        const int height = s_bottom - s_top;

                        if( height > 0 ) {
                            paint_backscreen( m_pixbuf_refer_post, 0, s_top, 1, y - ci.pos_y + s_top,
                                              m_pixbuf_refer_post->get_width(), height );
                        }
                    }
                }

            }

            break;


            //////////////////////////////////////////

            // 画像ノード
        case DBTREE::NODE_IMG:
        case DBTREE::NODE_SSSP:
            if( draw_one_img_node( layout, ci ) ) relayout = true;
            break;


            //////////////////////////////////////////

            // 水平線ノード
        case DBTREE::NODE_HR:
            if( layout->rect ){
                const int x = layout->rect->x;
                const int y = layout->rect->y - ci.pos_y;
                int color_text = get_colorid_text();
                if( color_text == COLOR_CHAR && layout->div && layout->div->css->color >= 0 ) {
                    color_text = layout->div->css->color;
                }

                cairo_t* const cr = cairo_create( m_backscreen.get() );
                gdk_cairo_set_source_rgba( cr, m_color[ color_text ].gobj() );
                cairo_set_line_width( cr, 1.0 );
                cairo_move_to( cr, x, y );
                cairo_line_to( cr, x + layout->rect->width - 1.0, y );
                cairo_stroke( cr );
                cairo_destroy( cr );
            }
            break;


            //////////////////////////////////////////

            // ノードが増えたらここに追加していくこと

        default:
            break;
    }

    return relayout;
}



//
// div 要素の描画
//
void DrawAreaBase::draw_div( LAYOUT* layout_div, const CLIPINFO& ci )
{
    if( ! ci.lower ) return;

    int bg_color = layout_div->css->bg_color;

    int border_left_color = layout_div->css->border_left_color;
    int border_right_color = layout_div->css->border_right_color;
    int border_top_color = layout_div->css->border_top_color;
    int border_bottom_color = layout_div->css->border_bottom_color;

    if( bg_color < 0 && border_left_color < 0 && border_top_color < 0 && border_bottom_color < 0 ) return;

    int border_left = layout_div->css->border_left_width;
    int border_right = layout_div->css->border_right_width;
    int border_top = layout_div->css->border_top_width;
    int border_bottom = layout_div->css->border_bottom_width;

    int border_style = layout_div->css->border_style;

    int y_div = layout_div->rect->y;
    int height_div = layout_div->rect->height;

    if( y_div < ci.upper ){

        if( border_top && y_div + border_top > ci.upper ) border_top -= ( ci.upper - y_div );
        else border_top = 0;

        height_div -= ( ci.upper - y_div );
        y_div = ci.upper;
    }
    if( y_div + height_div > ci.lower ){

        if( border_bottom && y_div + height_div - border_bottom < ci.lower ) border_bottom -= ( y_div + height_div - ci.lower );
        else border_bottom = 0;

        height_div = ( ci.lower - y_div );
    }

    // 背景
    if( bg_color >= 0 ){
        fill_backscreen( bg_color, layout_div->rect->x, y_div - ci.pos_y,
                         layout_div->rect->width, height_div );
    }

    // left
    if( border_style == CORE::BORDER_SOLID && border_left_color >= 0 && border_left ){
        fill_backscreen( border_left_color, layout_div->rect->x, y_div - ci.pos_y,
                         border_left, height_div );
    }

    // right
    if( border_style == CORE::BORDER_SOLID && border_right_color >= 0 && border_right ){
        fill_backscreen( border_right_color,
                         layout_div->rect->x + layout_div->rect->width - border_right, y_div - ci.pos_y,
                         border_right, height_div );
    }

    // top
    if( border_style == CORE::BORDER_SOLID && border_top_color >= 0 && border_top ){
        fill_backscreen( border_top_color, layout_div->rect->x, y_div - ci.pos_y,
                         layout_div->rect->width, border_top );
    }

    // bottom
    if( border_style == CORE::BORDER_SOLID && border_bottom_color >= 0 && border_bottom ){
        fill_backscreen( border_bottom_color,
                         layout_div->rect->x, y_div + height_div - border_bottom - ci.pos_y,
                         layout_div->rect->width, border_bottom );
    }
}


//
// オートスクロールマーカの描画
//
void DrawAreaBase::draw_marker()
{
    if( m_scrollinfo.mode == SCROLL_NOT ) return;
    if( ! m_scrollinfo.show_marker ) return;

    const int width_view = m_view.get_width();
    const int height_view = m_view.get_height();
    const int x_marker = m_scrollinfo.x - AUTOSCR_CIRCLE/2;
    const int y_marker = m_scrollinfo.y - AUTOSCR_CIRCLE/2;

    m_clip_marker.x = x_marker;
    m_clip_marker.y = y_marker;
    m_clip_marker.width = AUTOSCR_CIRCLE;
    m_clip_marker.height = AUTOSCR_CIRCLE;

    if( m_clip_marker.x < 0 ){

        m_clip_marker.width += m_clip_marker.x;
        m_clip_marker.x = 0;
    }
    if( m_clip_marker.x + m_clip_marker.width > width_view ){

        m_clip_marker.width = width_view - m_clip_marker.x;
    }
    if( m_clip_marker.y < 0 ){

        m_clip_marker.height += m_clip_marker.y;
        m_clip_marker.y = 0;
    }
    if( m_clip_marker.y + m_clip_marker.height > height_view ){

        m_clip_marker.height = height_view - m_clip_marker.y;
    }

    // オートスクロールマーカを描く前に背景のバックアップを取っておきスクロールする前に描き直す
    // exec_draw_screen() 参照
    {
        cairo_t* const cr = cairo_create( m_back_marker.get() );
        cairo_rectangle( cr, 0.0, 0.0, m_clip_marker.width, m_clip_marker.height );
        cairo_clip( cr );
        cairo_set_source_surface( cr, cairo_get_target( m_cr ), -m_clip_marker.x, -m_clip_marker.y );
        cairo_paint( cr );
        cairo_destroy( cr );

        m_ready_back_marker = true;
    }

    constexpr const double r = AUTOSCR_CIRCLE / 2.0;
    cairo_save( m_cr );
    cairo_rectangle( m_cr, m_clip_marker.x, m_clip_marker.y, m_clip_marker.width, m_clip_marker.height );
    cairo_clip( m_cr );
    gdk_cairo_set_source_rgba( m_cr, m_color[ COLOR_MARKER ].gobj() );
    cairo_arc( m_cr, x_marker + r, y_marker + r, r - 1.0, 0.0, 2.0 * M_PI );
    cairo_stroke( m_cr );
    cairo_restore( m_cr );
}


//
// 枠の描画
//
void DrawAreaBase::draw_frame()
{
    if( ! m_draw_frame ) return;

    const int width_win = m_view.get_width();
    const int height_win = m_view.get_height();

    cairo_surface_t* const borrowed_sf = cairo_get_target( m_cr );
    cairo_t* cr = cairo_create( m_back_frame_top.get() );
    cairo_rectangle( cr, 0.0, 0.0, width_win, WIDTH_FRAME );
    cairo_clip( cr );
    cairo_set_source_surface( cr, borrowed_sf, 0.0, 0.0 );
    cairo_paint( cr );
    cairo_destroy( cr );

    cr = cairo_create( m_back_frame_bottom.get() );
    cairo_rectangle( cr, 0.0, 0.0, width_win, WIDTH_FRAME );
    cairo_clip( cr );
    cairo_set_source_surface( cr, borrowed_sf, 0.0, WIDTH_FRAME - height_win );
    cairo_paint( cr );
    cairo_destroy( cr );

    m_ready_back_frame = true;

    cairo_save( m_cr );
    gdk_cairo_set_source_rgba( m_cr, m_color[ COLOR_FRAME ].gobj() );
    cairo_set_line_width( m_cr, 2.0 );
    cairo_rectangle( m_cr, 0.0, 0.0, width_win, height_win );
    cairo_stroke( m_cr );
    cairo_restore( m_cr );
}


//
// バックスクリーンを矩形で塗りつぶす
//
void DrawAreaBase::fill_backscreen( const int colorid, int x, int y, int width, int height )
{
    cairo_t* const cr = cairo_create( m_backscreen.get() );
    gdk_cairo_set_source_rgba( cr, m_color[ colorid ].gobj() );
    cairo_rectangle( cr, x, y, width, height );
    cairo_fill( cr );
    cairo_destroy( cr );
}


//
// Pixbufの内容をバックスクリーンに貼り付ける
//
void DrawAreaBase::paint_backscreen( const Glib::RefPtr< Gdk::Pixbuf >& pixbuf,
                                     int src_x, int src_y, int dest_x, int dest_y, int width, int height )
{
    // Cairoバージョンではsrc_x, src_yを使わない
    // 呼び出しをgdkバージョンと揃えるために引数の数合わせをしている
    static_cast< void >( src_x );
    static_cast< void >( src_y );
    cairo_t* const cr = cairo_create( m_backscreen.get() );
    cairo_rectangle( cr, dest_x, dest_y, width, height );
    cairo_clip( cr );
    gdk_cairo_set_source_pixbuf( cr, pixbuf->gobj(), dest_x, dest_y );
    cairo_paint( cr );
    cairo_destroy( cr );
}


//
// 範囲選択の描画をする必要があるかどうかの判定( draw_one_text_node()で使用 )
//
// 戻り値: 描画が必要かとどうか
// byte_from : 描画開始位置
// byte_to : 描画終了位置
//
bool DrawAreaBase::get_selection_byte( const LAYOUT* layout, const SELECTION& selection, size_t& byte_from,
                                       size_t& byte_to ) const
{
    if( ! layout ) return false;
    if( ! selection.caret_from.layout ) return false;
    if( ! selection.caret_to.layout ) return false;

    const int id_header = layout->id_header;
    const int id = layout->id ;

    int id_header_from = 0;
    int id_from = 0;

    int id_header_to = 0;
    int id_to = 0;

    id_header_from = selection.caret_from.layout->id_header;
    id_from = selection.caret_from.layout->id;

    id_header_to = selection.caret_to.layout->id_header;
    id_to = selection.caret_to.layout->id;

    // 選択開始ノードの場合は selection.caret_from.byte から、それ以外は0バイト目から描画
    byte_from =  selection.caret_from.byte * ( id_header == id_header_from && id == id_from );

    // 選択終了ノードの場合は selection.caret_to.byte から、それ以外は最後まで描画
    byte_to =  selection.caret_to.byte * ( id_header == id_header_to && id == id_to );
    if( byte_to == 0 ) byte_to = strlen( layout->text );

    if( byte_from == byte_to

        //  このノードは範囲選択外なので範囲選択の描画をしない
        || ( id_header < id_header_from )
        || ( id_header > id_header_to )
        || ( id_header == id_header_from && id < id_from )
        || ( id_header == id_header_to && id > id_to )

        // キャレットが先頭にあるので範囲選択の描画をしない
        ||  ( id_header == id_header_to && id == id_to && selection.caret_to.byte == 0 )
        ){

        return false;
    }

    return true;
}

//
// ノードで使うフォントを得る
//
int DrawAreaBase::get_layout_fontid( LAYOUT* layout ) const
{
    if( ! layout->node ) return m_fontid;

    switch( layout->node->fontid ){
    case FONT_AA:
        if( m_aafont_initialized ){
            return layout->node->fontid; // AA用フォント情報
        } else {
            return m_defaultfontid; // デフォルトフォント情報
        }
        break;

    case FONT_MAIL:
        if( m_mailfont_initialized ) {
            return layout->node->fontid; // メール欄などのフォント情報
        }
        else {
            return m_defaultfontid; // デフォルトフォント情報
        }
        break;

    case FONT_EMPTY: // フォントID未決定
    case FONT_DEFAULT:
    default:
        return m_defaultfontid; // デフォルトフォント情報
        break;
    }

    return m_defaultfontid; // デフォルトフォント情報
}

//
// ノードのフォント設定
//
void DrawAreaBase::set_node_font( LAYOUT* layout )
{
    if( ! layout->node ) return;

    // フォント設定
    const int layout_fontid = get_layout_fontid( layout );
    if( m_fontid != layout_fontid ){
        m_fontid = layout_fontid; // 新しいフォントIDをセット
        switch( m_fontid ){
        case FONT_AA:
            m_font = &m_aafont;
            break;
        case FONT_MAIL:
            m_font = &m_mailfont;
            break;
        default:
            m_font = &m_defaultfont;
            break;
        }

#if 0 // _DEBUG
        std::cout << "DrawAreaBase::set_node_font : fontid = " << (int)layout_fontid
            << " res = " << layout->header->res_number
            << " type = " << (int)(layout->node->type) << std::endl;
#endif

        // layoutにフォントをセット
        m_pango_layout->set_font_description( m_font->pfd );
        m_context->set_font_description( m_font->pfd );
    }
}


//
// テキストの含まれているノードひとつを描画する関数
//
// width_view : 描画領域の幅
// pos_y : 描画領域の開始座標
//
void DrawAreaBase::draw_one_text_node( LAYOUT* layout, const CLIPINFO& ci )
{
    // 範囲選択の描画をする必要があるかどうかの判定
    // 二度書きすると重いので、ちょっと式は複雑になるけど同じ所を二度書きしないようにする
    size_t byte_from = 0;
    size_t byte_to = 0;
    const bool draw_selection = ( m_selection.select && get_selection_byte( layout, m_selection, byte_from, byte_to ) );

    int color_text = get_colorid_text();
    if( layout->color_text && *layout->color_text != COLOR_CHAR ) color_text = *layout->color_text;
    if( color_text == COLOR_CHAR && layout->div && layout->div->css->color >= 0 ) color_text = layout->div->css->color;

    int color_back = get_colorid_back();
    if( layout->node && layout->node->color_back != COLOR_NONE ) color_back = layout->node->color_back;
    else if( layout->div && layout->div->css->bg_color >= 0 ) color_back = layout->div->css->bg_color;
    else if( layout->header && layout->header->css->bg_color >= 0 ) color_back = layout->header->css->bg_color;

    // 通常描画
    if( ! draw_selection ) draw_string( layout, ci, color_text, color_back, 0, 0 );

    else { // 範囲選択の前後描画

        // 前
        if( byte_from ) draw_string( layout, ci, color_text, color_back, 0, byte_from );

        // 後
        if( const auto len = std::strlen( layout->text ); byte_to != len ) {
            draw_string( layout, ci, color_text, color_back, byte_to, len );
        }
    }

    // 検索結果のハイライト
    if( m_multi_selection.size() > 0 ){

        for( const SELECTION& selection : m_multi_selection ) {

            size_t byte_from2;
            size_t byte_to2;
            if( get_selection_byte( layout, selection, byte_from2, byte_to2 )){

                draw_string( layout, ci, COLOR_CHAR_HIGHLIGHT, COLOR_BACK_HIGHLIGHT, byte_from2, byte_to2 );
            }
        }
    }

    // 範囲選択部分を描画
    if( draw_selection && byte_from != byte_to ){

        draw_string( layout, ci, COLOR_CHAR_SELECTION, COLOR_BACK_SELECTION, byte_from, byte_to );
    }
}



//
// 画像ノードひとつを描画する関数
//
// width_view : 描画領域の幅
// pos_y : 描画領域の開始座標
//
// 戻り値 : true なら描画後に再レイアウトを実行する
//
bool DrawAreaBase::draw_one_img_node( LAYOUT* layout, const CLIPINFO& ci )
{
#ifdef _DEBUG
    std::cout << "DrawAreaBase::draw_one_img_node link = " << layout->link << std::endl;
#endif

    bool relayout = false;

    if( ! layout->link ) return relayout;

    const RECTANGLE* rect = layout->rect;
    if( ! rect ) return relayout;

    const DBTREE::NODE* node = layout->node;
    if( ! node ) return relayout;

    DBIMG::Img* img = node->linkinfo->img;
    if( ! img ){
        img = node->linkinfo->img = DBIMG::get_img( layout->link );
        if( ! img ) return relayout;
    }

    int color = COLOR_IMG_ERR;
    const int code = img->get_code();

    // 画像が削除された場合、埋め込み画像を削除してノードの座標を再計算
    if( layout->eimg &&
        ( img->is_loading() || code == HTTP_INIT )
        ){

        // 削除対象はアドレスで判定する
        m_eimgs.remove_if( [p = layout->eimg]( auto& e ) { return p == std::addressof( e ); } );
        layout->eimg = nullptr;

        relayout = true;
    }

    if( img->is_loading() || img->is_wait() ) color = COLOR_IMG_LOADING;

    else if( code == HTTP_INIT ){

        if( img->get_abone() ) color = COLOR_IMG_ERR;
        else color = COLOR_IMG_NOCACHE;
    }

    // 画像描画
    else if( code == HTTP_OK ){

        color = COLOR_IMG_CACHED;

        // 埋め込み画像を作成
        if( ! layout->eimg ){

            // EmbeddedImageのリストはDrawAreaBase::clear()でリソースを解放する
            // NOTE: 挿入削除で参照が無効にならないコンテナが条件
            m_eimgs.emplace_back( layout->link );
            layout->eimg = std::addressof( m_eimgs.back() );

            layout->eimg->show();

            // 後のノードの座標を再計算
            relayout = true;
        }

        // 描画
        else{

            Glib::RefPtr< Gdk::Pixbuf > pixbuf = layout->eimg->get_pixbuf();
            if( pixbuf ){

                const int s_top = MAX( 0, ci.upper - ( rect->y + 1 ) );
                const int s_bottom = MIN( pixbuf->get_height(), ci.lower - ( rect->y + 1 ) );
                const int height = s_bottom - s_top;

                // モザイク
                if( img->get_mosaic() ){

                    const int moswidth = img->get_width_mosaic();
                    const int mosheight = img->get_height_mosaic();

                    if( moswidth && mosheight ){
                        if( CONFIG::get_use_grayscale_mosaic() ) {
                            // グレースケール化
                            Glib::RefPtr<Gdk::Pixbuf> gray = MISC::convert_to_grayscale( *pixbuf.get() );
                            pixbuf = std::move( gray );
                        }

                        Glib::RefPtr< Gdk::Pixbuf > pixbuf2;
                        pixbuf2 = pixbuf->scale_simple( moswidth, mosheight, Gdk::INTERP_NEAREST );
                        paint_backscreen( pixbuf2->scale_simple( pixbuf->get_width(), pixbuf->get_height(),
                                                                 Gdk::INTERP_NEAREST ),
                                          0, s_top, rect->x + 1, ( rect->y + 1 ) - ci.pos_y + s_top,
                                          pixbuf->get_width(), height );
                    }
                }

                // 通常
                else{
                    paint_backscreen( pixbuf, 0, s_top, rect->x + 1, ( rect->y + 1 ) - ci.pos_y + s_top,
                                      pixbuf->get_width(), height );
                }



            }
            else color = COLOR_IMG_ERR;
        }
    }

    // 枠の描画
    {
        cairo_t* const cr = cairo_create( m_backscreen.get() );
        gdk_cairo_set_source_rgba( cr, m_color[ color ].gobj() );
        cairo_rectangle( cr, rect->x, rect->y - ci.pos_y, rect->width, rect->height );
        cairo_stroke( cr );
        cairo_destroy( cr );
    }

    // 右上のアイコン
    if( code != HTTP_OK || img->is_loading() ){
        const int x_tmp = rect->x + rect->width / 10 + 1;
        const int y_tmp = rect->y + rect->height / 10 + 1;
        const int width_tmp = rect->width / 4;
        const int height_tmp = rect->width / 4;
        fill_backscreen( color, x_tmp, y_tmp - ci.pos_y, width_tmp, height_tmp );
    }

#ifdef _DEBUG
    std::cout << "code = " << code << " relayout = " << relayout << std::endl;
#endif

    return relayout;
}




/** @brief 文字を描画する関数
 *
 * @details ノードの byte_from バイト目の文字から byte_to バイト目の「ひとつ前」の文字まで描画<br>
 * byte_to が 0 なら最後まで描画<br>
 * <br>
 * たとえば node->text = "abcdefg" で byte_from = 1, byte_to = 3 なら "bc" を描画
 *
 * @param[in] node       文字列を含むレイアウトのデータ
 * @param[in] ci         描画領域のデータ
 * @param[in] color      文字色のID
 * @param[in] color_back 背景色のID
 * @param[in] byte_from  描画をする文字列の始点(バイト目)
 * @param[in] byte_to    描画をする文字列の終点(上記参照)
 */
void DrawAreaBase::draw_string( LAYOUT* node, const CLIPINFO& ci,
                                const int color, const int color_back, const int byte_from, const int byte_to )
{
    assert( node->text != nullptr );
    assert( m_layout_tree );

    if( ! node->lng_text ) return;
    if( byte_from >= node->lng_text ) return;

    RECTANGLE* rect = node->rect;
    while( rect ){

        // 描画領域のチェック
        if( rect->y + rect->height < ci.upper || rect->y > ci.lower ){
            if( rect->end ) break;
            rect = rect->next_rect;
            continue;
        }

        int x = rect->x;
        const int y = rect->y - ci.pos_y;
        int width_line = rect->width;
        int pos_start = rect->pos_start;
        int n_byte = rect->n_byte;
        int n_ustr = rect->n_ustr;

        // 描画する位置が指定されている場合
        if( byte_to
            && ! ( byte_from <= pos_start && pos_start + n_byte <= byte_to ) ){

            if( pos_start > byte_to || pos_start + n_byte < byte_from ) width_line = 0; // 指定範囲外は描画しない

            else{

                // この文字列の全角/半角モードの初期値を決定
                bool wide_mode = set_init_wide_mode( node->text, pos_start, pos_start + n_byte );

                // 左座標計算
                char pre_char = 0;
                int byte_char;
                while( pos_start < byte_from ){
                    x += PANGO_PIXELS( get_width_of_one_char( node->text + pos_start, byte_char, pre_char, wide_mode, get_layout_fontid( node ) ) );
                    pos_start += byte_char;
                }

                // 幅とバイト数計算
                int pos_to = pos_start;
                int byte_to_tmp = byte_to;
                if( rect->pos_start + n_byte < byte_to_tmp ) byte_to_tmp = rect->pos_start + n_byte;
                width_line = 0;
                n_ustr = 0;
                while( pos_to < byte_to_tmp ){
                    width_line += get_width_of_one_char( node->text + pos_to, byte_char, pre_char, wide_mode, get_layout_fontid( node ) );
                    pos_to += byte_char;
                    ++n_ustr;
                }

                width_line = PANGO_PIXELS( width_line );
            }
        }

        if( const int sz = static_cast<int>( m_color.size() ); color >= sz || color_back >= sz ) {
            init_color();
        }

        if( width_line ){

            const int xx = x;

            cairo_t* text_cr = cairo_create( m_backscreen.get() );

            RenderTextArguments args = {
                text_cr,
                rect, // NOTE: rect->width の値は更新されることがある
                node->text + pos_start,
                n_ustr,
                x,
                y,
                color,
                color_back,
                width_line,
                byte_to,
                node->bold,
            };

            (this->*m_render_text)( args );

            // リンクの時は下線を引く
            if( node->link && CONFIG::get_draw_underline() ){
                gdk_cairo_set_source_rgba( text_cr, m_color[ color ].gobj() );
                cairo_set_line_width( text_cr, 1.0 );
                cairo_move_to( text_cr, xx, y + m_font->underline_pos );
                cairo_line_to( text_cr, xx + width_line, y + m_font->underline_pos );
                cairo_stroke( text_cr );
            }

            cairo_destroy( text_cr );
        }

        if( rect->end ) break;
        rect = rect->next_rect;
    }
}


/** @brief 文字を描画する関数 (Pango::GlyphString を使うバージョン)
 *
 * @param[in] args 文字の描写に使うデータ
 */
void DrawAreaBase::render_text_glyphstring( RenderTextArguments& args )
{
    assert( m_context );

    // forループで繰り返し使われるものはローカル変数にコピーする
    cairo_t* text_cr = args.text_cr;
    int x = args.x;
    const int y = args.y;
    const bool bold = args.bold;

    fill_backscreen( args.color_back, x, y, args.width_line, m_font->height );

    gdk_cairo_set_source_rgba( text_cr, m_color[ args.color ].gobj() );

    Pango::AttrList attr;
    const auto text = Glib::ustring( args.text, args.n_ustr );
    const std::vector<Pango::Item> list_item = m_context->itemize( text, attr );

    Glib::RefPtr<Pango::Font> font;
    Pango::GlyphString grl;

    for( const Pango::Item& item : list_item ) {

        font = item.get_analysis().get_font();
        grl = item.shape( item.get_segment( text ) );
        const int width = PANGO_PIXELS( grl.get_width() );

        cairo_move_to( text_cr, x, y + m_font->ascent );
        pango_cairo_show_glyph_string( text_cr, font->gobj(), grl.gobj() );
        if( bold ) {
            cairo_move_to( text_cr, x + 1.0, y + m_font->ascent );
            pango_cairo_show_glyph_string( text_cr, font->gobj(), grl.gobj() );
        }

        x += width;
    }

    // 実際のラインの長さ(x - rect->x)とlayout_one_text_node()で計算した
    // 近似値(rect->width)を一致させる ( 応急処置 )
    if( ! args.byte_to && std::abs( ( x - args.rect->x ) - args.rect->width ) > 2 ) {
        args.rect->width = x - args.rect->x;
    }
}


/** @brief 文字を描画する関数 (Pango::Layout を使うバージョン)
 *
 * @param[in] args 文字の描写に使うデータ
 */
void DrawAreaBase::render_text_pangolayout( RenderTextArguments& args )
{
    const Gdk::RGBA& fg = m_color[ args.color ];
    const Gdk::RGBA& bg = m_color[ args.color_back ];
    using PA = Pango::Attribute;
    auto foreground = PA::create_attr_foreground( fg.get_red_u(), fg.get_green_u(), fg.get_blue_u() );
    auto background = PA::create_attr_background( bg.get_red_u(), bg.get_green_u(), bg.get_blue_u() );

    m_pango_layout->set_text( Glib::ustring( args.text, args.n_ustr ) );

    Pango::AttrList attr;
    attr.insert( foreground );
    attr.insert( background );
    m_pango_layout->set_attributes( attr );
    cairo_move_to( args.text_cr, args.x, args.y );
    pango_cairo_show_layout( args.text_cr, m_pango_layout->gobj() );

    // Pango::Weight属性を使うと文字幅が変わりレイアウトが乱れる
    // そこで文字を重ねて描画することで太字を表示する
    if( args.bold ) {
        Pango::AttrList overlapping;
        overlapping.insert( foreground );
        m_pango_layout->set_attributes( overlapping );
        cairo_move_to( args.text_cr, args.x + 1.0, args.y );
        pango_cairo_show_layout( args.text_cr, m_pango_layout->gobj() );
    }
}


/** @brief 整数 -> 文字列変換してレイアウトに何番目の投稿と発言数をセット
 *
 * @details 最大5桁を想定、5桁より大きい場合は99999にする
 * @param[in] layout 発言数をセットするレイアウト
 * @return セットした文字列の長さ
 * @retval 0 発言数をカウントしてない、または文字列変換に失敗した
 */
int DrawAreaBase::set_num_id( LAYOUT* layout )
{
    int pos = 0;

    const int num_id = (std::min)( layout->header->node->headinfo->num_id_name, 99999 );
    if( num_id >= 2 ){
        const int order = (std::min)( layout->header->node->headinfo->posting_order, 99999 );

        // + 1 はヌル文字の分
        constexpr std::size_t size = std::string::traits_type::length( DBTREE::kPlaceholderForNodeIdNum ) + 1;
        // (何番目の投稿/発言数) の形式で文字列に変換する
        pos = std::snprintf( layout->node->text, size, " (%d/%d)", order, num_id );
        if( pos < 0 ) pos = 0; // conversion error

        layout->lng_text = pos ;
    }

    return pos;
}




//
// スクロール方向指定
//
// 実際にスクロールして描画を実行するのは exec_scroll()
//
bool DrawAreaBase::set_scroll( const int control )
{
    // スクロール系の操作でないときは関数を抜ける
    switch( control ){

        case CONTROL::DownFast:
        case CONTROL::DownMid:
        case CONTROL::Down:
        case CONTROL::NextRes:
        case CONTROL::UpFast:
        case CONTROL::UpMid:
        case CONTROL::Up:
        case CONTROL::PrevRes:
        case CONTROL::Home:
        case CONTROL::End:
        case CONTROL::GotoNew:
        case CONTROL::Back:
            break;

        default:
            return false;
    }

    if( !m_vscrbar ){
        m_scrollinfo.reset();
        return true;
    }

    if( m_scrollinfo.mode == SCROLL_NOT ){

        double dy = 0;

        const int y = get_vscr_val();
        const bool enable_down = ( y < get_vscr_maxval() );
        const bool enable_up = ( y > 0 );

        switch( control ){

            // 下
            case CONTROL::DownFast:
                if( enable_down ) dy  = SCROLLSPEED_FAST;
                break;

            case CONTROL::DownMid:
                if( enable_down ) dy = SCROLLSPEED_MID;
                break;

            case CONTROL::Down:
                if( enable_down ) dy = SCROLLSPEED_SLOW;
                break;

            case CONTROL::NextRes:
                if( enable_down ) goto_next_res();
                break;

                // 上
            case CONTROL::UpFast:
                if( enable_up ) dy = - SCROLLSPEED_FAST;
                break;

            case CONTROL::UpMid:
                if( enable_up )dy = - SCROLLSPEED_MID;
                break;

            case CONTROL::Up:
                if( enable_up )dy = - SCROLLSPEED_SLOW;
                break;

            case CONTROL::PrevRes:
                if( enable_up ) goto_pre_res();
                break;

                // Home, End, New
            case CONTROL::Home:
                if( enable_up ) goto_top();
                break;

            case CONTROL::End:
                if( enable_down ) goto_bottom();
                break;

            case CONTROL::GotoNew:
                goto_new();
                break;

                // ジャンプ元に戻る
            case CONTROL::Back:
                goto_back();
                break;
        }

        // 誤差を考慮した dy != 0
        if( std::abs( dy ) > std::numeric_limits<double>::epsilon() ){

            m_scrollinfo.reset();
            m_scrollinfo.dy = static_cast<int>(dy);

            // キーを押しっぱなしにしてる場合スクロールロックする
            if( m_key_locked ) m_scrollinfo.mode = SCROLL_LOCKED;

            // レスポンスを上げるため押した直後はすぐ描画
            else{

                m_scrollinfo.mode = SCROLL_NORMAL;
                exec_scroll();
            }
        }
    }

    return true;
}




//
// マウスホイールの処理
//
void DrawAreaBase::wheelscroll( GdkEventScroll* event )
{
    const int speed = CONFIG::get_scroll_size();
    const int time_cancel = 15; // msec
    if( !m_vscrbar ) return;

    // あまり速く動かしたならキャンセル
    const int time_tmp = event->time - m_wheel_scroll_time;

    if( ( ! m_wheel_scroll_time || time_tmp >= time_cancel  ) && event->type == GDK_SCROLL ){

        m_wheel_scroll_time = event->time;

        if( m_scrollinfo.mode == SCROLL_NOT || m_scrollinfo.mode == SCROLL_NORMAL ){

            const auto adjust = m_vscrbar->get_adjustment();

            const int current_y = static_cast<int>(adjust->get_value());
            if( event->direction == GDK_SCROLL_UP && current_y == 0 ) return;
            if( event->direction == GDK_SCROLL_DOWN
                    && current_y == static_cast<int>( adjust->get_upper() - adjust->get_page_size() ) ) return;

            m_scrollinfo.reset();
            m_scrollinfo.mode = SCROLL_NORMAL;

            // ホイールのスクロールの場合は必ずスクロール処理を実施する
            m_wait_scroll = Monotonic::duration::zero();

            if( event->direction == GDK_SCROLL_UP ) {
                m_scrollinfo.dy = -static_cast<int>(adjust->get_step_increment()) * speed;
            }
            else if( event->direction == GDK_SCROLL_DOWN ) {
                m_scrollinfo.dy = static_cast<int>(adjust->get_step_increment()) * speed;
            }
            else if( event->direction == GDK_SCROLL_SMOOTH ) {
                constexpr double smooth_scroll_factor = 4.0;
                m_smooth_dy += smooth_scroll_factor * event->delta_y;
                if( std::abs( m_smooth_dy ) > 1.0 ) {
                    const double dy = adjust->get_step_increment() * speed;
                    m_scrollinfo.dy = static_cast<int>( std::copysign( dy, m_smooth_dy ) );
                    m_smooth_dy = 0.0;
                }
            }

            exec_scroll();

            // スクロール終了直後にポインタがリンクの上にある時はマウスをある程度動かすまでモーションイベントをキャンセルする
            // slot_motion_notify_event() を参照
            if( m_layout_current && m_layout_current->link ){
                m_scrollinfo.counter_nomotion = COUNTER_NOMOTION;
                m_scrollinfo.x = m_x_pointer;
                m_scrollinfo.y = m_y_pointer;
            }
        }
    }
}



//
// スクロールやジャンプを実行して再描画
//
// clock_in()からクロック入力される度にスクロールする
//
void DrawAreaBase::exec_scroll()
{
    if( ! m_layout_tree ) return;
    if( ! m_vscrbar ) return;
    if( m_scrollinfo.mode == SCROLL_NOT && ! m_scrollinfo.live ) return;

    // 描画で処理落ちしている時はスクロール処理をキャンセルする
    constexpr const auto zero = Monotonic::duration::zero();
    if( m_wait_scroll != zero ) {

        const auto current = Monotonic::now();
        if( current < m_scroll_time ) {
            m_wait_scroll = zero;
        }
        else {
            const auto passed = current - m_scroll_time;
            m_scroll_time = current;
            m_wait_scroll = std::max( zero, m_wait_scroll - passed );

            if( m_wait_scroll != zero ) {
#ifdef _DEBUG
                std::cout << "cancel scroll passed = " << passed.count()
                          << " wait = " << m_wait_scroll.count() << std::endl;
#endif
                return;
            }
        }
    }

#ifdef _DEBUG
    std::cout << "exec scroll\n";
#endif

    bool redraw_all = false;

    CARET_POSITION caret_pos;
    bool selection = false;

    // 移動後のスクロール位置を計算
    int y = 0;
    auto adjust = m_vscrbar->get_adjustment();
    const int current_y = static_cast<int>(adjust->get_value());

    switch( m_scrollinfo.mode ){

        case SCROLL_TO_NUM: // 指定したレス番号にジャンプ
        {
#ifdef _DEBUG
            std::cout << "DrawAreaBase::exec_scroll : goto " << m_scrollinfo.res << std::endl;
#endif
            const LAYOUT* layout = m_layout_tree->get_header_of_res_const( m_scrollinfo.res );
            if( layout && layout->rect ) y = layout->rect->y;
            m_scrollinfo.reset();
        }
        break;

        // 先頭に移動
        case SCROLL_TO_TOP:
            y = 0;
            m_scrollinfo.reset();
            redraw_all = true;
            break;

        // 最後に移動
        case SCROLL_TO_BOTTOM:
            y = static_cast<int>(adjust->get_upper());
            m_scrollinfo.reset();
            redraw_all = true;
            break;

            // y 座標に移動
        case SCROLL_TO_Y:
            y = m_scrollinfo.y;
            m_scrollinfo.reset();
#ifdef _DEBUG
            std::cout << "DrawAreaBase::exec_scroll : y = " << y << std::endl;
#endif
            break;

        case SCROLL_NORMAL: // 1 回だけスクロール

            y = current_y + m_scrollinfo.dy;
            m_scrollinfo.reset();
            break;

        case SCROLL_LOCKED: // ロックが外れるまでスクロールを続ける

            y = current_y + m_scrollinfo.dy;

            break;

        case SCROLL_AUTO:  // オートスクロールモード
        {
            // 現在のポインタの位置取得
            int x_point, y_point;
            MISC::get_pointer_at_window( m_view.get_window(), x_point, y_point );
            double dy = m_scrollinfo.y - y_point;

            if( dy >= 0 && ! m_scrollinfo.enable_up ) dy = 0;
            else if( dy < 0 && ! m_scrollinfo.enable_down ) dy = 0;
            else{

                // この辺の式は経験的に決定
                if( -AUTOSCR_CIRCLE/4 <= dy && dy <= AUTOSCR_CIRCLE/4 ) dy = 0;
                else {
                    dy = std::copysign( std::fmin( std::expm1( ( std::fabs( dy ) - AUTOSCR_CIRCLE/4 ) /50 ) * 5,
                                                   adjust->get_page_size() * 3 ),
                                        dy );
                }
                if( m_drugging ) dy *= 4;  // 範囲選択中ならスピード上げる
            }

            y = current_y - static_cast<int>(dy);

            // 範囲選択中ならキャレット移動して選択範囲更新
            if( m_drugging ){

                int y_tmp = MIN( MAX( 0, y_point ), m_view.get_height() );
                set_caret( caret_pos, x_point , y + y_tmp );
                selection = true; // スクロールさせてから対応する範囲を描画
            }
        }
        break;

        default:

            // 実況モード
            if( m_scrollinfo.live ){

                // 通常スクロール
                if( m_scrollinfo.live_speed < CONFIG::get_live_threshold() ){

                    const int mode = CONFIG::get_live_mode();
                    if( mode == LIVE_SCRMODE_VARIABLE ) y = static_cast<int>( current_y + m_scrollinfo.live_speed );
                    else if( mode == LIVE_SCRMODE_STEADY ) y = current_y + CONFIG::get_live_speed();
                }

                // 行単位スクロール
                else{

                    const int step_move = 4;
                    const int step_stop = 10;
                    const double step = ( m_scrollinfo.live_speed * ( step_move + step_stop ) ) / step_move;

                    ++m_scrollinfo.live_counter;

                    // スクロール中
                    if( m_scrollinfo.live_counter <= step_move ){
                        y = static_cast<int>( current_y + step );
                    }

                    // 停止中
                    else{
                        if( m_scrollinfo.live_counter == step_move + step_stop ) m_scrollinfo.live_counter = 0;
                        return;
                    }
                }
            }

            break;
    }

    // (upper - page_size) と y はどちらもマイナス値になることがある
    const int y_new = (std::max)( 0, (std::min)( static_cast<int>(adjust->get_upper() - adjust->get_page_size()), y ) );
    if( current_y != y_new ){

        m_cancel_change_adjust = true;
        adjust->set_value( y_new );
        m_cancel_change_adjust = false;

        // キーを押しっぱなしの時に一番上か下に着いたらスクロール停止して全画面再描画
        if( m_scrollinfo.mode == SCROLL_LOCKED && ( y_new <= 0 || y_new >= adjust->get_upper() - adjust->get_page_size() ) ){
            m_scrollinfo.reset();
            redraw_all = true;
        }
    }

    // 選択範囲のセット
    RECTANGLE rect_selection;
    if( selection ){
        if( ! set_selection( caret_pos, &rect_selection ) ) selection = false;
    }

    // 描画
    const int height_redraw = ( redraw_all ? m_view.get_height() : 0 );
    if( draw_screen( 0, height_redraw ) ){

        // 選択範囲描画
        if( selection

            // 既に描画済みならキャンセル
            && ! ( m_rect_backscreen.y <= rect_selection.y
                   && m_rect_backscreen.y + m_rect_backscreen.height >= rect_selection.y + rect_selection.height )
            ){

            // 描画済みの範囲を除く
            if( m_rect_backscreen.y <= rect_selection.y
                && m_rect_backscreen.y + m_rect_backscreen.height >= rect_selection.y ){

                const int dh = ( m_rect_backscreen.y + m_rect_backscreen.height ) - rect_selection.y;
                rect_selection.y += dh;
                rect_selection.height -= dh;
            }
            else if( m_rect_backscreen.y <= rect_selection.y + rect_selection.height
                     && m_rect_backscreen.y + m_rect_backscreen.height >= rect_selection.y + rect_selection.height ){

                const int dh = ( rect_selection.y + rect_selection.height ) - m_rect_backscreen.y;
                rect_selection.height -= dh;
            }

            draw_screen( rect_selection.y, rect_selection.height );
        }

        // カーソル形状の更新
        CARET_POSITION unused_caret_pos;
        m_layout_current = set_caret( unused_caret_pos, m_x_pointer , m_y_pointer + get_vscr_val() );
        change_cursor( get_cursor_type() );
    }
}



//
// スクロールバーの現在値
//
int DrawAreaBase::get_vscr_val() const
{
    if( m_vscrbar ) return static_cast<int>(m_vscrbar->get_adjustment()->get_value());
    return 0;
}


//
// スクロールバーの最大値値
//
int DrawAreaBase::get_vscr_maxval() const
{
    if( m_vscrbar ) return static_cast<int>( m_vscrbar->get_adjustment()->get_upper()
                                             - m_vscrbar->get_adjustment()->get_page_size() );
    return 0;
}



//
// num 番にジャンプ
//
void DrawAreaBase::goto_num( int num )
{
#ifdef _DEBUG
    std::cout << "DrawAreaBase::goto_num num =  " << num << std::endl;
#endif
    if( num <= 0 ) return;
    if( ! m_vscrbar ) return;

    // ジャンプ予約

    // まだ初期化中の場合はジャンプの予約をしておいて、初期化が終わったら時点でもう一回呼び出し
    if( ! m_backscreen ){
        m_goto_num_reserve = num;

#ifdef _DEBUG
        std::cout << "reserve goto_num(1) num = " << m_goto_num_reserve << std::endl;
#endif
        return;
    }

    // 表示範囲を越えていたら再レイアウトしたときにもう一度呼び出し
    else if( num > max_number() ){
        m_goto_num_reserve = num;

#ifdef _DEBUG
        std::cout << "reserve goto_num(2) num = " << m_goto_num_reserve << std::endl;
#endif
        return;
    }

    else m_goto_num_reserve = 0;

    // ロード数を越えている場合
    int number_load = DBTREE::article_number_load( m_url );
    if( number_load < num ) num = number_load;

    // num番が表示されていないときは近くの番号をセット
    while( ! m_layout_tree->get_header_of_res_const( num ) && num++ < number_load );
    while( ! m_layout_tree->get_header_of_res_const( num ) && num-- > 1 );

#ifdef _DEBUG
    std::cout << "exec goto_num num = " << num << std::endl;
#endif

    // スクロール実行
    m_scrollinfo.reset();
    m_scrollinfo.mode = SCROLL_TO_NUM;
    m_scrollinfo.res = num;
    exec_scroll();
}



//
// ジャンプ履歴にスレ番号を登録
//
void DrawAreaBase::set_jump_history( const int num )
{
    m_jump_history.push_back( num );
}


//
// 次のレスに移動
//
void DrawAreaBase::goto_next_res()
{
    if( m_seen_current == max_number() ) goto_bottom();
    else goto_num( m_seen_current + 1 );
}


//
// 前のレスに移動
//
void DrawAreaBase::goto_pre_res()
{
    if( m_seen_current == 0 ) {
        goto_top();
        return;
    }

    // 表示するレスを検索
    const LAYOUT* header;
    int num = m_seen_current;
    int pos_y = get_vscr_val();
    do{ header = m_layout_tree->get_header_of_res_const( --num ); } while( 0 < num && ( ! header || header->rect->y >= pos_y ) );
    goto_num( num );
}


//
// 先頭、新着、最後に移動
//
void DrawAreaBase::goto_top()
{
    if( m_vscrbar ){

        m_jump_history.push_back( get_seen_current() );

        m_scrollinfo.reset();
        m_scrollinfo.mode = SCROLL_TO_TOP;
        exec_scroll();
    }
}

void DrawAreaBase::goto_new()
{
    if( ! m_layout_tree ) return;
    const int separator_new = m_layout_tree->get_separator_new();
    if( separator_new ){

        m_jump_history.push_back( get_seen_current() );

        const int num = separator_new > 1 ? separator_new -1 : 1;

#ifdef _DEBUG
        std::cout << "DrawAreaBase::goto_new num = " << num << std::endl;
#endif
        goto_num( num );
    }
}

void DrawAreaBase::goto_bottom()
{
#ifdef _DEBUG
    std::cout << "DrawAreaBase::goto_bottom\n";
#endif

    if( m_vscrbar ){

        // まだ初期化中の場合はジャンプの予約をしておいて、初期化が終わったら時点でもう一回呼び出し
        if( ! m_backscreen ){

            m_goto_bottom_reserve = true;

#ifdef _DEBUG
            std::cout << "goto_bottom : reserve\n";
#endif
            return;
        }
        m_goto_bottom_reserve = false;

        m_jump_history.push_back( get_seen_current() );

        m_scrollinfo.reset();
        m_scrollinfo.mode = SCROLL_TO_BOTTOM;
        exec_scroll();
    }
}


//
// ジャンプした場所に戻る
//
void DrawAreaBase::goto_back()
{
    if( ! m_jump_history.size() ) return;

    int num = *m_jump_history.rbegin();
    m_jump_history.pop_back();

#ifdef _DEBUG
    std::cout << "DrawAreaBase::goto_back history = " << m_jump_history.size() << " num = " << num << std::endl;
#endif

    goto_num( num );
}


//
// 検索実行
//
// 戻り値: ヒット数
//
int DrawAreaBase::search( const std::list< std::string >& list_query, const bool reverse )
{
    assert( m_layout_tree );

    if( list_query.size() == 0 ) return 0;

    JDLIB::Regex regex;
    std::list<JDLIB::RegexPattern> list_regex;
    const auto make_pattern = []( const std::string& query ) {
        constexpr bool icase = true; // 大文字小文字区別しない
        constexpr bool newline = true; // . に改行をマッチさせない
        constexpr bool usemigemo = true; // migemo使用
        constexpr bool wchar = true;  // 全角半角の区別をしない

        return JDLIB::RegexPattern( query, icase, newline, usemigemo, wchar );
    };

#ifdef _DEBUG
    std::cout << "ArticleViewBase::search size = " << list_query.size() << std::endl;
#endif

    std::transform( list_query.cbegin(), list_query.cend(), std::back_inserter( list_regex ), make_pattern );

    m_multi_selection.clear();

    std::vector< LAYOUT_TABLE > layout_table;
    std::string target;

    // 先頭ノードから順にサーチして m_multi_selection に選択箇所をセットしていく
    LAYOUT* tmpheader = m_layout_tree->top_header();
    while( tmpheader ){

        LAYOUT* tmplayout = tmpheader->next_layout;
        while( tmplayout ){

            if( HAS_TEXT( tmplayout ) ){

                layout_table.clear();
                target.clear();

                // 次のノードもテキストを含んでいたら変換テーブルを作成しながらバッファに連結コピー
                if( tmplayout->next_layout && HAS_TEXT( tmplayout->next_layout ) ){

                    do{
                        layout_table.push_back( LAYOUT_TABLE{ tmplayout, target.size() } );

                        const size_t lng = strlen( tmplayout->text );

                        if( target.size() + lng >= SEARCH_BUFFER_SIZE ){

                            MISC::ERRMSG( "DrawAreaBase::search : buffer overflow." );
                            break;
                        }
                        target.append( tmplayout->text, lng );

                        tmplayout = tmplayout->next_layout;
                    }
                    while( tmplayout && HAS_TEXT( tmplayout ) );

#ifdef _DEBUG
                    std::cout << "use buffer lng = " << target.size() << std::endl << target << std::endl;
#endif
                }
                else {
                    target = tmplayout->text;
                }

                /////////////////////////////////

                size_t offset = 0;
                for(;;){

                    int min_offset = -1;
                    int lng = 0;
                    for( const JDLIB::RegexPattern& pattern : list_regex ) {

                        if( regex.match( pattern, target, offset ) ){

                            if( min_offset == -1 || regex.pos( 0 ) <= min_offset ){
                                min_offset = regex.pos( 0 );
                                lng = regex.length( 0 );
                            }
                        }
                    }

                    if( lng == 0 ) break;

                    offset = min_offset;
                    LAYOUT* layout_from = tmplayout;
                    LAYOUT* layout_to = tmplayout;
                    size_t offset_from = min_offset;
                    size_t offset_to = min_offset + lng;

                    // 変換テーブルを参照して開始位置と終了位置を求める
                    if( layout_table.size() ){

                        std::vector<LAYOUT_TABLE>::const_reverse_iterator it = layout_table.crbegin();
                        it = std::find_if( it, layout_table.crend(),
                                           [offset_to]( const auto& t ) { return t.offset < offset_to; } );
                        if( it != layout_table.crend() ) {
                            layout_to = it->layout;
                            offset_to -= it->offset;
                        }

                        it = std::find_if( it, layout_table.crend(),
                                           [offset_from]( const auto& t ) { return t.offset <= offset_from; } );
                        if( it != layout_table.crend() ) {
                            layout_from = it->layout;
                            offset_from -= it->offset;
                        }
                    }

#ifdef _DEBUG
                    std::cout << "id_from = " << layout_from->id <<  " offset_from = " << offset_from
                              << " -> id_to = " << layout_to->id <<  " offset_to = " << offset_to << std::endl;

                    const std::string text_from = std::string( layout_from->text ).substr( offset_from );
                    const std::string text_to = std::string( layout_to->text ).substr( 0, offset_to );
                    std::cout << text_from << std::endl << text_to << std::endl;
#endif

                    // 選択設定
                    SELECTION selection;
                    selection.select = false;
                    selection.click_pos = SELECTION::ClickPosition::unknown;
                    selection.caret_from.set( layout_from, offset_from );
                    selection.caret_to.set( layout_to, offset_to );
                    m_multi_selection.push_back( selection );
                    offset += lng;
                }
            }

            if( tmplayout ) tmplayout = tmplayout->next_layout;
        }

        tmpheader = tmpheader->next_header;
    }

#ifdef _DEBUG
    std::cout << "m_multi_selection.size = " << m_multi_selection.size() << std::endl;
#endif

    if( m_multi_selection.size() == 0 ) return 0;

    // 初期位置をセット
    // selection.select = true のアイテムが現在選択中
    auto it = std::find_if_not( m_multi_selection.begin(), m_multi_selection.end(),
                                [this]( const SELECTION& s ) { return s.caret_from < m_caret_pos; } );
    if( it != m_multi_selection.end() ) {
        it->select = true;
    }
    else {
        m_multi_selection.back().select = true;
    }

    // search_move でひとつ進めるのでひとつ前に戻しておく
    if( ! reverse ){
        if( it != m_multi_selection.end() ) ( *it ).select = false;
        if( it == m_multi_selection.begin() )  m_multi_selection.back().select = true;
        else {
            --it;
            it->select = true;
        }
    }

    redraw_view_force();
    return m_multi_selection.size();
}



//
// 次の検索結果に移動
//
// 戻り値: ヒット数
//
int DrawAreaBase::search_move( const bool reverse )
{
#ifdef _DEBUG
    std::cout << "ArticleViewBase::search_move " << m_multi_selection.size() << std::endl;
#endif

    if( m_multi_selection.size() == 0 ) return 0;
    if( ! m_vscrbar ) return m_multi_selection.size();

    auto it = std::find_if( m_multi_selection.begin(), m_multi_selection.end(),
                            []( const SELECTION& s ) { return s.select; } );
    if( it != m_multi_selection.end() ){
        it->select = false;

        // NOTE: マウスクリックして検索開始位置を変更したときは、 slot_button_press_event() で
        // SELECTION::select を変更するだけでは自然な移動にならないため移動先を補正します。
        // - 「前検索」ではキャレット位置の前にある検索結果へ移動します。
        // - 「次検索」ではキャレット位置の後にある検索結果へ移動します。

        // クリックして選択を変更したときの位置情報を一時変数に移してリセットします。
        const SELECTION::ClickPosition click_pos = it->click_pos;
        it->click_pos = SELECTION::ClickPosition::unknown;

        // 前に移動
        if( reverse ){
            // マウスクリックした位置がヒットとヒットの間なら、
            // slot_button_press_event() で一つ戻してあるので下のデクリメントを打ち消します。
            // ヒットの上(on_hit)にある場合は、前の検索結果に移動します。
            if( click_pos == SELECTION::ClickPosition::between_hits ) {
                ++it;
            }

            if( it == m_multi_selection.begin() ) it = m_multi_selection.end();
            --it;
        }

        // 次に移動
        else{
            // マウスクリックした位置が先頭のヒットより前でないなら、次の検索結果に移動します。
            if( click_pos != SELECTION::ClickPosition::before_first_hit ) {
                ++it;
            }

            if( it == m_multi_selection.end() ) it = m_multi_selection.begin();
        }

        it->select = true;

        // 移動先を範囲選択状態にする
        if( m_caret_pos != it->caret_to ) {
            m_caret_pos_dragstart = it->caret_from;
            set_selection( it->caret_to );
        }
        else {
            // m_caret_pos == it->caret_to のときに上記の引数で呼び出すと、
            // 前回の呼び出しからキャレット位置が変わないため選択できません。
            // そのため、引数を逆にして呼び出します。
            m_caret_pos_dragstart = it->caret_to;
            set_selection( it->caret_from );
            m_caret_pos_dragstart = it->caret_from;
        }

        const int y = MAX( 0, it->caret_from.layout->rect->y - 10 );

#ifdef _DEBUG
        std::cout << "move to y = " << y << std::endl;
#endif

        auto adjust = m_vscrbar->get_adjustment();
        const int value = static_cast<int>( adjust->get_value() );
        if( value > y || value + static_cast<int>( adjust->get_page_size() ) - m_font->br_size < y ){

            m_cancel_change_adjust = true;
            adjust->set_value( y );
            m_cancel_change_adjust = false;
        }

        redraw_view_force();
    }

    return m_multi_selection.size();
}


/** @brief 引数のキャレット位置に合わせて検索開始位置を更新します。
 *
 * @param[in] caret_pos 検索開始位置にするキャレット位置
 */
void DrawAreaBase::update_search_start_position( const CARET_POSITION& caret_pos )
{
    if( m_multi_selection.empty() ) return;

    // 現在選択中の検索結果の情報をリセットします。
    // XXX: 後続の find_if() とループをまとめたほうが良いか？
    auto it = std::find_if( m_multi_selection.begin(), m_multi_selection.end(),
                            []( const SELECTION& s ) { return s.select; } );
    if( it != m_multi_selection.end() ) {
        it->select = false;
        it->click_pos = SELECTION::ClickPosition::unknown;
    }

    // 新しいキャレット位置より後ろにある検索結果を探します。
    it = std::find_if( m_multi_selection.begin(), m_multi_selection.end(),
                       [&caret_pos]( const SELECTION& s ) { return caret_pos < s.caret_from; } );
    if( it == m_multi_selection.begin() ) {
        // 先頭のヒットより前にキャレットがあります。
        it->click_pos = SELECTION::ClickPosition::before_first_hit;
    }
    else {
        // 先頭のヒット以降にキャレットがあるときは一つ戻して選択します。
        --it;
        if( ! ( it->caret_to < caret_pos ) ) {
            // ヒットの上にキャレットがあります。
            it->click_pos = SELECTION::ClickPosition::on_hit;
        }
        else {
            // ヒットとヒットの間にキャレットがあります。
            it->click_pos = SELECTION::ClickPosition::between_hits;
        }
    }
    it->select = true;
}


//
// ハイライト解除
//
void DrawAreaBase::clear_highlight()
{
    m_multi_selection.clear();
    redraw_view_force();
}


/**
 * @brief キャレットの位置をリセットします。
 */
void DrawAreaBase::reset_caret_position()
{
    m_caret_pos = CARET_POSITION();
    m_caret_pos_pre = CARET_POSITION();
    m_caret_pos_dragstart = CARET_POSITION();
}


//
// 実況開始
//
void DrawAreaBase::live_start()
{
    m_scrollinfo.live = true;
}


//
// 実況停止
//
void DrawAreaBase::live_stop()
{
    m_scrollinfo.reset();
    m_scrollinfo.live = false;
}


//
// 実況時のスクロール速度更新
//
// sec : 更新間隔(秒)
//
void DrawAreaBase::update_live_speed( const int sec )
{
    if( ! m_scrollinfo.live ) return;
    if( sec <= 0 ) return;

    const int min_live_speed = CONFIG::get_live_speed();
    if( ! min_live_speed ) m_scrollinfo.live_speed = 0;
    else{

        double speed = ( get_vscr_maxval() - get_vscr_val() ) / ( sec * 1000/TIMER_TIMEOUT_SMOOTH_SCROLL );
        m_scrollinfo.live_speed = MAX( min_live_speed, speed );
    }

    m_scrollinfo.live_counter = 0;

#ifdef _DEBUG
    std::cout << "DrawAreaBase::update_live_speed sec = " << sec
              << " speed = " << m_scrollinfo.live_speed << std::endl;
#endif
}


//
// ポインタがRECTANGLEの上にあるか判定
//
// int& pos, int& width_line, int& char_width, int& byte_char
// はそれぞれポインタの下の文字の位置(バイト)とその文字までの長さ(ピクセル)、文字の幅(ピクセル)、バイト
//
bool DrawAreaBase::is_pointer_on_rect( const RECTANGLE* rect, LAYOUT* layout, const char* text, const int pos_start, const int pos_to,
                                       const int x, const int y,
                                       int& pos, int& width_line, int& char_width, int& byte_char )
{
    pos = pos_start;
    width_line = 0;
    char_width = 0;
    byte_char = 0;

    if( ! ( rect->y <= y && y <= rect->y + rect->height ) ) return false;

    // この文字列の全角/半角モードの初期値を決定
    bool wide_mode = set_init_wide_mode( text, pos_start, pos_to );
    char pre_char = 0;

    while( pos < pos_to ){

        char_width = get_width_of_one_char( text + pos, byte_char, pre_char, wide_mode, get_layout_fontid( layout ) );

        // マウスポインタの下にノードがある場合
        if( rect->x + PANGO_PIXELS( width_line ) <= x && x <= rect->x + PANGO_PIXELS( width_line + char_width ) ){

            width_line = PANGO_PIXELS( width_line );
            char_width = PANGO_PIXELS( char_width );
            return true;
        }

        // 次の文字へ
        pos += byte_char;
        width_line += char_width;
    }

    return false;
}



//
// 座標(x,y)を与えてキャレットの位置を計算してCARET_POSITIONに値をセット
//、ついでに(x,y)の下にあるレイアウトノードも調べる
//
// CARET_POSITION& caret_pos : キャレットの位置が計算されて入る
//
// 戻り値: 座標(x,y)の下のレイアウトノード。ノード外にある場合はnullptr
//
LAYOUT* DrawAreaBase::set_caret( CARET_POSITION& caret_pos, int x, int y )
{
    if( ! m_layout_tree ) return nullptr;

#ifdef _DEBUG_CARETMOVE
    std::cout << "DrawAreaBase::set_caret x = " << x << " y = " << y << std::endl;
#endif

    // 先頭のレイアウトブロックから順に調べる
    LAYOUT* header = m_layout_tree->top_header();
    if( ! header ) return nullptr;

    // まだレイアウト計算していない
    if( ! header->rect ) return nullptr;
    if( header->next_header && ! header->next_header->rect ) return nullptr;

    while( header ){

        // y が含まれているヘッダブロックだけチェックする
        int height_block = header->next_header ? ( header->next_header->rect->y - header->rect->y ) : BIG_HEIGHT;
        if( ! ( header->rect->y <= y && header->rect->y + height_block >= y ) ){
            header = header->next_header;
            continue;
        }

        // ヘッダブロック内のノードを順に調べていく
        LAYOUT* layout = header->next_layout;
        while( layout ){

            const RECTANGLE* rect = layout->rect;
            if( ! rect ){
                layout = layout->next_layout;
                continue;
            }

            int tmp_x = rect->x;
            int tmp_y = rect->y;
            int width = rect->width;
            int height = rect->height;
            int pos_start = rect->pos_start;
            int n_byte = rect->n_byte;

            // テキストノードでは無い
            if( ! layout->text ){
                layout = layout->next_layout;
                continue;
            }

            // フォント設定
            set_node_font( layout );

            //////////////////////////////////////////////
            //
            // 現在のテキストノードの中、又は左か右にあるか調べる
            //
            for(;;){

                int pos;
                int width_line;
                int char_width;
                int byte_char;

                // テキストノードの中にある場合
                if( is_pointer_on_rect( rect, layout, layout->text, pos_start, pos_start + n_byte, x, y,
                                        pos, width_line, char_width, byte_char ) ){

#ifdef _DEBUG_CARETMOVE
                    std::cout << "found: on node\n";
                    std::cout << "header id = " << header->id_header << std::endl;
                    std::cout << "node id = " << layout->id << std::endl;
                    std::cout << "pos = " << pos << std::endl;
                    std::cout << "tmp_x = " << tmp_x << std::endl;
                    std::cout << "tmp_y = " << tmp_y << std::endl;
                    std::cout << layout->text << std::endl;
#endif
                    // キャレットをセットして終了
                    caret_pos.set( layout, pos, x, tmp_x + width_line, tmp_y, char_width, byte_char );
                    return layout;
                }

                else if( tmp_y <= y && y <= tmp_y + height ){

                    // 左のマージンの上にポインタがある場合
                    if( x < tmp_x ){

#ifdef _DEBUG_CARETMOVE
                        std::cout << "found: left\n";
                        std::cout << "header id = " << header->id_header << std::endl;
                        std::cout << "node id = " << layout->id << std::endl;
                        std::cout << "pos = " << pos_start << std::endl;
#endif
                        // 左端にキャレットをセットして終了
                        caret_pos.set( layout, pos_start, x, tmp_x, tmp_y );

                        return nullptr;
                    }

                    // 右のマージンの上にポインタがある場合
                    else if( layout->next_layout == nullptr || layout->next_layout->type == DBTREE::NODE_BR ){

#ifdef _DEBUG_CARETMOVE
                        std::cout << "found: right\n";
                        std::cout << "header id = " << header->id_header << std::endl;
                        std::cout << "node id = " << layout->id << std::endl;
                        std::cout << "pos = " << pos_start + n_byte << std::endl;
#endif

                        // 右端にキャレットをセットして終わり
                        caret_pos.set( layout, pos_start + n_byte, x, tmp_x + width, tmp_y );
                        return nullptr;
                    }
                }

                // 折り返し無し
                if( rect->end ) break;

                rect = rect->next_rect;
                if( ! rect ) break;

                tmp_x = rect->x;
                tmp_y = rect->y;
                width = rect->width;
                height = rect->height;
                pos_start = rect->pos_start;
                n_byte = rect->n_byte;
            }


            //////////////////////////////////////////////
            //
            // 現在のノードと次のノード、又はヘッダブロックの間にポインタがあるか調べる
            //

            // 次のノードを取得する
            LAYOUT* layout_next = layout->next_layout;
            while( layout_next && ! ( layout_next->rect && layout_next->text ) ){

                // 次のノードが画像ノード場合
                // 現在のノードの右端にキャレットをセットして画像ノードを返す
                if( ( layout_next->type == DBTREE::NODE_IMG || layout_next->type == DBTREE::NODE_SSSP )
                    && layout_next->rect
                    && ( layout_next->rect->x <= x && x <= layout_next->rect->x + layout_next->rect->width )
                    && ( layout_next->rect->y <= y && y <= layout_next->rect->y + layout_next->rect->height ) ){

                    caret_pos.set( layout, pos_start + n_byte, x, tmp_x + width, tmp_y );
                    return layout_next;
                }

                layout_next = layout_next->next_layout;
            }

            // 残りのノードのうち、最小のy座標を取得
            int next_y = y + BIG_HEIGHT; // 次のノード or ブロックのy座標
            bool next_found = false;
            while( layout_next ){
                if( layout_next->rect && next_y > layout_next->rect->y ){
                    next_y = layout_next->rect->y;
                    if( next_y <= y ){
                        next_found = true;
                        break; // 小さいy座標のノードが見つかったので、次のノードの処理に進む
                    }
                }
                layout_next = layout_next->next_layout;
            }

            if( !next_found ) {

#ifdef _DEBUG_CARETMOVE
                std::cout << "found: between\n";
                std::cout << "header id = " << header->id_header << std::endl;
                std::cout << "node id = " << layout->id << std::endl;
#endif
                // 現在のノードの右端にキャレットをセット
                caret_pos.set( layout, pos_start + n_byte, x, tmp_x + width, tmp_y );

                return nullptr;
            }

            // 次のノードへ
            layout = layout->next_layout;
        }

        // 次のブロックへ
        header = header->next_header;
    }

    return nullptr;
}



//
// (x,y)地点をダブルクリック時の範囲選択のためのキャレット位置を取得
//
// caret_left : 左側のキャレット位置
// caret_left : 右側のキャレット位置
//
// 戻り値 : 成功すると true
//

// 区切り文字
static bool is_separate_char( const char32_t unich )
{
    if( unich == ' '

        || unich == '.'
        || unich == ','

        || unich == '('
        || unich == ')'

        || unich == '='

        // 全角空白
        || unich == 0x3000

        // 。、
        || unich == 0x3001
        || unich == 0x3002

        // ．，
        || unich == 0xFF0C
        || unich == 0xFF0E

        // 全角()
        || unich == 0xFF08
        || unich == 0xFF09

        // 全角 =
        || unich == 0xFF1D

        // 「」
        || unich == 0x300C
        || unich == 0x300D
        || unich == 0x300E
        || unich == 0x300F

        ) return true;

    return false;
}


//
// ダブルクリック時にキャレット位置を決める
//
bool DrawAreaBase::set_carets_dclick( CARET_POSITION& caret_left, CARET_POSITION& caret_right,
                                      const int x, const int y, const bool triple )
{
    if( ! m_layout_tree ) return false;

#ifdef _DEBUG
    std::cout << "DrawAreaBase::set_carets_dclick\n";
#endif

    // 先頭のヘッダブロックから順に調べる
    LAYOUT* header = m_layout_tree->top_header();
    while( header ){

        // y が含まれているブロックだけチェックする
        if( header->rect->y <= y && y <= header->rect->y + header->rect->height ){

            LAYOUT* layout = header->next_layout;
            LAYOUT* layout_before = layout;
            while( layout ){

                RECTANGLE* rect = layout->rect;

                if( ! layout->text || ! rect ){
                    layout = layout->next_layout;
                    if( layout && layout->text ) layout_before = layout;
                    else layout_before = nullptr;
                    continue;
                }

                // フォント設定
                set_node_font( layout );

                // ポインタの下にあるノードを探す
                int pos = 0;
                while( rect ){

                    int width_line;
                    int char_width;
                    int byte_char;

                    if( is_pointer_on_rect( rect, layout, layout->text, rect->pos_start, rect->pos_start + rect->n_byte,
                                            x, y,
                                            pos, width_line, char_width, byte_char ) ) break;

                    rect = rect->next_rect;
                }

                if( ! rect ){
                    layout = layout->next_layout;
                    continue;
                }

                // トリプルクリック
                if( triple ){

                    LAYOUT* layout_after = layout;
                    while( layout_after->next_layout && layout_after->next_layout->text ) layout_after = layout_after->next_layout;

                    if( layout_before ) caret_left.set( layout_before, 0 );
                    else caret_left.set( layout, 0 );

                    caret_right.set( layout_after, layout_after->lng_text );

                    return true;
                }

                int byte_char_pointer;
                const char32_t uch_pointer = MISC::utf8toutf32( layout->text + pos, byte_char_pointer );
                const MISC::UnicodeBlock block_pointer = MISC::get_unicodeblock( uch_pointer );
#ifdef _DEBUG
                std::cout << "utf32 = " << MISC::utf32tostr( uch_pointer )
                          << " type = " << static_cast<int>( block_pointer ) << " pos = " << pos << std::endl;
#endif

                // 区切り文字をダブルクリックした
                if( is_separate_char( uch_pointer ) ){
                    caret_left.set( layout, pos );
                    caret_right.set( layout, pos + byte_char_pointer );
                    return true;
                }

                // 左位置を求める
                int pos_left = 0;
                int pos_tmp = 0;
                while( pos_tmp < pos ){

                    int byte_char;
                    const char32_t uch = MISC::utf8toutf32( layout->text + pos_tmp, byte_char );
                    const MISC::UnicodeBlock block = MISC::get_unicodeblock( uch );

                    int byte_char_next;
                    const char32_t uch_next = MISC::utf8toutf32( layout->text + pos_tmp + byte_char, byte_char_next );
                    const MISC::UnicodeBlock block_next = MISC::get_unicodeblock( uch_next );

                    // 区切り文字が来たら左位置を移動する
                    if( uch_next == '\0' || is_separate_char( uch )
                        // 文字種が変わった
                        || ( block != block_pointer && block_next == block_pointer )

                        ) pos_left = pos_tmp + byte_char;

                    pos_tmp += ( byte_char ? byte_char : 1 );
                }

                // 右位置を求める
                int pos_right = pos;
                while( pos_right < layout->lng_text ){

                    int byte_char;
                    const char32_t uch = MISC::utf8toutf32( layout->text + pos_right, byte_char );
                    const MISC::UnicodeBlock block = MISC::get_unicodeblock( uch );

                    int byte_char_next;
                    const char32_t uch_next = MISC::utf8toutf32( layout->text + pos_right + byte_char, byte_char_next );
                    const MISC::UnicodeBlock block_next = MISC::get_unicodeblock( uch_next );

                    // 区切り文字が来たらbreak
                    if( is_separate_char( uch ) ) break;

                    pos_right += ( byte_char ? byte_char : 1 );

                    // 文字種が変わった
                    if( uch_next == '\0'
                        || ( block == block_pointer && block_next != block_pointer )
                        ) break;
                }

#ifdef _DEBUG
                std::cout << "pos_left = " << pos_left << " pos_right = " << pos_right << std::endl;
#endif

                // キャレット設定
                caret_left.set( layout, pos_left );
                caret_right.set( layout, pos_right );

                return true;
            }
        }

        // 次のブロックへ
        header = header->next_header;
    }

    return false;
}



//
// 範囲選択の範囲を計算してm_selectionにセット & 範囲選択箇所の再描画
//
// caret_left から caret_right まで範囲選択状態にする
//

bool DrawAreaBase::set_selection( const CARET_POSITION& caret_left, const CARET_POSITION& caret_right )
{
    m_caret_pos_pre = caret_left;
    m_caret_pos = caret_left;
    m_caret_pos_dragstart = caret_left;
    return set_selection( caret_right, nullptr );
}


//
// 範囲選択の範囲を計算してm_selectionにセット
//
// caret_pos : 移動後のキャレット位置、m_caret_pos_pre から caret_pos まで範囲選択状態にする
//
bool DrawAreaBase::set_selection( const CARET_POSITION& caret_pos )
{
    return set_selection( caret_pos, nullptr );
}


// rect に再描画範囲を計算して入れる( nullptr なら入らない )
// その後 draw_screen( rect.y, rect.height ) で選択範囲を描画する
bool DrawAreaBase::set_selection( const CARET_POSITION& caret_pos, RECTANGLE* rect )
{
    if( ! caret_pos.layout ) return false;
    if( ! m_caret_pos_dragstart.layout ) return false;

    // 前回の呼び出しからキャレット位置が変わってない
    if( m_caret_pos == caret_pos ) return false;

    m_caret_pos_pre = m_caret_pos;
    m_caret_pos = caret_pos;

#ifdef _DEBUG
    std::cout << "DrawAreaBase::set_selection()\n";
    std::cout << "start   header = " <<  m_caret_pos_dragstart.layout->id_header << " node = " << m_caret_pos_dragstart.layout->id;
    std::cout << " byte = " << m_caret_pos_dragstart.byte << std::endl;
    std::cout << "current header = " <<  m_caret_pos.layout->id_header << " node = " << m_caret_pos.layout->id;
    std::cout << " byte = " << m_caret_pos.byte  << std::endl;
#endif

    // ドラッグ開始位置と現在のキャレット位置が同じなら選択解除
    if( m_caret_pos_dragstart == m_caret_pos ) m_selection.select = false;

    // 範囲計算
    else{
        m_selection.select = true;

        if( m_caret_pos_dragstart > m_caret_pos ){
            m_selection.caret_from = m_caret_pos;
            m_selection.caret_to = m_caret_pos_dragstart;
        }
        else{
            m_selection.caret_from = m_caret_pos_dragstart;
            m_selection.caret_to = m_caret_pos;
        }
    }

    if( ! rect ) return true;

    // 再描画範囲計算

    const int pos_y = get_vscr_val();
    const int height_view = m_view.get_height();
    int y_redraw = 0;
    int height_redraw = height_view;

    LAYOUT* layout = m_caret_pos_pre.layout;
    LAYOUT* layout_to = m_caret_pos.layout;

    if( ! layout ) layout = m_caret_pos_dragstart.layout;

    // layout_toの方が前だったらポインタを入れ換え
    if( layout_to->id_header < layout->id_header
        || ( layout_to->id_header == layout->id_header && layout_to->id < layout->id ) ){
        LAYOUT* layout_tmp = layout_to;
        layout_to = layout;
        layout = layout_tmp;
    }

    const RECTANGLE* rect_from = layout->rect;
    const RECTANGLE* rect_to = layout_to->rect;
    if( ! rect_from || ! rect_to ) return false;
    while( rect_to->next_rect ) rect_to = rect_to->next_rect;

    // 範囲外
    if( rect_from->y > pos_y + height_view ) return false;
    if( rect_to->y + rect_to->height < pos_y ) return false;

    if( rect_from->y > pos_y ){
        y_redraw = rect_from->y - pos_y;
        height_redraw = height_view - y_redraw;
    }
    if( rect_to->y + rect_to->height < pos_y + height_view ){
        height_redraw -= ( pos_y + height_view ) - ( rect_to->y + rect_to->height );
    }

#ifdef _DEBUG
    std::cout << "redraw layout from : " << layout->id_header << ":" << layout->id
              << " to " << layout_to->id_header <<  ":" << layout_to->id
              << " y_redraw = " << y_redraw << " height_redraw = " << height_redraw
              << std::endl;
#endif

    rect->y = y_redraw;
    rect->height = height_redraw;

    return true;
}



//
// 範囲選択の文字列取得
//
// set_selection()の中で毎回やると重いので、ボタンのリリース時に一回だけ呼び出すこと
//
bool DrawAreaBase::set_selection_str()
{
    assert( m_layout_tree );

    if( ! m_selection.str.empty() ) m_selection.str_pre = m_selection.str;
    m_selection.str.clear();
    m_selection.imgurls.clear();

    if( !m_selection.select ) return false;

#ifdef _DEBUG
    std::cout << "DrawAreaBase::set_selection_str\n";
    std::cout << "from header = " << m_selection.caret_from.layout->id_header << " node = " <<  m_selection.caret_from.layout->id;
    std::cout << " byte = " <<  m_selection.caret_from.byte << std::endl;
    std::cout << "to   header = " << m_selection.caret_to.layout->id_header << " node = " << m_selection.caret_to.layout->id;
    std::cout << " byte = " << m_selection.caret_to.byte  << std::endl;
#endif

    std::vector< URLINFO > urls;

    bool start_copy = false;

    // 面倒臭いんで先頭のレイアウトブロックから順に調べていく
    LAYOUT* tmpheader = m_layout_tree->top_header();
    while( tmpheader ){

        // ブロック内のノードを順に調べていく
        LAYOUT* tmplayout = tmpheader->next_layout;
        while( tmplayout ){

            int copy_from = 0, copy_to = 0;

            // 開始ノード
            if( tmplayout == m_selection.caret_from.layout ){
                start_copy = true;
                copy_from = m_selection.caret_from.byte;
                copy_to = strlen( tmplayout->text );
            }

            // 終了ノード
            if( tmplayout == m_selection.caret_to.layout ) copy_to = m_selection.caret_to.byte;


            // 文字列コピー
            if( start_copy ){

                if( tmplayout->type == DBTREE::NODE_BR ) m_selection.str += "\n";
                else if( tmplayout->type == DBTREE::NODE_DIV ) m_selection.str += "\n";
                else if( tmplayout->type == DBTREE::NODE_HTAB ) m_selection.str += "\t";

                else if( tmplayout->text ){
                    if( copy_from || copy_to ) m_selection.str += std::string( tmplayout->text ).substr( copy_from, copy_to - copy_from );
                    else m_selection.str += tmplayout->text;

                    if( tmplayout->type == DBTREE::NODE_LINK && tmplayout->link ){

                        URLINFO urlinfo;
                        urlinfo.url = std::string( tmplayout->link );
                        urlinfo.res_number = tmplayout->res_number;
                        urls.push_back( urlinfo );
                    }
                }
            }

            // 終了
            if( tmplayout == m_selection.caret_to.layout ){

                // 画像のURLだけ抽出する
                if( urls.size() ){

                    for( const URLINFO& info : urls ) {

                        if( DBIMG::get_type_ext( info.url ) != DBIMG::T_UNKNOWN ) {

                            const std::string& url = info.url;
                            const bool found = std::any_of( m_selection.imgurls.cbegin(), m_selection.imgurls.cend(),
                                                            [&url]( const auto& i ) { return url == i.url; } );
                            if( ! found ) {
                                m_selection.imgurls.push_back( info );
                            }
                        }
                    }
                }

                return true;
            }

            tmplayout = tmplayout->next_layout;
        }

        tmpheader = tmpheader->next_header;

        if( start_copy ){
            m_selection.str += "\n";
            if( tmpheader ) m_selection.str += "\n";
        }
    }

    return false;
}



//
// caret_pos が範囲選択の上にあるか
//
//
bool DrawAreaBase::is_caret_on_selection( const CARET_POSITION& caret_pos ) const
{
    const LAYOUT* layout = caret_pos.layout;

    if( !layout || ! m_selection.select || m_selection.str.empty() ) return false;
    if( layout->id_header != m_selection.caret_from.layout->id_header ) return false;

    const int from_id = m_selection.caret_from.layout->id;
    const int from_byte = m_selection.caret_from.byte;

    const int to_id = m_selection.caret_to.layout->id;
    const int to_byte = m_selection.caret_to.byte;

    if( to_id < layout->id ) return false;
    if( to_id == layout->id && to_byte < caret_pos.byte ) return false;
    if( from_id > layout->id ) return false;
    if( from_id == layout->id && from_byte > caret_pos.byte ) return false;

    return true;
}


//
// 範囲選択範囲にcaret_posが含まれていて、かつ条件(IDや数字など)を満たしていたらURLとして範囲選択文字を返す
//
std::string DrawAreaBase::get_selection_as_url( const CARET_POSITION& caret_pos ) const
{
    std::string url;

    if( is_caret_on_selection( caret_pos ) ){

        size_t n,dig;
        std::string select_str = MISC::utf8_trim( m_selection.str );
        select_str = MISC::remove_str( select_str, "\n" );
        int num = MISC::str_to_uint( select_str.c_str(), dig, n );

        // 数字
        if( dig && num ){

            url = PROTO_ANCHORE + std::to_string( num );

            for(;;){

                select_str = select_str.substr( n );
                if( select_str.empty() ) break;

                std::string tmpstr, tmpstr2;
                if( select_str.rfind( '-', 0 ) == 0 ) tmpstr = tmpstr2 = "-";
                else if ( select_str.rfind( '=', 0 ) == 0 ) tmpstr = tmpstr2 = "=";
                else if ( select_str.rfind( ',', 0 ) == 0 ) tmpstr = tmpstr2 = ",";
                else if( select_str.rfind( "－", 0 ) == 0 ){ tmpstr = "－"; tmpstr2 = "-"; }
                else if( select_str.rfind( "−", 0 ) == 0 ){ tmpstr = "−"; tmpstr2 = "-"; }
                else if ( select_str.rfind( "＝", 0 ) == 0 ){ tmpstr = "＝"; tmpstr2 = "="; }
                else if ( select_str.rfind( "，", 0 ) == 0 ){ tmpstr = "，"; tmpstr2 = ","; }

                select_str = select_str.substr( tmpstr.length() );

                num = MISC::str_to_uint( select_str.c_str(), dig, n );
                if( dig && num ) url += tmpstr2 + std::to_string( num );
                else break;
            }
        }

        // ID
        else if( select_str.rfind( "ID:", 0 ) == 0 ) url = select_str;
    }

#ifdef _DEBUG
    if( !url.empty() ) std::cout << "DrawAreaBase::get_selection_as_url : " << url << std::endl;
#endif

    return url;
}


// 全選択
void DrawAreaBase::select_all()
{
#ifdef _DEBUG
    std::cout << "DrawAreaBase::select_all\n";
#endif

    CARET_POSITION caret_left, caret_right;
    LAYOUT* layout = nullptr;
    LAYOUT* layout_back = nullptr;

    // 先頭
    LAYOUT* header = m_layout_tree->top_header();
    while( header ){

        layout = header->next_layout;
        while( layout && ! layout->text ){
            layout = layout->next_layout;
        }
        if( layout ) break;

        header = header->next_header;
    }
    if( ! layout ) return;

#ifdef _DEBUG
    std::cout << "id = " << layout->id_header << "-" << layout->id << " text = " << layout->text << std::endl;
#endif

    caret_left.set( layout, 0 );

    // 最後
    while( header ){

        layout = header->next_layout;
        while( layout ){
            if( layout->text ) layout_back = layout;
            layout = layout->next_layout;
        }

        header = header->next_header;
    }
    if( ! layout_back ) return;

#ifdef _DEBUG
    std::cout << "-> id = " << layout_back->id_header << "-" << layout_back->id << " text = " << layout_back->text << std::endl;
#endif

    caret_right.set( layout_back, layout_back->lng_text );

    set_selection( caret_left, caret_right );
    set_selection_str();
    redraw_view_force();
}



//
// VScrollbar が動いた
//
void DrawAreaBase::slot_change_adjust()
{
    if( m_cancel_change_adjust ) return;
    if( m_scrollinfo.mode != SCROLL_NOT ) return; // スクロール中

#ifdef _DEBUG
    std::cout << "slot_change_adjust\n";
#endif

    m_scrollinfo.reset();
    m_scrollinfo.mode = SCROLL_NORMAL;
    exec_scroll();
}



//
// drawarea がリサイズした
//
bool DrawAreaBase::slot_configure_event( GdkEventConfigure* event )
{
    // 表示されていないview(is_drawable() != true ) は表示された段階で
    // redraw_view() したときに configure_impl() を呼び出す
    m_configure_reserve = true;
    configure_impl();

    return true;
}

//
// drawarea がリサイズ実行
//
void DrawAreaBase::configure_impl()
{
    if( ! m_configure_reserve ) return;
    if( !m_view.get_is_drawable() ) return;

    m_configure_reserve = false;

    const int width = m_view.get_width();
    const int height = m_view.get_height();

    if( height < LAYOUT_MIN_HEIGHT ) return;

    // サイズがほぼ変わっていないときは再レイアウトしない
    // 値が微妙に変化していることがあり == による比較ではスクロールしてしまうバグがあった
    if( std::abs( m_configure_width - width ) < 2 && std::abs( m_configure_height - height ) < 2 ) return;

    // リサイズする前のレス番号を保存しておいて
    // redrawした後にジャンプ
    const int seen_current = m_seen_current;

    // リサイズ前と横幅が同じ場合はスクロールバーの位置を変えない
    const int pos_y = ( m_configure_width == width ? get_vscr_val() : 0 );

#ifdef _DEBUG
    std::cout << "DrawAreaBase::configure_impl : url = " << m_url << std::endl
              << "seen_current = " << seen_current
              << " pos_y = " << pos_y
              << " width = " << width << " heigth = " << height
              << " pre_width = " << m_configure_width << " pre_height = " << m_configure_height << std::endl;
#endif

    m_configure_width = width;
    m_configure_height = height;

    if( exec_layout() ){

        // スクロール実行
        if( pos_y ){
            m_scrollinfo.reset();
            m_scrollinfo.mode = SCROLL_TO_Y;
            m_scrollinfo.y = pos_y;
            exec_scroll();
        }
        else if( seen_current ) goto_num( seen_current );
        else redraw_view_force();
    }
}



//
// drawarea の再描画イベント
//
bool DrawAreaBase::slot_draw( const Cairo::RefPtr< Cairo::Context >& cr )
{
    double x1, y1, x2, y2;
    cr->get_clip_extents( x1, y1, x2, y2 );
    const double width = x2 - x1;
    const double height = y2 - y1;

    // タブ操作中は再描画しない
    if( SESSION::is_tab_operating( URL_ARTICLEADMIN ) ) {
        return true;
    }
#ifdef _DEBUG
    std::cout << "DrawAreaBase::slot_draw"
              << " y = " << y1 << " height = " << height << " draw_screen = " << m_drawinfo.draw
              << " url = " << m_url
              << std::endl;
#endif
    // cairommが管理するオブジェクトを借用する
    m_cr = cr->cobj();

    // draw_screen からの呼び出し
    if( m_drawinfo.draw ) {
#ifdef _DEBUG
        std::cout << "draw\n";
#endif
        m_drawinfo.draw = false;
        exec_draw_screen( m_drawinfo.y, m_drawinfo.height );
    }
    // バックスクリーンに描画済みならコピー
    else if( y1 >= m_rect_backscreen.y
             && y2 <= m_rect_backscreen.y + m_rect_backscreen.height ) {
#ifdef _DEBUG
        std::cout << "copy from backscreen\n";
#endif
        cairo_save( m_cr );
        cairo_rectangle( m_cr, x1, y1, width, height );
        cairo_clip( m_cr );
        cairo_set_source_surface( m_cr, m_backscreen.get(), x1, y1 );
        cairo_paint( m_cr );
        cairo_restore( m_cr );

        // オートスクロールマーカと枠の描画
        draw_marker();
        draw_frame();
    }
    // レイアウトがセットされていない or まだリサイズしていない
    // ( m_backscreen == nullptr )なら画面消去
    else if( !m_layout_tree->top_header() || !m_backscreen ) {
#ifdef _DEBUG
        std::cout << "clear window\n";
#endif
        cairo_save( m_cr );
        gdk_cairo_set_source_rgba( m_cr, m_color[ get_colorid_back() ].gobj() );
        cairo_fill( m_cr );
        cairo_restore( m_cr );

        // シグナルハンドラの引数から借りただけなのでnullptrでリセットする
        m_cr = nullptr;
        return false;
    }
    // 必要な所だけ再描画
    else {
#ifdef _DEBUG
        std::cout << "expose\n";
#endif
        exec_draw_screen( static_cast< int >( y1 ), static_cast< int >( height ) );
    }
    // シグナルハンドラの引数から借りただけなのでnullptrでリセットする
    m_cr = nullptr;
    return true;
}



//
// drawarea でマウスホイールが動いた
//
bool DrawAreaBase::slot_scroll_event( GdkEventScroll* event )
{
    m_sig_scroll_event.emit( event );

    return true;
}


//
// マウスが領域外に出た
//
bool DrawAreaBase::slot_leave_notify_event( GdkEventCrossing* event )
{
#ifdef _DEBUG
    std::cout << "DrawAreaBase::slot_leave_notify_event\n";
#endif

    // 右ドラッグ中はシグナルを発行しない
    if( ! m_r_drugging ) m_sig_leave_notify.emit( event );

    return false;
}



//
// realize
//
// GCを作成してレイアウト
//
void DrawAreaBase::slot_realize()
{
#ifdef _DEBUG
    std::cout << "DrawAreaBase::slot_realize()" << std::endl;
#endif

    m_window = m_view.get_window();
    assert( m_window );

    // 色初期化
    init_color();

    m_back_marker.reset( gdk_window_create_similar_surface(
        m_window->gobj(), CAIRO_CONTENT_COLOR, AUTOSCR_CIRCLE, AUTOSCR_CIRCLE ) );
    assert( m_back_marker );

    exec_layout();
    m_view.grab_focus();
}



//
// マウスボタンを押した
//
bool DrawAreaBase::slot_button_press_event( GdkEventButton* event )
{
    m_clicked = true;

    std::string url;
    int res_num = 0;
    bool redraw_force = false;

    if( m_layout_current && m_layout_current->link ) url = m_layout_current->link;
    if( m_layout_current ) res_num = m_layout_current->res_number;

    const int pos = get_vscr_val();
    const int x = static_cast<int>(event->x);
    const int y = static_cast<int>(event->y) + pos;

    CARET_POSITION caret_pos;
    set_caret( caret_pos, x, y );

    m_view.grab_focus();

    // オートスクロール中ならオートスクロール解除
    if( m_scrollinfo.mode == SCROLL_AUTO ){

        m_scrollinfo.reset();

        // リンクのクリックを認識させないためオートスクロール解除直後はリンククリックの処理をしない
        // 直後に slot_button_release_event() が呼び出されて m_scrollinfo がリセットされる
        m_scrollinfo.autoscroll_finished = true;

        change_cursor( cursor_names::kDefault );
    }
    else {

        // 範囲選択解除、及びドラッグ開始
        if( m_control.button_alloted( event, CONTROL::ClickButton ) ){

            // 引数のキャレット位置に合わせて検索開始位置を更新します。
            update_search_start_position( caret_pos );

            m_drugging = true;
            m_selection.select = false;
            if( ! m_selection.str.empty() ) m_selection.str_pre = m_selection.str;
            m_selection.str.clear();
            m_selection.imgurls.clear();
            m_caret_pos_pre = m_caret_pos;
            m_caret_pos = caret_pos;
            m_caret_pos_dragstart = caret_pos;

            redraw_force = true;
        }

        // マウスジェスチャ(右ドラッグ)開始
        else if( m_control.button_alloted( event, CONTROL::GestureButton ) ) m_r_drugging = true;

        // オートスクロールボタン
        else if( m_control.button_alloted( event, CONTROL::AutoScrollButton ) ){

            if ( ! ( m_layout_current && m_layout_current->link ) ){ // リンク上で無いなら

                change_cursor( cursor_names::kAllScroll );

                m_scrollinfo.reset();
                m_scrollinfo.mode = SCROLL_AUTO;
                m_scrollinfo.show_marker = true;
                m_scrollinfo.enable_up = true;
                m_scrollinfo.enable_down = true;
                m_scrollinfo.x = static_cast<int>(event->x);
                m_scrollinfo.y = static_cast<int>(event->y);
            }
        }

        // ダブル、トリプルクリックしたら範囲選択
        else if( m_control.button_alloted( event, CONTROL::DblClickButton )
                 || m_control.button_alloted( event, CONTROL::TrpClickButton ) ){

            const bool triple = m_control.button_alloted( event, CONTROL::TrpClickButton );
            CARET_POSITION caret_left, caret_right;
            if( set_carets_dclick( caret_left, caret_right, x, y, triple ) ){

                set_selection( caret_left, caret_right );
                redraw_force = true;
                // GtkGestureMultiPressは指を離す度にreleasedが発行される
                // ダブルクリックの途中でfalseに戻らないように設定する
                m_drugging = true;
            }
        }
    }

    // 再描画
    if( redraw_force ) redraw_view_force();
    else redraw_view();

    m_sig_button_press.emit( url, res_num, event );

    return true;
}


//
// マウスボタンを離した
//
bool DrawAreaBase::slot_button_release_event( GdkEventButton* event )
{
    if( ! m_clicked ) return true;
    m_clicked = false;

    // リンクの上でコンテキストメニューを表示してからマウスを移動すると
    // slot_motion_notify_eventが呼び出されず m_layout_current が変わらないため
    // リンクを開いてしまう
    CARET_POSITION caret_pos;
    m_x_pointer = static_cast<int>(event->x);
    m_y_pointer = static_cast<int>(event->y);
    m_layout_current = set_caret( caret_pos, m_x_pointer , m_y_pointer + get_vscr_val() );

    std::string url;
    int res_num = 0;

    if( m_layout_current && m_layout_current->link ){
        url = m_layout_current->link;

        // ssspの場合は PROTO_SSSP を前に付ける
        if( m_layout_current->type == DBTREE::NODE_SSSP ) url = std::string( PROTO_SSSP ) + url;
    }
    if( m_layout_current ) res_num = m_layout_current->res_number;

    if( event->type == GDK_BUTTON_RELEASE ){

        // 範囲選択中だったら選択文字確定 & スクロール停止
        if( m_drugging && set_selection_str() ){

            // X のコピーバッファにコピー
            Glib::RefPtr< Gtk::Clipboard > clip = Gtk::Clipboard::get( GDK_SELECTION_PRIMARY );
            clip->set_text( m_selection.str );

            redraw_view();
            m_scrollinfo.reset();

            // リンククリック処理をキャンセル
            url = std::string();
        }

        // リンクのクリックを認識させないためオートスクロール解除直後はリンククリックの処理をしない
        if( m_scrollinfo.autoscroll_finished ){
            m_scrollinfo.reset();
            url = std::string();
        }

        m_drugging = false;
        m_r_drugging = false;

        // ダブルクリックで数字やID〜を範囲選択したときにon_urlシグナルを出す
        motion_mouse();
    }

    m_sig_button_release.emit( url, res_num, event );

    return true;
}



//
// マウスが動いた
//
bool DrawAreaBase::slot_motion_notify_event( GdkEventMotion* event )
{
    if( m_x_pointer == static_cast<int>(event->x) && m_y_pointer == static_cast<int>(event->y) ) return true;

    m_x_pointer = static_cast<int>(event->x);
    m_y_pointer = static_cast<int>(event->y);

    // ホイールスクロール終了直後はある程度マウスを動かすまでモーションイベントをキャンセル
    if( m_scrollinfo.counter_nomotion ){

#ifdef _DEBUG
        std::cout << "counter_nomotion = " << m_scrollinfo.counter_nomotion
                  << " x = " << m_scrollinfo.x
                  << " y = " << m_scrollinfo.x
                  << " xp = " << m_x_pointer
                  << " yp = " << m_x_pointer;
#endif

        m_scrollinfo.counter_nomotion
        =  MAX( 0,
               m_scrollinfo.counter_nomotion - abs( m_scrollinfo.x - m_x_pointer ) - abs( m_scrollinfo.y - m_y_pointer )
            );

        m_scrollinfo.x = m_x_pointer;
        m_scrollinfo.y = m_y_pointer;

#ifdef _DEBUG
        std::cout << " -> " << m_scrollinfo.counter_nomotion << std::endl;
#endif

        if( m_scrollinfo.counter_nomotion ) return true;

        m_scrollinfo.reset();
    }

    motion_mouse();

    m_sig_motion_notify.emit( event );
    return true;
}




//
// マウスが動いた時の処理
//
bool DrawAreaBase::motion_mouse()
{
    const int pos = get_vscr_val();
    CARET_POSITION caret_pos;

    // 現在のマウスポインタの下にあるレイアウトノードとキャレットの取得
    m_layout_current = set_caret( caret_pos, m_x_pointer , m_y_pointer + pos );

    int res_num = 0;
    std::string link_current;
    if( m_layout_current ){

        // 現在のポインタの下にあるレス番号取得
        res_num = m_layout_current->res_number;

        // 現在のポインタの下にあるリンクの文字列を更新
        // IDや数字だったらポップアップ表示する

        // リンクの上にポインタがある
        if( m_layout_current->link ){
            link_current =  m_layout_current->link;
        }

        // IDや数字などの範囲選択の上にポインタがある
        else link_current = get_selection_as_url( caret_pos );
    }

    bool link_changed = false;
    if( link_current != m_link_current ) link_changed = true; // 前回とリンクの文字列が変わった
    m_link_current = link_current;

    // ドラッグ中なら範囲選択
    if( m_drugging ){

        // ポインタが画面外に近かったらオートスクロールを開始する
        const int mrg = static_cast<int>( static_cast<double>(m_font->br_size) * 0.5 );

        // スクロールのリセット
        if (
             // スクロールページの一番上
             ( pos == 0 && m_y_pointer < m_view.get_height() - mrg )

             // スクロールページの一番下
             || ( pos >= get_vscr_maxval() && m_y_pointer > mrg )

             // ページの中央
             || ( m_y_pointer > mrg && m_y_pointer < m_view.get_height() - mrg ) ){

            m_scrollinfo.reset();
        }

        // スクロールモードをセット
        else if( m_scrollinfo.mode == SCROLL_NOT ){

            m_scrollinfo.mode = SCROLL_AUTO;
            m_scrollinfo.show_marker = false;
            m_scrollinfo.x = m_x_pointer;

            if( m_y_pointer <= mrg ){
                m_scrollinfo.enable_up = true;
                m_scrollinfo.enable_down = false;
                m_scrollinfo.y = mrg*6;
            }
            else{
                m_scrollinfo.enable_up = false;
                m_scrollinfo.enable_down = true;
                m_scrollinfo.y = m_view.get_height() - mrg*6;
            }
        }

        // スクロール中の場合は exec_scroll() で選択部分を描画する
        if( m_scrollinfo.mode == SCROLL_NOT ){
            RECTANGLE rect;
            if( set_selection( caret_pos, &rect ) ) draw_screen( rect.y, rect.height );
        }
    }

    // 右ドラッグしていないとき
    // 右ドラッグ中はリンクへの出入りを監視しない
    else if( ! m_r_drugging ){

        if( link_changed ){

            m_sig_leave_url.emit();

            // (別の)リンクノードに入った
            if( !m_link_current.empty()
                && m_scrollinfo.mode == SCROLL_NOT // スクロール中はon_urlシグナルを出さない
                ){

                std::string imgurl;
                const DBTREE::NODE* node = m_layout_current->node;
                if( node && node->linkinfo && node->linkinfo->imglink ) imgurl = node->linkinfo->imglink;

#ifdef _DEBUG
                std::cout << "slot_motion_notify_drawarea : enter link = " << m_link_current
                          << " imgurl = " << imgurl << std::endl;
#endif
                m_sig_on_url.emit( m_link_current, imgurl, res_num );
            }

            // リンクノードからでた
            else{
#ifdef _DEBUG
                std::cout << "slot_motion_notify_drawarea : leave\n";
#endif
            }
        }
    }

    change_cursor( get_cursor_type() );

    return true;
}



//
// 現在のポインターの下のノードからカーソルのタイプを決定する
//
Glib::ustring DrawAreaBase::get_cursor_type() const
{
    Glib::ustring cursor_type = cursor_names::kDefault;
    if( m_layout_current ){

        // テキストの上ではカーソルを I に変える
        if( m_layout_current->text && ! m_layout_current->link ) cursor_type = cursor_names::kText;

        // リンクの上にポインタがある
        if( m_layout_current->link ) cursor_type = cursor_names::kPointer;
    }

    return cursor_type;
}


//
// カーソルの形状の変更
//
void DrawAreaBase::change_cursor( const Glib::ustring& cursor_type )
{
    //オートスクロール中
    if( m_scrollinfo.mode == SCROLL_AUTO ) return;

    if( m_cursor_type != cursor_type ){
        m_cursor_type = cursor_type;
        if( m_cursor_type == cursor_names::kDefault ) m_window->set_cursor();
        else {
            m_window->set_cursor( Gdk::Cursor::create( m_window->get_display(), m_cursor_type ) );
        }
    }
}



//
// キーを押した
//
bool DrawAreaBase::slot_key_press_event( GdkEventKey* event )
{
    //オートスクロール中なら無視
    if( m_scrollinfo.mode == SCROLL_AUTO ) return true;

#ifdef _DEBUG
    std::cout << "DrawAreaBase::slot_key_press_event\n";
#endif

    if( m_key_press && m_keyval == event->keyval ) m_key_locked = true;
    m_key_press = true;
    m_keyval = event->keyval;

    return m_sig_key_press.emit( event );
}



//
// キーを離した
//
bool DrawAreaBase::slot_key_release_event( GdkEventKey* event )
{
    if( !m_key_press ) return true;

#ifdef _DEBUG
    std::cout << "DrawAreaBase::slot_key_release_event\n";
#endif

    m_scrollinfo.reset();
    if( m_key_locked ) redraw_view();

    m_key_press = false;
    m_key_locked = false;
    m_keyval = 0;

    m_sig_key_release.emit( event );
    return true;
}



void DrawAreaBase::setup_event_controller()
{
    m_view.add_events( Gdk::TOUCH_MASK );

    m_gesture_multipress = Gtk::GestureMultiPress::create( m_view );
    // 既存のシグナルハンドラに処理を委譲するので0を指定してどのボタンもリッスンする
    m_gesture_multipress->set_button( 0 );
    m_gesture_multipress->set_exclusive( true );
    m_gesture_multipress->set_touch_only( false );
    m_gesture_multipress->signal_pressed().connect( sigc::mem_fun( *this, &DrawAreaBase::slot_multipress_pressed ) );
    m_gesture_multipress->signal_released().connect( sigc::mem_fun( *this, &DrawAreaBase::slot_multipress_released ) );

    m_gesture_pan = Gtk::GesturePan::create( m_view, Gtk::ORIENTATION_VERTICAL );
    m_gesture_pan->set_touch_only( true );
    m_gesture_pan->signal_drag_begin().connect( sigc::mem_fun( *this, &DrawAreaBase::slot_pan_begin ) );
    m_gesture_pan->signal_drag_update().connect( sigc::mem_fun( *this, &DrawAreaBase::slot_pan_update ) );
    m_gesture_pan->signal_end().connect( sigc::mem_fun( *this, &DrawAreaBase::slot_gesture_end ) );

    m_gesture_swipe = Gtk::GestureSwipe::create( m_view );
    m_gesture_pan->group( m_gesture_swipe );
    m_gesture_swipe->set_touch_only( true );
    m_gesture_swipe->signal_swipe().connect( sigc::mem_fun( *this, &DrawAreaBase::slot_swipe ) );
}


//
// イベント構造体を取得してslot_button_(press|release)_eventに処理を委譲する
//
void DrawAreaBase::slot_multipress_pressed( int n_press, double, double )
{
    GdkEventSequence* const sequence = m_gesture_multipress->get_current_sequence();
    const GdkEvent* const event = m_gesture_multipress->get_last_event( sequence );
    if( !event || event->type != GDK_BUTTON_PRESS ) return;

    const unsigned int n_points = m_gesture_multipress->property_n_points();
    const unsigned int button = m_gesture_multipress->get_current_button();
#ifdef _DEBUG
    std::cout << "DrawAreaBase::slot_multipress_pressed "
              << "n_press = " << n_press << ", event = " << event->type << ", n_points = " << n_points
              << ", button = " << button << std::endl;
#endif

    const auto device = m_gesture_multipress->get_device();
    GdkEventButton button_event = event->button;
    if( n_press == 2 ) {
        button_event.type = GDK_DOUBLE_BUTTON_PRESS;
    }
    else if( n_press == 3 && ( device && device->get_source() != Gdk::SOURCE_TOUCHSCREEN ) ) {
        button_event.type = GDK_TRIPLE_BUTTON_PRESS;
    }
    if( n_points == 2 ) {
        // 2本指タップは右ボタンクリックとして扱う
        button_event.button = GDK_BUTTON_SECONDARY;
    }
    slot_button_press_event( &button_event );

    if( button >= 4 ) {
        // GtkGestureMultiPressを使うとButton4以上のマウスボタンで
        // button-release-eventの反応が悪くなる環境がある。
        // そのためpressedから直接releasedの処理を実行することによって問題を回避する。
        button_event.type = GDK_BUTTON_RELEASE;
        slot_button_release_event( &button_event );
        m_gesture_multipress->set_sequence_state( sequence, Gtk::EVENT_SEQUENCE_DENIED );
    }
}

void DrawAreaBase::slot_multipress_released( int n_press, double, double )
{
    static_cast< void >( n_press );

    GdkEventSequence* const sequence = m_gesture_multipress->get_current_sequence();
    const GdkEvent* const event = m_gesture_multipress->get_last_event( sequence );
    if( !event || event->type != GDK_BUTTON_RELEASE ) return;

    const unsigned int n_points = m_gesture_multipress->property_n_points();
#ifdef _DEBUG
    std::cout << "DrawAreaBase::slot_multipress_released "
              << "n_press = " << n_press << ", event = " << event->type << ", n_points = " << n_points
              << std::endl;
#endif

    GdkEventButton button_event = event->button;
    if( n_points == 2 ) {
        button_event.button = GDK_BUTTON_SECONDARY;
    }
    slot_button_release_event( &button_event );
}


void DrawAreaBase::slot_pan_begin( double start_x, double start_y )
{
    static_cast< void >( start_x );

    cancel_deceleration();

    Gtk::EventSequenceState state;
    if ( !m_vscrbar ) {
        state = Gtk::EVENT_SEQUENCE_DENIED;
    }
    else { // capture_button_press == true
        const auto adjust = m_vscrbar->get_adjustment();
        m_drag_start_y = adjust->get_value();

        state = Gtk::EVENT_SEQUENCE_CLAIMED;
    }

#ifdef _DEBUG
    std::cout << "DrawAreaBase::slot_pan_begin y = " << m_drag_start_y << std::endl;
#endif

    GdkEventSequence* const sequence = m_gesture_pan->get_current_sequence();
    m_gesture_pan->set_sequence_state( sequence, state );
}

void DrawAreaBase::slot_pan_update( double offset_x, double offset_y )
{
    assert( m_vscrbar );
    static_cast< void >( offset_x );

    GdkEventSequence* const sequence = m_gesture_pan->get_last_updated_sequence();
    if( m_gesture_pan->get_sequence_state( sequence ) != Gtk::EVENT_SEQUENCE_CLAIMED ) return;

    const double y = m_drag_start_y - offset_y;
    const auto adjust = m_vscrbar->get_adjustment();

#ifdef _DEBUG
    std::cout << "DrawAreaBase::slot_pan_update y = " << y << std::endl;
#endif

    if( y <= 0 ) return;
    if( y >= adjust->get_upper() - adjust->get_page_size() ) return;

    // パンの最中は必ずスクロール処理を実施する
    m_wait_scroll = Monotonic::duration::zero();

    m_scrollinfo.reset();
    m_scrollinfo.y = static_cast< int >( y );
    m_scrollinfo.mode = SCROLL_TO_Y;

    exec_scroll();
}

void DrawAreaBase::slot_gesture_end( GdkEventSequence* sequence )
{
    if( !m_gesture_pan->handles_sequence( sequence ) ) {
        m_gesture_pan->set_state( Gtk::EVENT_SEQUENCE_DENIED );
    }
}



namespace
{
    constexpr double kDecelerationFriction = 4.0; // スクロール速度の係数
    constexpr double kDecelerationRatio = 0.000001; // 経過時間を切り下げて減速をゆるやかにする
    constexpr double kInitialDyScale = 0.01; // pixcels/secからpixcels/frameに換算する係数
}

//
// 慣性スクロールを実行する (指を離した後も減速しながらスクロールする)
//
void DrawAreaBase::slot_swipe( double velocity_x, double velocity_y )
{
    if( !m_vscrbar ) return;

    static_cast< void >( velocity_x );

    GdkEventSequence* const sequence = m_gesture_swipe->get_last_updated_sequence();
    if( m_gesture_swipe->get_sequence_state( sequence ) != Gtk::EVENT_SEQUENCE_CLAIMED ) return;

#ifdef _DEBUG
    std::cout << "DrawAreaBase::slot_swipe vx = " << velocity_x << ", vy = " << velocity_y << std::endl;
#endif
    GdkFrameClock* const borrowed_clock = gtk_widget_get_frame_clock( GTK_WIDGET( this->gobj() ) );
    m_deceleration.last_time = gdk_frame_clock_get_frame_time( borrowed_clock );
    m_deceleration.elapsed = 0.0;
    m_deceleration.initial_dy = velocity_y * kInitialDyScale;
    m_deceleration.id = gtk_widget_add_tick_callback( GTK_WIDGET( this->gobj() ),
                                                      &DrawAreaBase::deceleration_tick_cb, nullptr, nullptr );
}

gboolean DrawAreaBase::deceleration_tick_cb( GtkWidget* cwidget, GdkFrameClock* clock, gpointer )
{
    Gtk::Widget* const widget = Glib::wrap( cwidget );
    if( auto area = dynamic_cast<DrawAreaBase*>( widget ) ) {
        return area->deceleration_tick_impl( clock );
    }
    return G_SOURCE_REMOVE;
}

gboolean DrawAreaBase::deceleration_tick_impl( GdkFrameClock* clock )
{
    const gint64 current_time = gdk_frame_clock_get_frame_time( clock );
    m_deceleration.elapsed += static_cast<double>( current_time - m_deceleration.last_time ) * kDecelerationRatio;
    m_deceleration.last_time = current_time;
    const double exp_part = std::exp( -kDecelerationFriction * m_deceleration.elapsed );
    const double dy = -kDecelerationFriction * exp_part * m_deceleration.initial_dy;

#ifdef _DEBUG
    std::cout << "DrawAreaBase::deceleration_tick_impl dy = " << dy << std::endl;
#endif
    if( std::fabs( dy ) < 1.0 ) {
        cancel_deceleration();
        return G_SOURCE_REMOVE;
    }

    // 減速中は必ずスクロール処理を実施する
    m_wait_scroll = Monotonic::duration::zero();

    m_scrollinfo.reset();
    m_scrollinfo.dy = static_cast< int >( dy );
    m_scrollinfo.mode = SCROLL_NORMAL;

    exec_scroll();
    return G_SOURCE_CONTINUE;
}

void DrawAreaBase::cancel_deceleration()
{
    if( m_deceleration.id && GTK_IS_WIDGET( this->gobj() ) ) {
        gtk_widget_remove_tick_callback( GTK_WIDGET( this->gobj() ), m_deceleration.id );
        m_deceleration.id = 0;
    }
}
