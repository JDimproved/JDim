// ライセンス: GPL2

//#define _DEBUG
//#define _DEBUG_CARETMOVE
#include "jddebug.h"

#include "drawareabase.h"
#include "layouttree.h"
#include "font.h"

#include "jdlib/miscutil.h"
#include "jdlib/miscmsg.h"
#include "jdlib/jdregex.h"

#include "dbtree/articlebase.h"
#include "dbtree/node.h"
#include "dbtree/interface.h"

#include "dbimg/imginterface.h"
#include "dbimg/img.h"

#include "config/globalconf.h"

#include "global.h"
#include "httpcode.h"
#include "controlid.h"

#include <math.h>
#include <sstream>

using namespace ARTICLE;

#define AUTOSCR_CIRCLE 24 // オートスクロールの時のサークルの大きさ
#define BIG_WIDTH 100000
#define BIG_HEIGHT 100000

#define SCROLLSPEED_FAST ( m_vscrbar ? m_vscrbar->get_adjustment()->get_page_size() : 0 )
#define SCROLLSPEED_MID  ( m_vscrbar ? m_vscrbar->get_adjustment()->get_page_size()/2 : 0 )
#define SCROLLSPEED_SLOW ( m_vscrbar ? m_vscrbar->get_adjustment()->get_step_increment()*2 : 0 )

#define SEPARATOR_HEIGHT 6 // 新着セパレータの高さ


//////////////////////////////////////////////////////////



DrawAreaBase::DrawAreaBase( const std::string& url )
    : m_url( url )
    , m_vscrbar( 0 )
    , m_layout_tree( 0 )
    , m_seen_current( 0 )
    , m_window( 0 )
    , m_gc( 0 )
    , m_backscreen( 0 )
    , m_pango_layout( 0 )
    , m_draw_frame( false )
{
#ifdef _DEBUG
    std::cout << "DrawAreaBase::DrawAreaBase " << m_url << std::endl;;
#endif
}



DrawAreaBase::~DrawAreaBase()
{
#ifdef _DEBUG
    std::cout << "DrawAreaBase::~DrawAreaBase " << m_url << std::endl;;
#endif

    if( m_layout_tree ) delete m_layout_tree;
    m_layout_tree = NULL;
    clear();
}


// 背景色( virtual )
const int* DrawAreaBase::rgb_color_back()
{
    return  CONFIG::get_color_back();
}


// フォント
const std::string& DrawAreaBase::fontname()
{
    return CONFIG::get_fontname_main();

}

// フォントモード
const int DrawAreaBase::fontmode()
{
    return FONT_MAIN;
}


//
// セットアップ
//
// show_abone : あぼーんされたスレも表示
// show_scrbar : スクロールバーを最初から表示
//
void DrawAreaBase::setup( bool show_abone, bool show_scrbar )
{
    if( m_layout_tree ) delete m_layout_tree;
    m_layout_tree = NULL;
    clear();

    m_article = DBTREE::get_article( m_url );
    m_layout_tree = new LayoutTree( m_url, show_abone );

    m_view.set_double_buffered( false );

    // デフォルトではoffになってるイベントを追加
    m_view.add_events( Gdk::BUTTON_PRESS_MASK );
    m_view.add_events( Gdk::BUTTON_RELEASE_MASK );
    m_view.add_events( Gdk::SCROLL_MASK );
    m_view.add_events( Gdk::POINTER_MOTION_MASK );
    m_view.add_events( Gdk::LEAVE_NOTIFY_MASK );

    // focus 可にセット
    m_view.set_flags( m_view.get_flags() | Gtk::CAN_FOCUS );
    m_view.add_events( Gdk::KEY_PRESS_MASK );
    m_view.add_events( Gdk::KEY_RELEASE_MASK );

    // イベント接続
    m_view.signal_leave_notify_event().connect(  sigc::mem_fun( *this, &DrawAreaBase::slot_leave_notify_event ) );
    m_view.signal_realize().connect( sigc::mem_fun( *this, &DrawAreaBase::slot_realize ));
    m_view.signal_configure_event().connect(  sigc::mem_fun( *this, &DrawAreaBase::slot_configure_event ));
    m_view.signal_expose_event().connect(  sigc::mem_fun( *this, &DrawAreaBase::slot_expose_event ));    
    m_view.signal_scroll_event().connect(  sigc::mem_fun( *this, &DrawAreaBase::slot_scroll_event ));        
    m_view.signal_button_press_event().connect(  sigc::mem_fun( *this, &DrawAreaBase::slot_button_press_event ));
    m_view.signal_button_release_event().connect(  sigc::mem_fun( *this, &DrawAreaBase::slot_button_release_event ));
    m_view.signal_motion_notify_event().connect(  sigc::mem_fun( *this, &DrawAreaBase::slot_motion_notify_event ));
    m_view.signal_key_press_event().connect( sigc::mem_fun(*this, &DrawAreaBase::slot_key_press_event ));
    m_view.signal_key_release_event().connect( sigc::mem_fun(*this, &DrawAreaBase::slot_key_release_event ));

    pack_start( m_view );

    // pango layout 作成
    m_pango_layout = m_view.create_pango_layout( "" );
    m_pango_layout->set_width( -1 ); // no wrap

    // 色、フォント初期化
    init_color();
    init_font();

    // スクロールバー作成
    if( show_scrbar ) create_scrbar();

    show_all_children();
}


//
// 変数初期化
//
void DrawAreaBase::clear()
{
    m_scrollinfo.reset();

    m_selection.select = false;
    m_separator_new = 0;
    m_multi_selection.clear();
    m_layout_current = NULL;
    m_width_client = 0;
    m_height_client = 0;
    m_drugging = false;
    m_r_drugging = false;
    m_pre_pos_y = 0;
    m_key_press = false;
    m_goto_num_reserve = 0;
    m_wheel_scroll_time = 0;
    m_caret_pos = CARET_POSITION();
    m_caret_pos_pre = CARET_POSITION();
    m_caret_pos_current = CARET_POSITION();
    m_caret_pos_dragstart = CARET_POSITION();
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
    m_vscrbar = Gtk::manage( new Gtk::VScrollbar() );
    m_event = Gtk::manage( new Gtk::EventBox() );
    assert( m_vscrbar );
    assert( m_event );

    m_event->add( *m_vscrbar );
    pack_start( *m_event, Gtk::PACK_SHRINK );
    m_vscrbar->get_adjustment()->signal_value_changed().connect( sigc::mem_fun( *this, &DrawAreaBase::slot_change_adjust ) );

    show_all_children();
}




//
// 色初期化
//
void DrawAreaBase::init_color()
{
    Glib::RefPtr< Gdk::Colormap > colormap = get_default_colormap();
    const int *rgb;
    
    // 文字色
    rgb = CONFIG::get_color_char();
    m_color[ COLOR_CHAR ].set_rgb( rgb[ 0 ], rgb[ 1 ], rgb[ 2 ] );

    rgb = CONFIG::get_color_char_age();
    m_color[ COLOR_CHAR_AGE ].set_rgb( rgb[ 0 ], rgb[ 1 ], rgb[ 2 ] );

    m_color[ COLOR_CHAR_NAME ] = Gdk::Color( "darkgreen" );    
    m_color[ COLOR_CHAR_SELECTION  ] = Gdk::Color( "white" );
    m_color[ COLOR_CHAR_LINK ] = Gdk::Color( "blue" );
    m_color[ COLOR_CHAR_LINK_PUR ] = Gdk::Color( "magenta" );
    m_color[ COLOR_CHAR_LINK_RED ] = Gdk::Color( "red" );    
    m_color[ COLOR_CHAR_HIGHLIGHT ] = Gdk::Color( "black" );
    m_color[ COLOR_CHAR_BOOKMARK ] = Gdk::Color( "red" );    

    // 背景色
    rgb = rgb_color_back();
    m_color[ COLOR_BACK ].set_rgb( rgb[ 0 ], rgb[ 1 ], rgb[ 2 ] );
    m_color[ COLOR_BACK_SELECTION ] = Gdk::Color( "blue" );
    m_color[ COLOR_BACK_HIGHLIGHT ] = Gdk::Color( "yellow" );

    // 新着セパレータ
    rgb = CONFIG::get_color_separator();
    m_color[ COLOR_SEPARATOR_NEW ].set_rgb( rgb[ 0 ], rgb[ 1 ], rgb[ 2 ] );

    // 画像
    m_color[ COLOR_IMG_NOCACHE ] = Gdk::Color( "brown" );
    m_color[ COLOR_IMG_LOADING ] = Gdk::Color( "darkorange" );
    m_color[ COLOR_IMG_CACHED ]  = Gdk::Color( "darkcyan" );
    m_color[ COLOR_IMG_ERR ] = Gdk::Color( "red" );

    for( int i = 0; i < COLOR_NUM; ++i ) colormap->alloc_color( m_color[ i ] );
}



