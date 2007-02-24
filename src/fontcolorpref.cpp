// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "skeleton/msgdiag.h"

#include "config/globalconf.h"

#include "jdlib/miscgtk.h"

#include "fontcolorpref.h"
#include "colorid.h"
#include "fontid.h"

#define WARNING_STRICTCHAR "スレビューのフォント幅の近似計算を厳密に行います\n\nレイアウトが崩れにくくなるかわりにパフォーマンスが著しく低下します。通常は設定しないでください"

using namespace CORE;

FontColorPref::FontColorPref( const std::string& url )
    : SKELETON::PrefDiag( url )
{
    CONFIG::bkup_conf();

    // フォント設定をセット
    set_font_settings( "スレビュー", FONT_MAIN, "スレビューのフォント" );
    set_font_settings( "ポップアップ", FONT_POPUP, "ポップアップのフォント" );
    set_font_settings( "板一覧", FONT_BBS, "板一覧のフォント" );
    set_font_settings( "スレ一覧", FONT_BOARD, "スレ一覧のフォント" );
    set_font_settings( "書き込みエディタ", FONT_MESSAGE, "書き込みエディタのフォント" );

    // 色設定をセット
    set_color_settings( "文字: スレビュー", COLOR_CHAR, "スレビューの基本の文字色" );
    set_color_settings( "文字: 名前欄", COLOR_CHAR_NAME, "名前欄の通常の文字色" );
    set_color_settings( "文字: 名前欄(トリップ等)", COLOR_CHAR_NAME_B, "名前欄のトリップ等の文字色" );
    set_color_settings( "文字: メール欄(非sage)", COLOR_CHAR_AGE, "sage でないメール欄の文字色" );
    set_color_settings( "文字: 選択範囲", COLOR_CHAR_SELECTION, "選択範囲の文字色" );
    set_color_settings( "文字: ハイライト", COLOR_CHAR_HIGHLIGHT, "検索結果などのハイライトの文字色" );
    set_color_settings( "文字: ブックマーク", COLOR_CHAR_BOOKMARK, "レスのブックマークの文字色" );
    set_color_settings( "文字: リンク(通常)", COLOR_CHAR_LINK, "通常のリンクや参照されていないレス番号、複数発言したIDの文字色" );
    set_color_settings( "文字: リンク(参照)", COLOR_CHAR_LINK_LOW, "他のレスから参照されたレス番号の文字色" );
    set_color_settings( "文字: リンク(多数)", COLOR_CHAR_LINK_HIGH, "参照された数が多いレス番号や多く発言したIDの文字色" );
    set_color_settings( "文字: 画像(キャッシュ無)", COLOR_IMG_NOCACHE, "画像として扱うリンクのうち、キャッシュされていない物の文字色" );
    set_color_settings( "文字: 画像(キャッシュ有)", COLOR_IMG_CACHED, "画像として扱うリンクのうち、キャッシュされている物の文字色" );
    set_color_settings( "文字: 画像(ロード中)", COLOR_IMG_LOADING, "画像として扱うリンクのうち、ロード中の物の文字色" );
    set_color_settings( "文字: 画像(エラー)", COLOR_IMG_ERR, "画像として扱うリンクのうち、エラーになっている物の文字色" );
    set_color_settings( "特殊: 新着しおり", COLOR_SEPARATOR_NEW, "新着しおりの色" );
    set_color_settings( "背景: スレビュー", COLOR_BACK, "スレビューの背景色" );
    set_color_settings( "背景: ポップアップ", COLOR_BACK_POPUP, "ポップアップの背景色" );
    set_color_settings( "背景: 選択範囲", COLOR_BACK_SELECTION, "選択範囲の背景色" );
    set_color_settings( "背景: ハイライト", COLOR_BACK_HIGHLIGHT, "検索結果などのハイライトの背景色" );
    set_color_settings( "背景: 板一覧", COLOR_BACK_BBS, "板一覧の背景色" );
    set_color_settings( "背景: スレ一覧", COLOR_BACK_BOARD, "スレ一覧の背景色" );


    pack_widget();

    set_title( "フォントと色の詳細設定" );
    show_all_children();
    resize( 480, 240 );
}


FontColorPref::~FontColorPref()
{}