//
// フォント初期化
//
void DrawAreaBase::init_font()
{
    if( fontname().empty() ) return;

    m_context = get_pango_context();
    assert( m_context );

    // layoutにフォントをセット
    Pango::FontDescription pfd( fontname() );
    pfd.set_weight( Pango::WEIGHT_NORMAL );
    m_pango_layout->set_font_description( pfd );
    modify_font( pfd );

    // フォント情報取得
    Pango::FontMetrics metrics = m_context->get_metrics( pfd );
    m_font_ascent = PANGO_PIXELS( metrics.get_ascent() );
    m_font_descent = PANGO_PIXELS( metrics.get_descent() );
    m_font_height = m_font_ascent + m_font_descent;

    // 改行高さ ( トップからの距離 )
    m_br_size = ( int )( m_font_height * CONFIG::get_adjust_line_space() );

    const char* wstr = "あいうえお";
    m_pango_layout->set_text( wstr );

    // リンクの下線の位置 ( トップからの距離 )
#if GTKMMVER <= 240
    m_underline_pos = int( ( m_pango_layout->get_pixel_logical_extents().get_height() )
                           * CONFIG::get_adjust_underline_pos() );
#else
    m_underline_pos = PANGO_PIXELS( ( metrics.get_ascent() - metrics.get_underline_position() )
                                  * CONFIG::get_adjust_underline_pos() ); 
#endif

    // 左右マージン幅取得
    // マージン幅は真面目にやると大変そうなので文字列 wstr の平均を取る
    int width = m_pango_layout->get_pixel_ink_extents().get_width() / 5;
    m_mrg_right = width /2 * 3;
    m_mrg_left = width;

    // 字下げ量
    m_down_size = m_mrg_left;
}


//
// クロック入力
//
void DrawAreaBase::clock_in()
{
    exec_scroll( false );
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
    if( !m_gc ) return;

    get_window()->set_cursor();

    m_key_press = false;
    if( m_scrollinfo.mode != SCROLL_AUTO ) m_scrollinfo.reset();

}


// 範囲選択中の文字列
const std::string DrawAreaBase::str_selection()
{
    if( ! m_selection.select ) return std::string();

    return m_selection.str;
}



//
// 表示されている最後のレスの番号
//
int DrawAreaBase::max_number()
{
    assert( m_layout_tree );

    return m_layout_tree->max_res_number();
}


//
// from_num　から to_num までレスをappendして再レイアウト
//
void DrawAreaBase::append_res( int from_num, int to_num )
{
    assert( m_article );
    assert( m_layout_tree );

#ifdef _DEBUG
    std::cout << "DrawAreaBase::append_res from " << from_num << " to " << to_num << std::endl;
#endif

    for( int num = from_num; num <= to_num; ++num ) m_layout_tree->append_node( m_article->res_header( num ), false );

    // クライアント領域のサイズをリセットして再レイアウト
    m_width_client = 0;
    m_height_client = 0;
    layout();
}



//
// リストで指定したレスをappendして再レイアウト
//
void DrawAreaBase::append_res( std::list< int >& list_resnum )
{
    std::list< bool > list_joint;
    append_res( list_resnum, list_joint );
}



//
// リストで指定したレスをappendして再レイアウト( 連結情報付き )
// 
// list_joint で連結指定したレスはヘッダを取り除いて前のレスに連結する
//
void DrawAreaBase::append_res( std::list< int >& list_resnum, std::list< bool >& list_joint )
{
    assert( m_article );
    assert( m_layout_tree );

    if( list_resnum.size() == 0 ) return;

#ifdef _DEBUG
    std::cout << "DrawAreaBase::append_res" << std::endl;
#endif

    bool use_joint = ( list_joint.size() == list_resnum.size() );
    
    std::list< int >::iterator it = list_resnum.begin();
    std::list< bool >::iterator it_joint = list_joint.begin();
    for( ; it != list_resnum.end(); ++it ){

        bool joint = false;
        if( use_joint ){
            joint = ( *it_joint );
            ++it_joint;
        }

#ifdef _DEBUG
        std::cout << "append no. " << ( *it ) << " joint = " << joint << std::endl;
#endif

        m_layout_tree->append_node( m_article->res_header( ( *it ) ), joint );
    }

    // クライアント領域のサイズをリセットして再レイアウト
    m_width_client = 0;
    m_height_client = 0;
    layout();
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
    layout();
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
    layout();
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
    layout();
    redraw_view();
}



//
// バックスクリーンを描き直して再描画予約(queue_draw())する。再レイアウトはしない
//
void DrawAreaBase::redraw_view()
{
#ifdef _DEBUG    
    std::cout << "DrawAreaBase::redraw_view()\n";
#endif

    draw_backscreen( true );
    m_view.queue_draw();
}





//
// レイアウト実行
//
void DrawAreaBase::layout()
{
    layout_impl( false, 0, 0 );
}




//
// 先頭ノードから順に全ノードの座標を計算する(描画はしない)
//
// nowrap = true なら wrap なしで計算
// offset_y は y 座標の上オフセット行数
// right_mrg は右マージン量(ピクセル)
//
void DrawAreaBase::layout_impl( bool nowrap, int offset_y, int right_mrg )
{
    if( ! m_layout_tree ) return;

    // レイアウトがセットされていない
    if( ! m_layout_tree->top_header() ) return;
    
    // drawareaのウィンドウサイズ

    // nowrap = true の時は十分大きい横幅で計算して wrap させない
    const int width = nowrap ? BIG_WIDTH : m_view.get_width();
    const int height = m_view.get_height();
    const int min_height = 2;

#ifdef _DEBUG
    std::cout << "DrawAreaBase::layout_impl : nowrap = " << nowrap << " width = " << width << " height  = " << height<< std::endl
              << "m_width_client = " << m_width_client << " m_height_client = " << m_height_client << std::endl;
#endif

    //表示はされてるがまだリサイズしてない状況
    if( !nowrap && height < min_height ){
#ifdef _DEBUG
        std::cout << "drawarea is not resized yet.\n";
#endif        
        return;
    }

    m_width_client = 0;
    m_height_client = 0;
    int mrg_level; // 現在の左字下げレベル
    int x, y = 0;
    LAYOUT* tmpheader = m_layout_tree->top_header();

    y += offset_y * m_br_size;
    
    while( tmpheader ){

        // ヘッダの座標等をセット
        mrg_level = 0;
        tmpheader->mrg_level = mrg_level;
        x = m_mrg_left + m_down_size * tmpheader->mrg_level;
        tmpheader->x = x;
        tmpheader->y = y;
        tmpheader->width = 0;

        // 先頭の子ノードから順にレイアウトしていく
        LAYOUT* tmplayout  = tmpheader->next_layout;
        while ( tmplayout ){
            
            layout_one_node( tmplayout, x, y, width, mrg_level );

            // ブロック幅更新
            int width_tmp = tmplayout->width + m_mrg_left + m_down_size * tmplayout->mrg_level;
            if( width_tmp> tmpheader->width ) tmpheader->width = width_tmp;
            
            tmplayout = tmplayout->next_layout;
        }
        y += m_br_size;
        
        // ブロック高さ確定
        tmpheader->height = y - tmpheader->y;

        // クライアント領域全体幅更新
        if( tmpheader->width > m_width_client ) m_width_client = tmpheader->width;

        tmpheader = tmpheader->next_header;

        // ブロック間はスペースを入れる
        y += m_br_size;
    }

    // クライアント領域の幅、高さ確定
    m_width_client += right_mrg;
    m_height_client = y;

#ifdef _DEBUG    
    std::cout << "virtual size of drawarea : m_width_client = " << m_width_client
              << " m_height_client = " << m_height_client << std::endl;
#endif

    // 実際に画面に表示されてない
    if( !m_window ) return;

    // 表示はされてるがまだリサイズしてない状況
    if( height < min_height ) return;
    
    // スクロールバーが表示されていないならここで作成
    if( ! m_vscrbar && m_height_client > height ) create_scrbar();

    // adjustment 範囲変更
    Gtk::Adjustment* adjust = m_vscrbar ? m_vscrbar->get_adjustment(): NULL;
    if( adjust ){

        double current = adjust->get_value();
        adjust->set_lower( 0 );
        adjust->set_upper( y + m_br_size );
        adjust->set_page_size(  height );
        adjust->set_step_increment( m_br_size );
        adjust->set_page_increment(  height / 2 );
        adjust->set_value( MAX( 0, MIN( adjust->get_upper() - adjust->get_page_size() , current ) ) );
    }

    // 裏描画画面作成と初期描画
#ifdef _DEBUG    
    std::cout << "create backscreen : width = " << m_view.get_width() << " height = " << m_view.get_height() << std::endl;
#endif

    m_backscreen.clear();
    m_backscreen = Gdk::Pixmap::create( m_window, m_view.get_width(), m_view.get_height() );
    draw_backscreen( true );

    // 予約されているならジャンプ予約を実行
    if( m_goto_num_reserve ) goto_num( m_goto_num_reserve );
}



//
// ノードひとつのレイアウト関数
//
// draw_one_node()と違い実際に描画はしない。ノードの座標計算に使う
//
// x,y : ノードの初期座標(左上)、次のノードの左上座標が入って返る
// mrg_level : 字下げ下げレベル
//
void DrawAreaBase::layout_one_node( LAYOUT* layout, int& x, int& y, int width_view, int& mrg_level )
{
    int width, height;

    switch( layout->type ){

        case DBTREE::NODE_IDNUM: // 発言数ノード

            if( !set_num_id( layout ) ) break;

        case DBTREE::NODE_TEXT: // テキスト
        case DBTREE::NODE_LINK: // リンク

            layout->mrg_level = mrg_level;    
            layout->x = x;
            layout->y = y;
            if( ! layout->lng_text ) layout->lng_text = strlen( layout->text );

            // 次のノードの左上座標を計算(x,y,width,height,が参照なので更新された値が戻る)
            layout_draw_one_node( layout, x, y, width, height, width_view, false, layout->bold );

            layout->width = width;
            layout->height = height;

            break;


        case DBTREE::NODE_BR: // 改行
        
            layout->x = x;
            layout->y = y;
        
            x = m_mrg_left + m_down_size * mrg_level;        
            y += m_br_size;

            break;

            // スペース
        case DBTREE::NODE_ZWSP: break;

        case DBTREE::NODE_DOWN_LEFT: // 字下げ
            ++mrg_level;
            x = m_mrg_left + m_down_size * mrg_level;
            break;
    }
}



//
// バックスクリーン描画
//
// ここで書かれた画面がexposeイベントで表画面にコピーされる
//
// redraw_all = true なら全画面を描画、falseならスクロールした分だけ
//
bool DrawAreaBase::draw_backscreen( bool redraw_all )
{
    if( ! m_gc ) return false;
    if( ! m_backscreen ) return false;
    if( ! m_layout_tree ) return false;
    if( ! m_layout_tree->top_header() ) return false;

    int width_view = m_view.get_width();
    int height_view = m_view.get_height();
    int pos_y = get_vscr_val();

    // 移動量、再描画範囲の上限、下限
    int dy = 0;
    int upper = pos_y;
    int lower = upper + height_view;

    m_gc->set_foreground( m_color[ COLOR_BACK ] );

    // 画面全体を再描画
    if( redraw_all ){
        m_backscreen->draw_rectangle( m_gc, true, 0, 0, width_view,  height_view );
    }
    else{

        dy = pos_y - m_pre_pos_y;

        // 上にスクロールした
        if( dy > 0 ){

            // キーを押しっぱなしの時は描画を省略する
            if( m_key_press ) dy = MIN( dy, (int)SCROLLSPEED_SLOW );

            if( dy < height_view ){
                upper += ( height_view - dy );
                m_backscreen->draw_drawable( m_gc, m_backscreen, 0, dy, 0, 0, width_view , height_view - dy );
            }

            m_backscreen->draw_rectangle( m_gc, true, 0, MAX( 0, height_view - dy ), width_view, MIN( dy, height_view ) );
        }

        // 下にスクロールした
        else if( dy < 0 ){

            // キーを押しっぱなしの時は描画を省略する
            if( m_key_press ) dy = MAX( dy, (int)-SCROLLSPEED_SLOW );

            if( -dy < height_view ){
                lower = upper - dy;
                lower += SEPARATOR_HEIGHT * 2; // 新着セパレータの分を広げる
                m_backscreen->draw_drawable( m_gc, m_backscreen, 0, 0, 0, -dy, width_view , height_view + dy );
            }

            m_backscreen->draw_rectangle( m_gc, true, 0, 0, width_view, MIN( -dy, height_view ) );
        }
    }
    
    m_pre_pos_y = pos_y;

    if( !redraw_all && ! dy ) return false;

#ifdef _DEBUG
//    std::cout << "DrawAreaBase::draw_backscreen all = " << redraw_all << " y = " << pos_y <<" dy = " << dy << std::endl;
#endif    

    // 新着セパレータのの位置の調整
    // あぼーんなどで表示されていないときは表示位置を移動する
    if( m_separator_new && ! m_layout_tree->get_header_of_res( m_separator_new ) ){

        int num = m_separator_new;
        int max_number = m_layout_tree->max_res_number();
        while( ! m_layout_tree->get_header_of_res( num ) && num++ < max_number );

        if( m_layout_tree->get_header_of_res( num ) ) m_separator_new = num;;
    }

    // 一番最後のレスが半分以上表示されていたら最後のレス番号をm_seen_currentにセット
    m_seen_current = 0;
    int num = m_layout_tree->max_res_number();
    const LAYOUT* lastheader = m_layout_tree->get_header_of_res( num );

    // あぼーんなどで表示されていないときは前のレスを調べる
    if( !lastheader ){
        while( ! m_layout_tree->get_header_of_res( num ) && num-- > 0 );
        lastheader = m_layout_tree->get_header_of_res( num );
    }
    if( lastheader && lastheader->y + lastheader->height/2 < pos_y + height_view ){

        m_seen_current = m_layout_tree->max_res_number();
    }

    // ノード描画
    LAYOUT* tmpheader = m_layout_tree->top_header();
    while( tmpheader ){

        // 現在みているスレ番号取得
        if( ! m_seen_current ){

            if( ! tmpheader->next_header // 最後のレス
                || ( tmpheader->next_header->y >= ( pos_y + m_br_size ) // 改行分下にずらす
                     && tmpheader->y <= ( pos_y + m_br_size ) ) ){

                m_seen_current = tmpheader->res_number;
            }
        }

        // ヘッダが範囲に含まれてるなら描画
        if( tmpheader->y + tmpheader->height > upper
            && tmpheader->y < lower ){

            // ノードが範囲に含まれてるなら描画
            LAYOUT* tmplayout  = tmpheader;
            while ( tmplayout ){
                if( tmplayout->y + tmplayout->height > upper && tmplayout->y < lower ) draw_one_node( tmplayout, width_view, pos_y );
                tmplayout = tmplayout->next_layout;
            }
        }

        tmpheader = tmpheader->next_header;        
    }

    return true;
}


//
// 枠の描画
//
void DrawAreaBase::draw_frame()
{
    int width_win = m_view.get_width();
    int height_win = m_view.get_height();
    m_gc->set_foreground( m_color[ COLOR_CHAR ] );
    m_window->draw_rectangle( m_gc, false, 0, 0, width_win-1, height_win-1 );
}



//
// バックスクリーンを表画面に描画
//
// draw_backscreen()でバックスクリーンを描画してから呼ぶこと
// 
bool DrawAreaBase::draw_drawarea( int x, int y, int width, int height )
{
    if( ! m_layout_tree ) return false;

    // まだ realize してない
    if( !m_gc ) return false;

    // レイアウトがセットされていない or まだリサイズしていない( m_backscreen == NULL )
    // なら画面消去して終わり
    if( ! m_layout_tree->top_header()
        || ! m_backscreen
        ){
        m_window->set_background( m_color[ COLOR_BACK ] );
        m_window->clear();
        return false;
    }
    
    if( !width )  width = m_view.get_width();
    if( !height ) height = m_view.get_height();

    // バックスクリーンをコピー
    m_window->draw_drawable( m_gc, m_backscreen, x, y, x, y, width, height );

#if 0
    // キャレット描画
    int xx = m_caret_pos.x;
    int yy = m_caret_pos.y - pos_y;
    if( yy >= 0 ){
        m_gc->set_foreground( m_color[ COLOR_CHAR ] );
        m_window->draw_line( m_gc, xx, yy, xx, yy + m_underline_pos );
    }
#endif

    // オートスクロールのマーカ
    if( m_scrollinfo.mode != SCROLL_NOT && m_scrollinfo.show_marker ){
        m_gc->set_foreground( m_color[ COLOR_CHAR ] );
        m_window->draw_arc( m_gc, false, m_scrollinfo.x - AUTOSCR_CIRCLE/2, m_scrollinfo.y - AUTOSCR_CIRCLE/2 ,
                            AUTOSCR_CIRCLE, AUTOSCR_CIRCLE, 0, 360 * 64 );
    }

    // フレーム描画
    if( m_draw_frame ) draw_frame();

    return true;
}


//
// 一文字の幅を取得
//
// wide_mode :  全角半角モード( 前の文字が全角ならtrue、半角ならfalse )
// mode : フォントのモード( スレビューのフォントかポップアップのフォントか)
//
int DrawAreaBase::get_width_of_one_char( const char* str, int& byte, bool& wide_mode, const int mode )
{
    int width, width_wide;
    ARTICLE::get_width_of_char( str, byte, width, width_wide, mode );

    // キャッシュに無かったら幅を調べてキャッシュに登録
    if( ! width || ! width_wide ){

        char tmpchar[ byte + 8 ];
        memcpy( tmpchar, str, byte );
        tmpchar[ byte ] = '\0';

        // 半角モードでの幅
        // ダミーの半角文字を前に付けて幅を取得
        const std::string dummy = "a";

        m_pango_layout->set_text( dummy + dummy );
        int width_dummy = m_pango_layout->get_pixel_logical_extents().get_width() / 2;

        m_pango_layout->set_text( dummy + tmpchar );
        width = m_pango_layout->get_pixel_logical_extents().get_width() - width_dummy;

        // 全角モードでの幅
        m_pango_layout->set_text( tmpchar );
        width_wide = m_pango_layout->get_pixel_logical_extents().get_width();

#ifdef _DEBUG
        if( width != width_wide ){
            std::cout << "DrawAreaBase::get_width_of_one_char c = ["
                      << tmpchar << "] w = " << width << " wide = " << width_wide  << std::endl;
        }
#endif

        // フォントが無い
        if( width <=0 && width_wide <= 0 ){

            int byte_tmp;
            unsigned int code = MISC::utf8toucs2( tmpchar, byte_tmp );

            std::stringstream ss_err;
            ss_err << "unknown font byte = " << byte_tmp << " code = " << code << " " << width << std::endl;

#ifdef _DEBUG
            std::cout << "DrawAreaBase::get_width_of_one_char "
                      << "byte = " << byte
                      << " byte_tmp = " << byte_tmp
                      << " code = " << code
                      << " [" << tmpchar << "]\n";
#endif        

            MISC::ERRMSG( ss_err.str() );

            width = width_wide = 0;
        }

        ARTICLE::set_width_of_char( str, byte, width, width_wide, mode );
    }

    // 全角、アルファベット以外の文字は、前の文字が半角か全角かで幅を切り替える
    // また全角半角モードの切り替えはしない
    int ret = 0;
    char tmp_char = str[ 0 ];
    if( ! (
            ( tmp_char >= 'a' && tmp_char <= 'z' )
            || ( tmp_char >= 'A' && tmp_char <= 'Z' ) )
        ){

        if( wide_mode ) ret = width_wide;
        else ret = width;
    }
    else{

        ret = width_wide;

        // 全角半角モード切り替え
        if( byte != 1 || ( byte == 1 && tmp_char == ' ' ) ) wide_mode = true;
        else wide_mode = false;
    }

    return ret;
}