//
// 各ウィジェットを追加
//
void FontColorPref::pack_widget()
{
    const int mrg = 8;

    // フォント
    m_event_font.add( m_combo_font );
    m_combo_font.set_active( 0 );
    m_fontbutton.set_font_name( CONFIG::get_fontname( m_font_tbl[ 0 ] ) );
    m_tooltips.set_tip( m_event_font, m_tooltips_font[ 0 ] );
    m_tooltips.set_tip( m_fontbutton, m_tooltips_font[ 0 ] );

    m_hbox_font.set_spacing( mrg );
    m_hbox_font.set_border_width( mrg );
    m_hbox_font.pack_start( m_event_font, Gtk::PACK_SHRINK );
    m_hbox_font.pack_start( m_fontbutton, Gtk::PACK_EXPAND_WIDGET, mrg );

    m_vbox_font.pack_start( m_hbox_font, Gtk::PACK_SHRINK );

    m_checkbutton_font.set_label( "フォント幅の近似計算を厳密に行う");
    m_checkbutton_font.set_active( CONFIG::get_strict_char_width() );
    m_tooltips.set_tip( m_checkbutton_font, WARNING_STRICTCHAR );

    m_vbox_font.pack_start( m_checkbutton_font, Gtk::PACK_SHRINK, mrg );

    m_frame_font.set_label( "フォントの設定" );
    m_frame_font.add( m_vbox_font );

    get_vbox()->pack_start( m_frame_font, Gtk::PACK_SHRINK );

    m_combo_font.signal_changed().connect( sigc::mem_fun( *this, &FontColorPref::slot_combo_font_changed ) );
    m_fontbutton.signal_font_set().connect( sigc::mem_fun( *this, &FontColorPref::slot_fontbutton_on_set ) );
    m_checkbutton_font.signal_toggled().connect( sigc::mem_fun( *this, &FontColorPref::slot_checkbutton_font_toggled ) );


    // 色
    m_event_color.add( m_combo_color );
    m_combo_color.set_active( 0 );
    m_colorbutton.set_color( Gdk::Color( CONFIG::get_color( m_color_tbl[ 0 ] ) ) );
    m_tooltips.set_tip( m_event_color, m_tooltips_color[ 0 ] );
    m_tooltips.set_tip( m_colorbutton, m_tooltips_color[ 0 ] );

    m_hbox_color.set_spacing( mrg );
    m_hbox_color.set_border_width( mrg );
    m_hbox_color.pack_start( m_event_color, Gtk::PACK_SHRINK );
    m_hbox_color.pack_start( m_colorbutton, Gtk::PACK_SHRINK, mrg );

    m_vbox_color.pack_start( m_hbox_color, Gtk::PACK_EXPAND_WIDGET, mrg );

    m_frame_color.set_label( "色の設定" );
    m_frame_color.add( m_vbox_color );

    get_vbox()->pack_start( m_frame_color, Gtk::PACK_SHRINK );

    m_combo_color.signal_changed().connect( sigc::mem_fun( *this, &FontColorPref::slot_combo_color_changed ) );
    m_colorbutton.signal_color_set().connect( sigc::mem_fun( *this, &FontColorPref::slot_colorbutton_on_set ) );

    // 全体
    get_vbox()->set_spacing( mrg );
    set_border_width( mrg );
}


//
// 設定ダイアログで OK が押された
//
void FontColorPref::slot_ok_clicked()
{
#ifdef _DEBUG
    std::cout << "FontColorPref::slot_ok_clicked\n";
#endif

    CORE::core_set_command( "relayout_all_bbslist" );
    CORE::core_set_command( "relayout_all_board" );

    CORE::core_set_command( "init_font_all_article" );
    CORE::core_set_command( "relayout_all_article" );
}


//
// 設定ダイアログで OK が押された
//
void FontColorPref::slot_cancel_clicked()
{
#ifdef _DEBUG
    std::cout << "FontColorPref::slot_cancel_clicked\n";
#endif

    CONFIG::restore_conf();
}


//
// フォント設定の名前と設定値をセット
//
void FontColorPref::set_font_settings( const std::string& name, const int fontid, const std::string& tooltip )
{
    if( ! name.empty() && fontid < FONT_NUM )
    {
        m_combo_font.append_text( name );
        m_font_tbl.push_back( fontid );
        m_tooltips_font.push_back( tooltip );
    }
}


//
// 色設定の名前と設定値をセット
//
void FontColorPref::set_color_settings( const std::string& name, const int colorid, const std::string& tooltip )
{
    if( ! name.empty() && colorid < COLOR_NUM )
    {
#ifdef _DEBUG
        std::cout << " FontColorPref::set_color_settings name = " << name << " id = " << colorid << std::endl;
#endif
        m_combo_color.append_text( name );
        m_color_tbl.push_back( colorid );
        m_tooltips_color.push_back( tooltip );
    }
}


//
// フォント設定のコンボボックスの状態が変わった
//
void FontColorPref::slot_combo_font_changed()
{
    int num = m_combo_font.get_active_row_number();
    m_fontbutton.set_font_name( CONFIG::get_fontname( m_font_tbl[ num ] ) );
    m_tooltips.set_tip( m_event_font, m_tooltips_font[ num ] );
    m_tooltips.set_tip( m_fontbutton, m_tooltips_font[ num ] );
}


//
// 色設定のコンボボックスの状態が変わった
//
void FontColorPref::slot_combo_color_changed()
{
    int num = m_combo_color.get_active_row_number();

    m_colorbutton.set_color( Gdk::Color( CONFIG::get_color( m_color_tbl[ num ] ) ) );
    m_tooltips.set_tip( m_event_color, m_tooltips_color[ num ] );
    m_tooltips.set_tip( m_colorbutton, m_tooltips_color[ num ] );
}


//
// チェックボックスの状態が変わった
//
void FontColorPref::slot_checkbutton_font_toggled()
{
    bool result = m_checkbutton_font.property_active();

    if( result )
    {
        SKELETON::MsgDiag mdiag( NULL, WARNING_STRICTCHAR );
        mdiag.run();
    }

    CONFIG::set_strict_char_width( result );
}


//
// フォント選択ダイアログで OK が押された
//
void FontColorPref::slot_fontbutton_on_set()
{
    int num = m_combo_font.get_active_row_number();
    std::string result = m_fontbutton.get_font_name();

    CONFIG::set_fontname( m_font_tbl[ num ], result );
}


//
// 色選択ダイアログで OK が押された
//
void FontColorPref::slot_colorbutton_on_set()
{
    int num = m_combo_color.get_active_row_number();
    std::string result = MISC::color_to_str( m_colorbutton.get_color() );

    CONFIG::set_color( m_color_tbl[ num ] , result );
}