//
// ノードひとつを描画する関数
//
// width_view : 描画領域の幅
// pos_y : 描画領域の開始座標
//
void DrawAreaBase::draw_one_node( LAYOUT* layout, const int& width_view, const int& pos_y )
{
    if( ! m_article ) return;

    // ノード種類別の処理
    switch( layout->type ){

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
                    else if( img->get_code() == HTTP_OK ) node->color_text = COLOR_IMG_CACHED;
                    else if( img->get_code() == HTTP_ERR || img->get_code() == HTTP_INIT ) node->color_text = COLOR_IMG_NOCACHE;
                    else node->color_text = COLOR_IMG_ERR;
                }
            }

            // テキストノード
        case DBTREE::NODE_TEXT:

            draw_one_text_node( layout, width_view, pos_y );
            break;


            // 発言回数ノード
        case DBTREE::NODE_IDNUM:

            if( set_num_id( layout ) ) draw_one_text_node( layout, width_view, pos_y );
            break;

        // ヘッダ
        case DBTREE::NODE_HEADER:

            // ブックマークのマーク描画
            if( layout->res_number && m_article->is_bookmarked( layout->res_number ) ){

                int y = layout->y - pos_y;

                m_pango_layout->set_text( ">" );
                m_backscreen->draw_layout( m_gc, 0, y, m_pango_layout, m_color[ COLOR_CHAR_BOOKMARK ], m_color[ COLOR_BACK ] );
            }

            // 新着セパレータ
            if( m_separator_new && m_separator_new == layout->res_number ){

                int y = layout->y - pos_y - SEPARATOR_HEIGHT *2;
                m_gc->set_foreground( m_color[ COLOR_SEPARATOR_NEW ] );
                m_backscreen->draw_rectangle( m_gc, true, 0, y, width_view, SEPARATOR_HEIGHT );
            }

            break;


            // ノードが増えたらここに追加していくこと

        default:
            break;
    }
}



//
// テキストの含まれているノードひとつを描画する関数
//
// width_view : 描画領域の幅
// pos_y : 描画領域の開始座標
//
void DrawAreaBase::draw_one_text_node( LAYOUT* layout, const int& width_view, const int& pos_y )
{
    int x = layout->x;
    int y = layout->y - pos_y;
    int width, height;

    int x_selection;
    int y_selection;
    
    int id_header = layout->id_header;
    int id = layout->id ;
    
    int id_header_from = 0;
    int id_from = 0;

    int id_header_to = 0;
    int id_to = 0;

    unsigned int byte_from = 0;
    unsigned int byte_to = 0;

    bool bold = layout->bold;

    // 範囲選択の描画をする必要があるかどうかの判定
    // 二度書きすると重いので、ちょっと式は複雑になるけど同じ所を二度書きしないようにする
    bool draw_selection = false;
    if( m_selection.select && m_selection.caret_from.layout && m_selection.caret_to.layout ){

        draw_selection = true;
    
        id_header_from = m_selection.caret_from.layout->id_header;
        id_from = m_selection.caret_from.layout->id;

        id_header_to = m_selection.caret_to.layout->id_header;
        id_to = m_selection.caret_to.layout->id;

        // 選択開始ノードの場合は m_selection.caret_from.byte から、それ以外は0バイト目から描画
        byte_from =  m_selection.caret_from.byte * ( id_header == id_header_from && id == id_from );

        // 選択終了ノードの場合は m_selection.caret_to.byte から、それ以外は最後まで描画
        byte_to =  m_selection.caret_to.byte * ( id_header == id_header_to && id == id_to );
        if( byte_to == 0 ) byte_to = strlen( layout->text );
        
        if( byte_from == byte_to

            //  このノードは範囲選択外なので範囲選択の描画をしない
            || ( id_header < id_header_from )
            || ( id_header > id_header_to )
            || ( id_header == id_header_from && id < id_from )
            || ( id_header == id_header_to && id > id_to )

            // キャレットが先頭にあるので範囲選択の描画をしない
            ||  ( id_header == id_header_to && id == id_to && m_selection.caret_to.byte == 0 )
            ){

            draw_selection = false;
        }

    }

    int color_text = COLOR_CHAR;
    if( layout->color_text ) color_text = *layout->color_text;
    
    // 通常描画
    if( ! draw_selection ){
        layout_draw_one_node( layout, x, y, width, height, width_view, true, bold, color_text, COLOR_BACK );

    } else { // 範囲選択の前後描画

        // 前
        if( byte_from ) layout_draw_one_node( layout, x, y, width, height, width_view, true, bold, color_text, COLOR_BACK, 0, byte_from );

        // ここでは選択部分の座標計算のみ( do_draw = false )してハイライト後に選択部分を描画
        x_selection = x;
        y_selection = y;
        layout_draw_one_node( layout, x, y, width, height, width_view, false, bold, color_text, COLOR_BACK,  byte_from, byte_to );
    
        // 後
        if( byte_to != strlen( layout->text ) ) layout_draw_one_node( layout, x, y, width, height, width_view, true, bold, color_text, COLOR_BACK, byte_to );
    }
     
    // 検索結果のハイライト
    if( m_multi_selection.size() > 0 ){

        std::list< SELECTION >::iterator it;
        for( it = m_multi_selection.begin(); it != m_multi_selection.end(); ++it ){
            if( layout->id_header == ( *it ).caret_from.layout->id_header && layout->id == ( *it ).caret_from.layout->id ){

                // 描画開始位置の座標を計算( do_draw = false )、
                x = layout->x;
                y = layout->y - pos_y;
                int byte_from2 = ( *it ).caret_from.byte;
                int byte_to2 = ( *it ).caret_to.byte;
                if( byte_from2 ) layout_draw_one_node( layout, x, y, width, height,
                                                       width_view, false, bold, COLOR_CHAR_HIGHLIGHT , COLOR_BACK_HIGHLIGHT, 0, byte_from2 );

                // ハイライト描画
                layout_draw_one_node( layout, x, y, width, height,
                                      width_view, true, bold, COLOR_CHAR_HIGHLIGHT , COLOR_BACK_HIGHLIGHT, byte_from2, byte_to2 );
            }
        }
    }

    // 範囲選択部分を描画
    if( draw_selection && byte_from != byte_to ){
        layout_draw_one_node( layout, x_selection, y_selection, width, height, width_view, true, bold,
                                            COLOR_CHAR_SELECTION , COLOR_BACK_SELECTION, byte_from, byte_to );
    }
}



//
// 実際にレイアウトノード内の文字の座標を計算したり描画する関数
//
// node_x,node_y (参照)  : ノードの初期座標(左上)、次のノードの左上座標が入って返る
// node_width, node_height (参照) : ノードの幅、高さが入って返る
// do_draw : true なら描画, false なら座標計算のみ
// byte_from バイト目の文字から byte_to バイト目の「ひとつ前」の文字まで描画
// byte_to が 0 なら最後まで描画
// bold : 太字
//
// たとえば node->text = "abcdefg" で byte_from = 1, byte_to = 3 なら "bc" を描画
//
void DrawAreaBase::layout_draw_one_node( LAYOUT* node, int& node_x, int& node_y, int& node_width, int& node_height, int width_view,
                                         bool do_draw, bool bold, int color , int color_back, int byte_from, int byte_to )
{
    assert( node->text != NULL );

    if( ! node->lng_text ) return;
    if( byte_from >= node->lng_text ) return;
    if( byte_to > node->lng_text ) byte_to = node->lng_text;
    if( byte_to == 0 ) byte_to = node->lng_text; 

    node_width = 0;
    node_height = m_br_size;

    bool wide_mode = true;
    int pos_start = byte_from;
    for(;;){

        // 横に何文字並べるか計算

        int byte_char;
        int pos_to = pos_start;
        int n_str = 0;
        int n_ustr = 0;
        int width_line = 0;

        // 右端がはみ出るまで文字を足していく

        bool draw_head = true; // 最低1文字は描画
        while( ( node_x + width_line < width_view - m_mrg_right // > ならwrap が起こったということ
                 &&  pos_to < byte_to ) || draw_head ) {

            width_line += get_width_of_one_char( node->text + pos_to, byte_char, wide_mode, fontmode() );
            pos_to += byte_char;
            n_str += byte_char;
            ++n_ustr;
            draw_head = false;
        }

        // pos_start から pos_to の前まで描画
        if( do_draw ){

#ifdef USE_PANGOLAYOUT  // Pango::Layout を使って文字を描画

            m_pango_layout->set_text( Glib::ustring( node->text + pos_start, n_ustr ) );
            m_backscreen->draw_layout( m_gc, node_x, node_y, m_pango_layout, m_color[ color ], m_color[ color_back ] );

            if( bold ){
                m_gc->set_foreground( m_color[ color ] );
                m_backscreen->draw_layout( m_gc, node_x+1, node_y, m_pango_layout );
            }
            
#else // Pango::GlyphString を使って文字を描画

            assert( m_context );

            m_gc->set_foreground( m_color[ color_back ] );
            m_backscreen->draw_rectangle( m_gc, true, node_x, node_y, width_line, m_font_height );

            m_gc->set_foreground( m_color[ color ] );

            Pango::AttrList attr;
            std::string text = std::string( node->text + pos_start, n_str );
            std::list< Pango::Item > list_item = m_context->itemize( text, attr );

            int xx = node_x;
            std::list< Pango::Item >::iterator it = list_item.begin();
            for( ; it != list_item.end(); ++it ){

                Pango::Item &item = *it;
                Pango::GlyphString grl = item.shape( text.substr( item.get_offset(), item.get_length() ) ) ;
                Pango::Rectangle rect = grl.get_logical_extents(  item.get_analysis().get_font() );
                int width = PANGO_PIXELS( rect.get_width() );

                m_backscreen->draw_glyphs( m_gc, item.get_analysis().get_font(), xx, node_y + m_font_ascent, grl );
                if( bold ) m_backscreen->draw_glyphs( m_gc, item.get_analysis().get_font(), xx+1, node_y + m_font_ascent, grl );
                xx += width;
            }

            if( xx - node_x != width_line ){
                std::stringstream ss_err;
                ss_err << "estimating width failed : " << text << " e = " << width_line << " r = " << xx - node_x;
                MISC::ERRMSG( ss_err.str() );
            }

#endif
            // リンクの時は下線を引く
            if( node->link && CONFIG::get_draw_underline() ){

                m_gc->set_foreground( m_color[ color ] );
                m_backscreen->draw_line( m_gc, node_x, node_y + m_underline_pos, node_x + width_line, node_y + m_underline_pos );
            }
        }

        if( bold ) ++width_line;

        // ノードの幅更新
        node_x += width_line;
        int width_tmp = node_x - ( m_mrg_left + m_down_size * node->mrg_level );
        if( width_tmp > node_width ) node_width = width_tmp;
        
        // wrap 処理
        if( node_x >= width_view - m_mrg_right ) {

            // 改行
            node_x = ( m_mrg_left + m_down_size * node->mrg_level );            
            node_y += m_br_size;

            // ノード高さ更新
            node_height += m_br_size;
        }

        if( pos_to >= byte_to ) break;
        pos_start = pos_to;
    }
}




// 整数 -> 文字変換してノードに発言数をセット
// 最大4桁を想定
int DrawAreaBase::set_num_id( LAYOUT* layout )
{
    int pos = 0;

    int num_id = layout->header->node->headinfo->num_id_name;
    if( num_id >= 2 ){

        layout->node->text[ pos++] = ' ';
        layout->node->text[ pos++] = '(';
        int div_tmp = 1;
        if( num_id / 1000 ) div_tmp = 1000;
        else if( num_id / 100 ) div_tmp = 100;
        else if( num_id / 10 ) div_tmp = 10;
        while( div_tmp ){

            int tmp_val = num_id / div_tmp;
            num_id -= tmp_val * div_tmp;
            div_tmp /= 10;

            layout->node->text[ pos++] = '0' + tmp_val;
        }
        layout->node->text[ pos++] = ')';
        layout->node->text[ pos ] = '\0';
        layout->lng_text = pos;
    }

    return pos;
}




//
// スクロール方向指定
//
// 実際にスクロールして描画を実行するのは exec_scroll()
//
bool DrawAreaBase::set_scroll( const int& control )
{
    if( !m_vscrbar ){
        m_scrollinfo.reset();
        return false;
    }
    double dy = 0;

    int y = get_vscr_val();
    bool enable_down = ( y < get_vscr_maxval() );
    bool enable_up = ( y > 0 );

    if( m_scrollinfo.mode == SCROLL_NOT ){

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
        }

        if( dy ){
        
            m_scrollinfo.reset();
            m_scrollinfo.dy = ( int ) dy;
            
            // キーを押しっぱなしにしてる場合スクロールロックする
            if( m_key_press ) m_scrollinfo.mode = SCROLL_LOCKED;
        
            // レスポンスを上げるため押した直後はすぐ描画
            else{

                m_scrollinfo.mode = SCROLL_NORMAL;
                exec_scroll( false );
            }

            return true;
        }
    }

    return false;
}




//
// マウスホイールの処理
//
void DrawAreaBase::wheelscroll( GdkEventScroll* event )
{
    const int time_cancel = 15; // msec
    if( !m_vscrbar ) return;

    // あまり速く動かしたならキャンセル
    int time_tmp = event->time - m_wheel_scroll_time;

    if( ( ! m_wheel_scroll_time || time_tmp >= time_cancel  ) && event->type == GDK_SCROLL ){

        m_wheel_scroll_time = event->time;

        if( m_vscrbar && ( m_scrollinfo.mode == SCROLL_NOT || m_scrollinfo.mode == SCROLL_NORMAL ) ){

            Gtk::Adjustment* adjust = m_vscrbar->get_adjustment();

            m_scrollinfo.reset();
            m_scrollinfo.mode = SCROLL_NORMAL;
        
            if( event->direction == GDK_SCROLL_UP ) m_scrollinfo.dy = -( int ) adjust->get_step_increment() *3;
            else if( event->direction == GDK_SCROLL_DOWN ) m_scrollinfo.dy = ( int ) adjust->get_step_increment() *3;

            exec_scroll( false );
        }
    }
}



//
// スクロールやジャンプを実行して再描画
//
// clock_in()からクロック入力される度にスクロールする
//
// redraw_all : true なら全画面再描画
//
void DrawAreaBase::exec_scroll( bool redraw_all )
{
    if( ! m_layout_tree ) return;
    if( ! m_vscrbar ) return;
    if( m_scrollinfo.mode == SCROLL_NOT ) return;

    // 移動後のスクロール位置を計算
    int y = 0;
    Gtk::Adjustment* adjust = m_vscrbar->get_adjustment();
    switch( m_scrollinfo.mode ){

        case SCROLL_TO_NUM: // 指定したレス番号にジャンプ
        {
#ifdef _DEBUG
            std::cout << "DrawAreaBase::exec_scroll : goto " << m_scrollinfo.res << std::endl;
#endif        
            const LAYOUT* layout = m_layout_tree->get_header_of_res( m_scrollinfo.res );
            if( layout ) y = layout->y;
            m_scrollinfo.reset();
        }
        break;

        // 先頭、最後に移動
        case SCROLL_TO_TOP:
            y = 0;
            m_scrollinfo.reset();
            break;

        case SCROLL_TO_BOTTOM:
            y = (int) adjust->get_upper();
            m_scrollinfo.reset();
            break;
        
        case SCROLL_NORMAL: // 1 回だけスクロール

            y = ( int ) adjust->get_value() + m_scrollinfo.dy;
            m_scrollinfo.reset();
            break;

        case SCROLL_LOCKED: // ロックが外れるまでスクロールを続ける

            y = ( int ) adjust->get_value() + m_scrollinfo.dy;

            break;

        case SCROLL_AUTO:  // オートスクロールモード
        {
            // 現在のポインタの位置取得
            int x_point, y_point;
            m_view.get_pointer( x_point, y_point );
            double dy = m_scrollinfo.y - y_point;

            if( dy >= 0 && ! m_scrollinfo.enable_up ) dy = 0;
            else if( dy < 0 && ! m_scrollinfo.enable_down ) dy = 0;
            else{

                // この辺の式は経験的に決定
                if( -AUTOSCR_CIRCLE/4 <= dy && dy <= AUTOSCR_CIRCLE/4 ) dy = 0;
                else dy =  ( dy / fabs( dy ) ) * MIN( ( exp( ( fabs( dy ) - AUTOSCR_CIRCLE/4 ) /50 ) -1 ) * 5,
                                                      adjust->get_page_size() * 3 );
                if( m_drugging ) dy *= 4;  // 範囲選択中ならスピード上げる
            }

            y = ( int ) adjust->get_value() -( int ) dy;

            // 範囲選択中ならキャレット移動して選択範囲更新
            if( m_drugging ){

                CARET_POSITION caret_pos;
                int y_tmp = MIN( MAX( 0, y_point ), m_view.get_height() );
                set_caret( caret_pos, x_point , y + y_tmp );
                set_selection( caret_pos );
            }
        }
        break;
    
    }

    y = (int)MAX( 0, MIN( adjust->get_upper() - adjust->get_page_size() , y ) );
    adjust->set_value( y );

    // キーを押しっぱなしの時に一番上か下に着いたらスクロール停止
    if( m_scrollinfo.mode == SCROLL_LOCKED && ( y <= 0 || y >= adjust->get_upper() - adjust->get_page_size() ) ){
        m_scrollinfo.reset();
        redraw_all = true;
    }

    // 再描画
    if( draw_backscreen( redraw_all ) ) draw_drawarea();
}



//
// スクロールバーの現在値
//
int DrawAreaBase::get_vscr_val()
{
    if( m_vscrbar ) return ( int ) m_vscrbar->get_adjustment()->get_value();
    return 0;
}


//
// スクロールバーの最大値値
//
int DrawAreaBase::get_vscr_maxval()
{
    if( m_vscrbar ) return ( int ) ( m_vscrbar->get_adjustment()->get_upper()
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
        std::cout << "reserve goto_num(1)\n";
#endif
        return;
    }

    // 表示範囲を越えていたら再レイアウトしたときにもう一度呼び出し
    else if( num > max_number() ){
        m_goto_num_reserve = num;

#ifdef _DEBUG
        std::cout << "reserve goto_num(2)\n";
#endif
        return;
    }

    else m_goto_num_reserve = 0;

    // ロード数を越えている場合
    int number_load = DBTREE::article_number_load( m_url );
    if( number_load < num ) num = number_load;

    // num番が表示されていないときは近くの番号をセット
    while( ! m_layout_tree->get_header_of_res( num ) && num++ < number_load );
    while( ! m_layout_tree->get_header_of_res( num ) && num-- > 1 );

#ifdef _DEBUG
    std::cout << "exec goto_num\n";
#endif

    // スクロール実行
    m_scrollinfo.reset();
    m_scrollinfo.mode = SCROLL_TO_NUM;
    m_scrollinfo.res = num;
    exec_scroll( true );
}


//
// 先頭、新着、最後に移動
//
void DrawAreaBase::goto_top()
{
    if( m_vscrbar ){
        m_scrollinfo.reset();
        m_scrollinfo.mode = SCROLL_TO_TOP;
        exec_scroll( true );
    }
}

void DrawAreaBase::goto_new()
{
    if( m_separator_new ){
        int num = m_separator_new > 1 ? m_separator_new -1 : 1;
        goto_num( num );
    }
}

void DrawAreaBase::goto_bottom()
{
    if( m_vscrbar ){
        m_scrollinfo.reset();
        m_scrollinfo.mode = SCROLL_TO_BOTTOM;
        exec_scroll( true );
    }
}


//
// 検索実行
//
bool DrawAreaBase::search( std::list< std::string >& list_query, bool reverse )
{
    assert( m_layout_tree );

    JDLIB::Regex regex;

    if( list_query.size() == 0 ) return false;

#ifdef _DEBUG    
    std::cout << "ArticleViewBase::search size = " << list_query.size() << std::endl;
#endif

    m_multi_selection.clear();

    // 先頭ノードから順にサーチして m_multi_selection に選択箇所をセットしていく
    LAYOUT* tmpheader = m_layout_tree->top_header();
    while( tmpheader ){

        LAYOUT* tmplayout = tmpheader->next_layout;
        while( tmplayout ){

            // (注意) 今のところレイアウトノードをまたがった検索は出来ない
            if( ( tmplayout->type == DBTREE::NODE_TEXT
                  || tmplayout->type == DBTREE::NODE_IDNUM
                  || tmplayout->type == DBTREE::NODE_LINK )
                && tmplayout->text ){

                std::string text = tmplayout->text;
                int offset = 0;
                for(;;){

                    int lng = 0;
                    std::list< std::string >::iterator it_query;
                    for( it_query = list_query.begin(); it_query != list_query.end() ; ++it_query ){

                        std::string query = ( *it_query );
                        if( regex.exec( query, tmplayout->text, offset, true, true, true ) ){

                            offset = regex.pos( 0 );
                            lng = regex.str( 0 ).length();

#ifdef _DEBUG
                            std::cout << "id = " << tmplayout->id <<  " offset = " << offset << " lng = " << lng
                                      << " " << text.substr( offset, lng ) << std::endl;
#endif

                            break;
                        }
                    }

                    if( lng == 0 ) break;

                    // 選択設定
                    SELECTION selection;
                    selection.select = false;
                    selection.caret_from.set( tmplayout, offset );
                    selection.caret_to.set( tmplayout, offset + lng );
                    m_multi_selection.push_back( selection );
                    offset += lng;
                }
            }

            tmplayout = tmplayout->next_layout;
        }

        tmpheader = tmpheader->next_header;
    }

#ifdef _DEBUG    
    std::cout << "m_multi_selection.size = " << m_multi_selection.size() << std::endl;
#endif
    
    if( m_multi_selection.size() == 0 ) return false;

    // 初期位置をセット
    // selection.select = true のアイテムが現在選択中
    std::list< SELECTION >::iterator it;
    for( it = m_multi_selection.begin(); it != m_multi_selection.end(); ++it ){
        
        if( ( *it ).caret_from < m_caret_pos ) continue;
        ( *it ).select = true;
        break;
    }

    if( it == m_multi_selection.end() ) m_multi_selection.back().select = true;
    
    // search_move でひとつ進めるのでひとつ前に戻しておく
    if( ! reverse ){
        if( it != m_multi_selection.end() ) ( *it ).select = false;
        if( it == m_multi_selection.begin() )  m_multi_selection.back().select = true;
        else ( *( --it ) ).select = true;            
    }
    
    redraw_view();
    return true;
}



//
// 次の検索結果に移動
//
void DrawAreaBase::search_move( bool reverse )
{
#ifdef _DEBUG
    std::cout << "ArticleViewBase::search_move " << m_multi_selection.size() << std::endl;
#endif

    if( ! m_vscrbar ) return;
    if( m_multi_selection.size() == 0 ) return;

    std::list< SELECTION >::iterator it;
    for( it = m_multi_selection.begin(); it != m_multi_selection.end(); ++it ){

        if( ( *it ).select ){

            ( *it ).select = false;

            // 前に移動
            if( reverse ){
                if( it == m_multi_selection.begin() ) it = m_multi_selection.end();
                --it;
            }

            // 次に移動
            else{
                if( ( ++it ) == m_multi_selection.end() ) it = m_multi_selection.begin();
            }

            ( *it ).select = true;

            // 移動先を範囲選択状態にする
            m_caret_pos_dragstart = ( *it ).caret_from;
            set_selection( ( *it ).caret_to, false );

            int y = MAX( 0, ( *it ).caret_from.layout->y - 10 );

#ifdef _DEBUG
            std::cout << "move to y = " << y << std::endl;
#endif            

            Gtk::Adjustment* adjust = m_vscrbar->get_adjustment();
            if( ( int ) adjust->get_value() > y || ( int ) adjust->get_value() + ( int ) adjust->get_page_size() - m_br_size < y )
                adjust->set_value( y );

            return;
        }
    }
}


//
// ハイライト解除
//
void DrawAreaBase::clear_highlight()
{
    m_multi_selection.clear();
    redraw_view();
}    




//
// 座標(x,y)を与えてキャレットの位置を計算してCARET_POSITIONに値をセット
//、ついでに(x,y)の下にあるレイアウトノードも調べる
//
// CARET_POSITION& caret_pos : キャレットの位置が計算されて入る
//
// 戻り値: 座標(x,y)の下のレイアウトノード。ノード外にある場合はNULL
//
LAYOUT* DrawAreaBase::set_caret( CARET_POSITION& caret_pos,  int x, int y )
{
    if( ! m_layout_tree ) return NULL;
    
#ifdef _DEBUG_CARETMOVE
    std::cout << "DrawAreaBase::set_caret\n";
#endif

    int width_view = m_view.get_width();
    
    // 先頭のレイアウトブロックから順に調べる
    LAYOUT* tmpheader = m_layout_tree->top_header();
    while( tmpheader ){

        // y が含まれているブロックだけチェックする
        int height_block = tmpheader->next_header ? ( tmpheader->next_header->y - tmpheader->y ) : BIG_HEIGHT;
        if( tmpheader->y <= y && tmpheader->y + height_block >= y ){        

#ifdef _DEBUG_CARETMOVE
            std::cout << "header id = " << tmpheader->id_header << std::endl;
#endif                    
            // ブロック内のノードを順に調べていく
            LAYOUT* tmplayout = tmpheader->next_layout;
            while( tmplayout ){

#ifdef _DEBUG_CARETMOVE
                std::cout << "node id = " << tmplayout->id << std::endl;
#endif

                if( ! tmplayout->text ){
                    tmplayout = tmplayout->next_layout;
                    continue;
                }
            
                int tmp_x = tmplayout->x;
                int tmp_y = tmplayout->y;
                const char* pos = tmplayout->text;
                int byte_char = 0;

                // 左のマージンの上にポインタがある場合
                if( ( tmp_y <= y && tmp_y + m_br_size >= y  ) && x <= tmp_x ){

#ifdef _DEBUG_CARETMOVE
                    std::cout << "found: left\n";
#endif
                    // 左端にキャレットをセットして終了
                    caret_pos.set( tmplayout, 0, x, tmp_x, tmp_y, 0, byte_char );
                    return NULL;
                }

                // ノードの中のテキストを調べていく
                bool wide_mode = true;
                while( *pos != '\0' ){

                    int char_width = get_width_of_one_char( pos, byte_char, wide_mode, fontmode() );
                                    
                    // マウスポインタの下にノードがある場合
                    if( ( tmp_x <= x && tmp_x + char_width >= x )
                        && ( tmp_y <= y && tmp_y + m_br_size >= y ) ){

#ifdef _DEBUG_CARETMOVE                  
                        std::cout << "found: on node\n";
                        if( tmplayout->link != NULL ) std::cout << "link = " << tmplayout->link << std::endl;
#endif
                        // キャレットをセットして終了
                        caret_pos.set( tmplayout, ( int )( pos - tmplayout->text ), x, tmp_x, tmp_y, char_width, byte_char );
                        return tmplayout;
                    }

                    // 次の文字へ
                    pos += byte_char;
                    tmp_x += char_width;

                    if( tmp_x >= width_view - m_mrg_right ){ // wrap

                        // wrapが起きたときに右のマージンの上にポインタがある場合
                        if( ( tmp_y <= y && tmp_y + m_br_size >= y  ) && x >= tmp_x ){ 
#ifdef _DEBUG_CARETMOVE
                            std::cout << "found: right (wrap)\n";
#endif
                            // 右端にキャレットをセットして終わり
                            caret_pos.set( tmplayout, ( int )( pos - tmplayout->text ) - byte_char, x, tmp_x, tmp_y, 0, 0 );
                            return NULL;
                        }

                        // 改行
                        tmp_x = m_mrg_left + m_down_size * tmplayout->mrg_level;
                        tmp_y += m_br_size;
                    }
                }

                // とりあえずノードの中にはポインタは無かったので
                // 今のノードと次のノード or ブロックの間にyが無いか調べる

                int next_y = -1; // 次のノード or ブロックのy座標
                LAYOUT* tmplayout2 = tmplayout->next_layout;

                // 次のノードを取得する(文字列の含まれないノードは飛ばす)
                while( tmplayout2 && !tmplayout2->text ) tmplayout2 = tmplayout2->next_layout;

                // 次のノードのy座標を取得
                if( tmplayout2 ) next_y = tmplayout2->y;

                // 最終ノードなので次のブロックの y 座標を取得
                else{
                    
                    if( tmpheader->next_header ) next_y = tmpheader->next_header->y;
                    else next_y = y + BIG_HEIGHT; // 最終ブロックの最終行ということ
                }
                
                if(
                    // 次のノード or ブロックに行くとyを飛び越える場合
                    next_y > y 

                    // 右のマージンの上にポインタがある場合
                    || (
                        ( tmplayout->type == DBTREE::NODE_BR // 改行ノード
                          || tmplayout->next_layout == NULL // 最終ノード
                          || tmplayout->next_layout->type == DBTREE::NODE_BR // 一番右端のノード
                            )
                        && ( tmp_y <= y && tmp_y + m_br_size >= y  ) && x >= tmp_x )

                    ){
#ifdef _DEBUG_CARETMOVE
                    std::cout << "found: right\n";
#endif
                    // 現在のノードの右端にキャレットをセット
                    caret_pos.set( tmplayout, ( int )( pos - tmplayout->text ), x, tmp_x, tmp_y, 0, 0 );
                    return NULL;
                }

                // 次のノードへ
                tmplayout = tmplayout->next_layout;
            }
        }

        // 次のブロックへ
        tmpheader = tmpheader->next_header;        
    }

    return NULL;
}



//
// (x,y)地点をダブルクリック時の範囲選択のためのキャレット位置を取得
//
// caret_left : 左側のキャレット位置
// caret_left : 右側のキャレット位置
//
// 戻り値 : 成功すると true
//
bool DrawAreaBase::set_carets_dclick( CARET_POSITION& caret_left, CARET_POSITION& caret_right,  int x, int y )
{
    if( ! m_layout_tree ) return false;

#ifdef _DEBUG
    std::cout << "DrawAreaBase::set_carets_dclick\n";
#endif
    
    int width_view = m_view.get_width();
    
    // 先頭のレイアウトブロックから順に調べる
    LAYOUT* tmpheader = m_layout_tree->top_header();
    while( tmpheader ){

        // y が含まれているブロックだけチェックする
        int height_block = tmpheader->next_header ? tmpheader->next_header->y - tmpheader->y : BIG_HEIGHT;
        if( tmpheader->y <= y && tmpheader->y + height_block >= y ){        

            // ブロック内のノードを順に調べていく
            LAYOUT* tmplayout = tmpheader->next_layout;
            while( tmplayout ){

                if( ! tmplayout->text ){
                    tmplayout = tmplayout->next_layout;
                    continue;
                }
            
                int tmp_x = tmplayout->x;
                int tmp_y = tmplayout->y;
                int byte_char = 0;
                const char* pos = tmplayout->text;

                int x_left = 0, y_left = 0;
                const char* pos_left = NULL;
                const char* pos_right = NULL;

                const char* pos_tmp = pos;
                bool mode_ascii = ( *( ( unsigned char* )( pos ) ) < 128 );

                // ノードの中のテキストを調べていく
                bool wide_mode = true;
                while( *pos != '\0' ){

                    int char_width =  get_width_of_one_char( pos, byte_char, wide_mode, fontmode() );

                    // x,yの下にノードがある場合
                    // 左側の位置を確定
                    if( ! pos_left && tmp_x <= x && tmp_x + char_width >= x && tmp_y <= y && tmp_y + m_br_size >= y ){
                        pos_left = pos_tmp;
                    }

                    unsigned char code = *( ( unsigned char* )( pos ) );
                    unsigned char code_next = *( ( unsigned char* )( pos + byte_char ) );

                    if( code_next == '\0'
                        || code == ' '
                        || code == ','
                        || ( mode_ascii && code_next >= 128 )
                        || ( !mode_ascii && code_next < 128 ) ){

                        // 左側の仮位置を移動
                        if( ! pos_left ){

                            x_left = tmp_x;
                            y_left = tmp_y;

                            pos_tmp = pos + byte_char;
                            mode_ascii = ( *( ( unsigned char* )( pos_tmp ) ) < 128 );
                        }

                        // 確定
                        else{

                            pos_right = pos;
                            int x_right = tmp_x;

                            if( code != ' ' && code != ',' ){
                                pos_right += byte_char;
                                tmp_x += char_width;
                            }

                            // キャレット設定
                            caret_left.set( tmplayout, ( int )( pos_left - tmplayout->text ), x_left, x_left, y_left );
                            caret_right.set( tmplayout, ( int )( pos_right - tmplayout->text ), x_right , x_right, tmp_y );

#ifdef _DEBUG
                            std::cout << "from : " << caret_left.byte << std::endl;
                            std::cout << "to : " << caret_right.byte << std::endl;
                            std::cout << "string : " << std::string( caret_left.layout->text )
                            .substr( caret_left.byte, caret_right.byte - caret_left.byte ) << std::endl;
#endif

                            return true;
                        }
                    }
                    
                    pos += byte_char;
                    tmp_x += char_width;

                    if( tmp_x >= width_view - m_mrg_right ){ // wrap

                        // 改行
                        tmp_x = m_mrg_left + m_down_size * tmplayout->mrg_level;
                        tmp_y += m_br_size;
                    }
                }

                // 次のノードへ
                tmplayout = tmplayout->next_layout;
            }

            return false;
        }

        // 次のブロックへ
        tmpheader = tmpheader->next_header;        
    }

    return false;
}



//
// 範囲選択の範囲を計算してm_selectionにセット & 範囲選択箇所の再描画
//
//
// caret_left から caret_right まで範囲選択状態にする
// redraw : true なら再描画, false なら範囲の計算のみ
//

bool DrawAreaBase::set_selection( CARET_POSITION& caret_left, CARET_POSITION& caret_right )
{
    m_caret_pos_pre = caret_left;
    m_caret_pos = caret_left;
    m_caret_pos_dragstart = caret_left;
    return set_selection( caret_right );
}



//
// 範囲選択の範囲を計算してm_selectionにセット & 範囲選択箇所のバックスクリーンの再描画
//
// caret_pos : 移動後のキャレット位置
// redraw : true なら再描画, false なら範囲の計算のみ
//
// m_caret_pos_pre から caret_pos まで範囲選択状態にする
//
bool DrawAreaBase::set_selection( CARET_POSITION& caret_pos, bool redraw )
{
    if( ! caret_pos.layout ) return false;
    if( ! m_caret_pos_dragstart.layout ) return false;

    // 前回の呼び出しからキャレット位置が変わってない
    if( m_caret_pos == caret_pos ) return false;

    int pos_y = get_vscr_val();
    int width_view = m_view.get_width();

    m_caret_pos_pre = m_caret_pos;;
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
            m_selection.caret_from = m_caret_pos;;
            m_selection.caret_to = m_caret_pos_dragstart;
        }
        else{
            m_selection.caret_from = m_caret_pos_dragstart;
            m_selection.caret_to = m_caret_pos;
        }
    }

    if( !redraw ) return true;


    /////////////////////////////////////////////////
    //
    // 前回呼び出した位置(m_caret_pos_pre.layout) から現在の位置( m_caret_pos.layout) までバックスクリーンを再描画
    //

    LAYOUT* layout = m_caret_pos_pre.layout;
    LAYOUT* layout_to = m_caret_pos.layout;

    if( !layout ) layout = m_caret_pos_dragstart.layout;

    // layout_toの方が前だったらポインタを入れ換え
    if( layout_to->id_header < layout->id_header
        || ( layout_to->id_header == layout->id_header && layout_to->id < layout->id ) ){
        LAYOUT* layout_tmp = layout_to;
        layout_to = layout;
        layout = layout_tmp;
    }

#ifdef _DEBUG
    std::cout << "redraw layout from : " << layout->id_header << ":" << layout->id
              << " to " << layout_to->id_header <<  ":" << layout_to->id << std::endl;
#endif

    while ( layout ){

        draw_one_node( layout, width_view, pos_y );
        if( layout == layout_to ) break;

        if( layout->next_layout ) layout = layout->next_layout;
        else if( layout->header->next_header ) layout = layout->header->next_header;
        else break;
    }

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

    m_selection.str.clear();
    if( !m_selection.select ) return false;
    
#ifdef _DEBUG
    std::cout << "DrawAreaBase::set_selection_str\n";
    std::cout << "from header = " << m_selection.caret_from.layout->id_header << " node = " <<  m_selection.caret_from.layout->id;
    std::cout << " byte = " <<  m_selection.caret_from.byte << std::endl;
    std::cout << "to   header = " << m_selection.caret_to.layout->id_header << " node = " << m_selection.caret_to.layout->id;
    std::cout << " byte = " << m_selection.caret_to.byte  << std::endl;
#endif
    
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

                else if( tmplayout->text ){
                    if( copy_from || copy_to ) m_selection.str += std::string( tmplayout->text ).substr( copy_from, copy_to - copy_from );
                    else m_selection.str += tmplayout->text;
                }
            }

            // 終了
            if( tmplayout == m_selection.caret_to.layout ) return true;
            
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
// 範囲選択範囲にcaret_posが含まれていて、かつ条件(IDや数字など)を満たしていたらURLとして範囲選択文字を返す
//
// TODO: ノード間をまたがっていると取得できない
//
std::string DrawAreaBase::get_selection_as_url( const CARET_POSITION& caret_pos )
{
    std::string url;
    LAYOUT* layout = caret_pos.layout;

    if( !layout || ! m_selection.select || m_selection.str.empty() ) return url;

    if( layout->id_header == m_selection.caret_from.layout->id_header
        && layout->id == m_selection.caret_from.layout->id
        && caret_pos.byte >= m_selection.caret_from.byte
        && caret_pos.byte <= m_selection.caret_to.byte
        ){

        unsigned int n,dig;
        std::string select_str = MISC::remove_space( m_selection.str );
        int num = MISC::str_to_uint( select_str.c_str(), dig, n );

        // 数字
        if( n == strlen( select_str.c_str() ) && dig && num ) url = PROTO_ANCHORE + MISC::itostr( num );

        // ID
        else if( select_str.find( "ID:" ) == 0 ) url = select_str;
    }

#ifdef _DEBUG
//    if( !url.empty() ) std::cout << "DrawAreaBase::get_selection_as_url : " << url << std::endl;
#endif

    return url;
}


//
// VScrollbar が動いた
//
void DrawAreaBase::slot_change_adjust()
{
    if( m_scrollinfo.mode != SCROLL_NOT ) return; // スクロール中
    
    m_scrollinfo.reset();
    m_scrollinfo.mode = SCROLL_NORMAL;
    exec_scroll( false );
}



//
// drawarea がリサイズした
//
bool DrawAreaBase::slot_configure_event( GdkEventConfigure* event )
{
#ifdef _DEBUG    
    std::cout << "DrawAreaBase::slot_configure_event x = " << event->x << " y =  " << event->y
              << " width = " << m_view.get_width() << " heith = " << m_view.get_height() << std::endl;
#endif

    // リサイズする前のレス番号を保存しておいて
    // redrawした後にジャンプ
    int seen_current = m_seen_current;

    layout();
    redraw_view();

    if( seen_current ) goto_num( seen_current );

    return true;
}



//
// drawarea の再描画イベント
//
bool DrawAreaBase::slot_expose_event( GdkEventExpose* event )
{
#ifdef _DEBUG    
    std::cout << "DrawAreaBase::slot_expose_event\n";
#endif

    draw_drawarea();

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

    m_sig_leave_notify.emit( event );

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

    m_gc = Gdk::GC::create( m_window );
    assert( m_gc );

    layout();
    m_view.grab_focus();
}



//
// マウスボタンを押した
//
bool DrawAreaBase::slot_button_press_event( GdkEventButton* event )
{
    int x, y;
    int pos = get_vscr_val();
    x = ( int ) event->x;
    y = ( int ) event->y + pos;

    CARET_POSITION caret_pos; 
    set_caret( caret_pos, x, y );

    m_view.grab_focus();

    // オートスクロール中ならオートスクロール解除
    if( m_scrollinfo.mode == SCROLL_AUTO ){
            
        m_scrollinfo.reset();
        m_scrollinfo.just_finished = true;
        m_window->set_cursor();
    }
    else {

        // キャレット移動
        if( m_control.button_alloted( event, CONTROL::ClickButton ) ){

            // ctrl+クリックで範囲選択
            if( event->state & GDK_CONTROL_MASK ){

                set_selection( caret_pos );
            }
            else{
                // ドラッグ開始
                m_drugging = true;
                m_selection.select = false;
                m_selection.str.clear();
                m_caret_pos_pre = m_caret_pos;
                m_caret_pos = caret_pos;
                m_caret_pos_dragstart = caret_pos;
            }
        }

        // マウスジェスチャ(右ドラッグ)開始
        else if( m_control.button_alloted( event, CONTROL::GestureButton ) ) m_r_drugging = true;

        // オートスクロールボタン
        else if( m_control.button_alloted( event, CONTROL::AutoScrollButton ) ){

            if ( ! ( m_layout_current && m_layout_current->link ) ){ // リンク上で無いなら                

                m_scrollinfo.reset();
                m_scrollinfo.mode = SCROLL_AUTO;
                m_scrollinfo.show_marker = true;
                m_scrollinfo.enable_up = true;
                m_scrollinfo.enable_down = true;                
                m_scrollinfo.x = ( int ) event->x;
                m_scrollinfo.y = ( int ) event->y;
                m_window->set_cursor( Gdk::Cursor( Gdk::DOUBLE_ARROW ));
            }
        }

        // ダブルクリックしたら範囲選択
        else if( m_control.button_alloted( event, CONTROL::DblClickButton ) ){

            CARET_POSITION caret_left, caret_right;
            if( set_carets_dclick( caret_left, caret_right, x, y ) ) set_selection( caret_left, caret_right );
        }
    }

    // 再描画
    redraw_view();

    m_sig_button_press.emit( event );

    return true;
}


//
// マウスボタンを離した
//
bool DrawAreaBase::slot_button_release_event( GdkEventButton* event )
{
    std::string url;
    int res_num = 0;

    if( m_layout_current && m_layout_current->link ) url = m_layout_current->link;
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
        if( m_scrollinfo.just_finished ){
            m_scrollinfo.just_finished = false;
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
    m_x_pointer = ( int ) event->x;
    m_y_pointer = ( int ) event->y;

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

    // 現在のマウスポインタの下にあるレイアウトノードとキャレットの更新
    m_layout_current = set_caret( m_caret_pos_current, m_x_pointer , m_y_pointer + pos );

    int res_num = 0;
    if( m_layout_current ) res_num = m_layout_current->res_number;

    // 現在のマウスポインタの下にあるリンクの文字列を更新
    // IDや数字だったらポップアップ表示する
    bool link_changed = false;
    std::string link_current;
    if( m_layout_current ){

        // リンクの上にポインタがある
        if( m_layout_current->link  ) link_current =  m_layout_current->link;

        // IDや数字などの範囲選択の上にポインタがある
        else link_current = get_selection_as_url( m_caret_pos_current );
    }

    if( link_current != m_link_current ) link_changed = true; // 前回とリンクの文字列が変わった
    m_link_current = link_current;
    
    // ドラッグ中なら範囲選択
    if( m_drugging ){

        if( set_selection( m_caret_pos_current ) ){

            if( m_scrollinfo.mode == SCROLL_NOT ) draw_drawarea();
        }
    
        // ポインタが画面外に近かったらオートスクロールを開始する
        const int mrg = ( int )( (double)m_br_size*0.5 );

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
    }

    // 右ドラッグしていないとき
    else if( ! m_r_drugging ){
           
        if( link_changed ){

            m_sig_leave_url.emit();

            // (別の)リンクノードに入った
            if( !m_link_current.empty()
                && m_scrollinfo.mode == SCROLL_NOT // スクロール中はon_urlシグナルを出さない
                ){
#ifdef _DEBUG
                std::cout << "slot_motion_notify_drawarea : enter link = " << m_link_current << std::endl;;
#endif
                m_sig_on_url.emit( m_link_current, res_num );
                get_window()->set_cursor( Gdk::Cursor( Gdk::HAND2 ) );
            }

            // リンクノードからでた
            else{
#ifdef _DEBUG
                std::cout << "slot_motion_notify_drawarea : leave\n";
#endif
                get_window()->set_cursor();
            }
        }
    }

    return true;
}


//
// キーを押した
//
bool DrawAreaBase::slot_key_press_event( GdkEventKey* event )
{
#ifdef _DEBUG
    std::cout << "DrawAreaBase::slot_key_press_event\n";
#endif

    //オートスクロール中なら無視
    if( m_scrollinfo.mode == SCROLL_AUTO ) return true;

    m_sig_key_press.emit( event );

    m_key_press = true;
    return true;
}



//
// キーを離した
//
bool DrawAreaBase::slot_key_release_event( GdkEventKey* event )
{
#ifdef _DEBUG
    std::cout << "DrawAreaBase::slot_key_release_event\n";
#endif

    if( !m_key_press ) return true;
    m_key_press = false;

    m_scrollinfo.reset();
    redraw_view();

    m_sig_key_release.emit( event );
    return true;
}


