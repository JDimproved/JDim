// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "skeleton/msgdiag.h"

#include "config/globalconf.h"
#include "config/defaultconf.h"

#include "jdlib/miscgtk.h"

#include "control/controlid.h"
#include "control/controlutil.h"

#include "fontcolorpref.h"
#include "colorid.h"
#include "fontid.h"
#include "command.h"

#define WARNING_STRICTCHAR "スレビューのフォント幅の近似計算を厳密に行います\n\nレイアウトが崩れにくくなるかわりにパフォーマンスが著しく低下します。通常は設定しないでください"

#define WARNING_GTKRC_TREE "gtkrc 関係の設定はJDの再起動後に有効になります"

using namespace CORE;

FontColorPref::FontColorPref( Gtk::Window* parent, const std::string& url )
    : SKELETON::PrefDiag( parent, url, true, true ),
      m_label_aafont( true, "AAレスと判定する正規表現(_R)： " ),
      m_bt_reset_font( "フォントの設定を全てデフォルトに戻す(_F)", true ),

      m_bt_change_color( "選択行の色を設定する(_S)", true ),
      m_bt_reset_color( "選択行の色をデフォルトに戻す(_R)", true ),
      m_bt_reset_all_colors( "色の設定を全てデフォルトに戻す(_C)", true )
{
    CONFIG::bkup_conf();

    pack_widget();

    // フォント設定をセット
    set_font_settings( "スレビュー", FONT_MAIN, "スレビューのフォント" );
    set_font_settings( "ポップアップ", FONT_POPUP, "ポップアップのフォント" );
    set_font_settings( "アスキーアート", FONT_AA, "AA(スレビュー)のフォント" );
    set_font_settings( "板一覧／お気に入り", FONT_BBS, "板一覧／お気に入りのフォント" );
    set_font_settings( "スレ一覧", FONT_BOARD, "スレ一覧のフォント" );
    set_font_settings( "書き込みビュー", FONT_MESSAGE, "書き込みビューのフォント" );

    // 色設定をセット
    set_color_settings( COLOR_NONE, "■ " + CONTROL::get_mode_label( CONTROL::MODE_COMMON ), "" );
    set_color_settings( COLOR_BACK_HIGHLIGHT_TREE, "板、スレ一覧での検索結果などのハイライトの背景色", CONF_COLOR_BACK_HIGHLIGHT_TREE );

    set_color_settings( COLOR_NONE, "", "" );
    set_color_settings( COLOR_NONE, "■ "+ CONTROL::get_mode_label( CONTROL::MODE_BBSLIST ), "" );
    set_color_settings( COLOR_CHAR_BBS, "文字色", CONF_COLOR_CHAR_BBS );
    set_color_settings( COLOR_CHAR_BBS_COMMENT, "コメントの文字色", CONF_COLOR_CHAR_BBS_COMMENT );
    set_color_settings( COLOR_BACK_BBS, "奇数行の背景色", CONF_COLOR_BACK_BBS );
    set_color_settings( COLOR_BACK_BBS_EVEN, "偶数行の背景色", CONF_COLOR_BACK_BBS_EVEN );

    set_color_settings( COLOR_NONE, "", "" );
    set_color_settings( COLOR_NONE, "■ "+ CONTROL::get_mode_label( CONTROL::MODE_BOARD ), "" );
    set_color_settings( COLOR_CHAR_BOARD, "文字色", CONF_COLOR_CHAR_BOARD );
    set_color_settings( COLOR_BACK_BOARD, "奇数行の背景色", CONF_COLOR_BACK_BOARD );
    set_color_settings( COLOR_BACK_BOARD_EVEN, "偶数行の背景色", CONF_COLOR_BACK_BOARD_EVEN );

    set_color_settings( COLOR_NONE, "", "" );
    set_color_settings( COLOR_NONE, "■ "+ CONTROL::get_mode_label( CONTROL::MODE_ARTICLE ), "" );
    set_color_settings( COLOR_CHAR, "基本の文字色", CONF_COLOR_CHAR );
    set_color_settings( COLOR_CHAR_NAME, "名前欄の通常の文字色", CONF_COLOR_CHAR_NAME );
    set_color_settings( COLOR_CHAR_NAME_B, "名前欄のトリップ等の文字色", CONF_COLOR_CHAR_NAME_B );
    set_color_settings( COLOR_CHAR_NAME_NOMAIL, "メール無しの名前欄の文字色", CONF_COLOR_CHAR_NAME_NOMAIL );
    set_color_settings( COLOR_CHAR_AGE, "sage でないメール欄の文字色", CONF_COLOR_CHAR_AGE );
    set_color_settings( COLOR_CHAR_SELECTION, "選択範囲の文字色", CONF_COLOR_CHAR_SELECTION );
    set_color_settings( COLOR_CHAR_HIGHLIGHT, "検索結果などのハイライトの文字色", CONF_COLOR_CHAR_HIGHLIGHT );
    set_color_settings( COLOR_CHAR_LINK, "通常のリンクの文字色", CONF_COLOR_CHAR_LINK );
    set_color_settings( COLOR_CHAR_LINK_ID_LOW, "複数発言したIDの文字色", CONF_COLOR_CHAR_LINK_ID_LOW );
    set_color_settings( COLOR_CHAR_LINK_ID_HIGH, "多く発言したIDの文字色", CONF_COLOR_CHAR_LINK_ID_HIGH );
    set_color_settings( COLOR_CHAR_LINK_RES, "参照されていないレス番号の文字色", CONF_COLOR_CHAR_LINK_RES );
    set_color_settings( COLOR_CHAR_LINK_LOW, "他のレスから参照されたレス番号の文字色", CONF_COLOR_CHAR_LINK_LOW );
    set_color_settings( COLOR_CHAR_LINK_HIGH, "参照された数が多いレス番号の文字色", CONF_COLOR_CHAR_LINK_HIGH );
    set_color_settings( COLOR_IMG_NOCACHE, "画像として扱うリンクのうち、キャッシュされていない物の文字色", CONF_COLOR_IMG_NOCACHE );
    set_color_settings( COLOR_IMG_CACHED, "画像として扱うリンクのうち、キャッシュされている物の文字色", CONF_COLOR_IMG_CACHED );
    set_color_settings( COLOR_IMG_LOADING, "画像として扱うリンクのうち、ロード中の物の文字色", CONF_COLOR_IMG_LOADING );
    set_color_settings( COLOR_IMG_ERR, "画像として扱うリンクのうち、エラーになっている物の文字色", CONF_COLOR_IMG_ERR );
    set_color_settings( COLOR_BACK, "スレビューの背景色", CONF_COLOR_BACK );
    set_color_settings( COLOR_BACK_POPUP, "ポップアップの背景色", CONF_COLOR_BACK_POPUP );
    set_color_settings( COLOR_BACK_SELECTION, "選択範囲の背景色", CONF_COLOR_BACK_SELECTION );
    set_color_settings( COLOR_BACK_HIGHLIGHT, "検索結果などのハイライトの背景色", CONF_COLOR_BACK_HIGHLIGHT );
    set_color_settings( COLOR_SEPARATOR_NEW, "新着しおりの色", CONF_COLOR_SEPARATOR_NEW );
    set_color_settings( COLOR_FRAME, "ポップアップのフレーム色", CONF_COLOR_FRAME );
    set_color_settings( COLOR_MARKER, "オートスクロールのマーカ色", CONF_COLOR_MARKER );

    set_color_settings( COLOR_NONE, "", "" );
    set_color_settings( COLOR_NONE, "■ " + CONTROL::get_mode_label( CONTROL::MODE_MESSAGE ), "" );
    set_color_settings( COLOR_CHAR_MESSAGE, "文字色", CONF_COLOR_CHAR_MESSAGE );
    set_color_settings( COLOR_CHAR_MESSAGE_SELECTION, "選択範囲の文字色", CONF_COLOR_CHAR_MESSAGE_SELECTION );
    set_color_settings( COLOR_BACK_MESSAGE, "背景色", CONF_COLOR_BACK_MESSAGE );
    set_color_settings( COLOR_BACK_MESSAGE_SELECTION, "選択範囲の背景色", CONF_COLOR_BACK_MESSAGE_SELECTION );

    m_combo_font.set_active( 0 );
    m_fontbutton.set_font_name( CONFIG::get_fontname( m_font_tbl[ 0 ] ) );
    m_tooltips.set_tip( m_event_font, m_tooltips_font[ 0 ] );
    m_tooltips.set_tip( m_fontbutton, m_tooltips_font[ 0 ] );

    set_title( "フォントと色の詳細設定" );
    show_all_children();
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

    m_hbox_font.pack_start( m_event_font, Gtk::PACK_SHRINK );
    m_hbox_font.pack_start( m_fontbutton, Gtk::PACK_EXPAND_WIDGET, mrg );

    m_vbox_font.set_border_width( mrg );
    m_vbox_font.pack_start( m_hbox_font, Gtk::PACK_SHRINK, mrg/2 );

    m_checkbutton_font.add_label( "スレビューでフォント幅の近似計算を厳密に行う(_S)", true ),
    m_checkbutton_font.set_active( CONFIG::get_strict_char_width() );
    m_tooltips.set_tip( m_checkbutton_font, WARNING_STRICTCHAR );
    m_hbox_checkbutton.pack_start( m_checkbutton_font, Gtk::PACK_SHRINK );
    m_vbox_font.pack_start( m_hbox_checkbutton, Gtk::PACK_SHRINK, mrg/2 );

    // 行高さ
    m_spin_space.set_digits( 1 );
    m_spin_space.set_range( 0.1, 10.0 );
    m_spin_space.set_increments( 0.1, 0.1 );
    m_spin_space.set_value( CONFIG::get_adjust_line_space() );
    m_label_space.set_text_with_mnemonic( "スレビューの文字列の行の高さ(_H)： " );
    m_label_space.set_mnemonic_widget( m_spin_space );

    m_hbox_space.set_spacing ( mrg );
    m_hbox_space.pack_start( m_label_space, Gtk::PACK_SHRINK );
    m_hbox_space.pack_start( m_spin_space, Gtk::PACK_SHRINK );
    m_tooltips.set_tip( m_spin_space, "スレビューにおいて行の高さを調節します( 標準は 1 )" );
    m_vbox_font.pack_start( m_hbox_space, Gtk::PACK_SHRINK, mrg/2 );

    set_activate_entry( m_spin_space );

    // 下線位置
    m_spin_ubar.set_digits( 1 );
    m_spin_ubar.set_range( 0.1, 10.0 );
    m_spin_ubar.set_increments( 0.1, 0.1 );
    m_spin_ubar.set_value( CONFIG::get_adjust_underline_pos() );
    m_label_ubar.set_text_with_mnemonic( "スレビューの文字列の下線位置(_U)： " );
    m_label_ubar.set_mnemonic_widget( m_spin_ubar );

    m_hbox_ubar.pack_start( m_label_ubar, Gtk::PACK_SHRINK );
    m_hbox_ubar.pack_start( m_spin_ubar, Gtk::PACK_SHRINK );
    m_vbox_font.pack_start( m_hbox_ubar, Gtk::PACK_SHRINK, mrg/2 );
    m_tooltips.set_tip( m_spin_ubar, "スレビューにおいてアンカーなどの下線の位置を調節します( 標準は 1 )" );

    set_activate_entry( m_spin_ubar );

    // AAレスと判定する正規表現
    m_label_aafont.set_text( CONFIG::get_regex_res_aa() );
    m_vbox_font.pack_start( m_label_aafont, Gtk::PACK_SHRINK, mrg/2 );
    m_tooltips.set_tip( m_label_aafont, "この正規表現に一致したレスは、アスキーアートフォントで表示します( 次に開いたスレから有効 )" );

    set_activate_entry( m_label_aafont );

    // フォントのリセット
    m_bt_reset_font.signal_clicked().connect( sigc::mem_fun( *this, &FontColorPref::slot_reset_font ) );
    m_vbox_font.pack_end( m_bt_reset_font, Gtk::PACK_SHRINK );

    m_notebook.append_page( m_vbox_font, "フォントの設定" );    

    m_combo_font.signal_changed().connect( sigc::mem_fun( *this, &FontColorPref::slot_combo_font_changed ) );
    m_fontbutton.signal_font_set().connect( sigc::mem_fun( *this, &FontColorPref::slot_fontbutton_on_set ) );
    m_checkbutton_font.signal_toggled().connect( sigc::mem_fun( *this, &FontColorPref::slot_checkbutton_font_toggled ) );


    // 色
    m_vbox_color.set_border_width( mrg );
    m_vbox_color.set_spacing( mrg );

    m_label_warning_color.set_text( "Ctrl+クリック又はShift+クリックで複数行選択可能\nテーマによってはツリービュー(板一覧、スレ一覧)の背景色が正しく設定されない場合があります。" );
    m_vbox_color.pack_start( m_label_warning_color, Gtk::PACK_SHRINK );

    m_liststore_color = Gtk::ListStore::create( m_columns_color );
    m_treeview_color.set_model( m_liststore_color );
    m_treeview_color.set_size_request( 480, 280 );
    m_treeview_color.get_selection()->set_mode( Gtk::SELECTION_MULTIPLE );
    m_treeview_color.signal_row_activated().connect( sigc::mem_fun( *this, &FontColorPref::slot_row_activated ) );
    m_scrollwin_color.add( m_treeview_color );
    m_scrollwin_color.set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS );
    m_vbox_color.pack_start( m_scrollwin_color, Gtk::PACK_EXPAND_WIDGET );

    Gtk::TreeViewColumn* column = Gtk::manage( new Gtk::TreeViewColumn( "設定名", m_columns_color.m_col_name ) );
    column->set_fixed_width( 430 );
    column->set_sizing( Gtk::TREE_VIEW_COLUMN_FIXED );
    column->set_resizable( true );
    m_treeview_color.append_column( *column );
    Gtk::CellRenderer *cell = column->get_first_cell_renderer();
    if( cell ) column->set_cell_data_func( *cell, sigc::mem_fun( *this, &FontColorPref::slot_cell_data_name ) );

    column = Gtk::manage( new Gtk::TreeViewColumn( "色", m_columns_color.m_col_color ) );
    m_treeview_color.append_column( *column );
    cell = column->get_first_cell_renderer();
    if( cell ) column->set_cell_data_func( *cell, sigc::mem_fun( *this, &FontColorPref::slot_cell_data_color ) );

    m_bt_change_color.signal_clicked().connect( sigc::mem_fun( *this, &FontColorPref::slot_change_color ) );
    m_bt_reset_color.signal_clicked().connect( sigc::mem_fun( *this, &FontColorPref::slot_reset_color ) );

    m_hbox_change_color.set_spacing( mrg );
    m_hbox_change_color.pack_end( m_bt_reset_color, Gtk::PACK_SHRINK );
    m_hbox_change_color.pack_end( m_bt_change_color , Gtk::PACK_SHRINK );
    m_vbox_color.pack_start( m_hbox_change_color, Gtk::PACK_SHRINK );

    m_chk_use_gtkrc_tree.add_label( "ツリービューの背景色設定に gtkrc を用いる(_T)", true ),
    m_chk_use_gtkrc_tree.set_active( CONFIG::get_use_tree_gtkrc() );
    m_chk_use_gtkrc_tree.signal_toggled().connect( sigc::mem_fun( *this, &FontColorPref::slot_chk_use_gtkrc_toggled ) );
    m_vbox_color.pack_start( m_chk_use_gtkrc_tree, Gtk::PACK_SHRINK );

    m_chk_use_gtkrc_selection.add_label( "スレビューの選択範囲の色設定に gtkrc を用いる(_S)", true ),
    m_chk_use_gtkrc_selection.set_active( CONFIG::get_use_select_gtkrc() );
    m_vbox_color.pack_start( m_chk_use_gtkrc_selection, Gtk::PACK_SHRINK );

    m_bt_reset_all_colors.signal_clicked().connect( sigc::mem_fun( *this, &FontColorPref::slot_reset_all_colors ) );
    m_vbox_color.pack_end( m_bt_reset_all_colors, Gtk::PACK_SHRINK );

    m_notebook.append_page( m_vbox_color, "色の設定" );    

    // 全体
    get_vbox()->pack_start( m_notebook );
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

    CONFIG::set_adjust_line_space( m_spin_space.get_value() );
    CONFIG::set_adjust_underline_pos( m_spin_ubar.get_value() );
    CONFIG::set_regex_res_aa( m_label_aafont.get_text() );

    CONFIG::set_strict_char_width( m_checkbutton_font.property_active() );

    CONFIG::set_use_tree_gtkrc( m_chk_use_gtkrc_tree.property_active() );
    CONFIG::set_use_select_gtkrc( m_chk_use_gtkrc_selection.property_active() );

    CORE::core_set_command( "relayout_all_bbslist" );
    CORE::core_set_command( "relayout_all_board" );

    CORE::core_set_command( "init_font_all_article" );
    CORE::core_set_command( "relayout_all_article" );

    CORE::core_set_command( "relayout_all_message" );
}


void FontColorPref::slot_apply_clicked()
{
#ifdef _DEBUG
    std::cout << "FontColorPref::slot_apply_clicked\n";
#endif

    slot_ok_clicked();
    CONFIG::bkup_conf();
}


//
// 設定ダイアログでキャンセルが押された
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
        m_combo_font.append( name );
        m_font_tbl.push_back( fontid );
        m_tooltips_font.push_back( tooltip );
    }
}


//
// フォント設定のコンボボックスの状態が変わった
//
void FontColorPref::slot_combo_font_changed()
{
    const int num = m_combo_font.get_active_row_number();
    m_fontbutton.set_font_name( CONFIG::get_fontname( m_font_tbl[ num ] ) );
    m_tooltips.set_tip( m_event_font, m_tooltips_font[ num ] );
    m_tooltips.set_tip( m_fontbutton, m_tooltips_font[ num ] );
}


//
// チェックボックスの状態が変わった
//
void FontColorPref::slot_checkbutton_font_toggled()
{
    if( m_checkbutton_font.property_active() )
    {
        SKELETON::MsgDiag mdiag( NULL, WARNING_STRICTCHAR );
        mdiag.run();
    }
}

void FontColorPref::slot_chk_use_gtkrc_toggled()
{
    if( m_chk_use_gtkrc_tree.property_active() )
    {
        SKELETON::MsgDiag mdiag( NULL, WARNING_GTKRC_TREE );
        mdiag.run();
    }
}


//
// フォント選択ダイアログで OK が押された
//
void FontColorPref::slot_fontbutton_on_set()
{
    const int num = m_combo_font.get_active_row_number();
    const std::string result = m_fontbutton.get_font_name();

    CONFIG::set_fontname( m_font_tbl[ num ], result );
}


//
// フォント設定のリセット
//
void FontColorPref::slot_reset_font()
{
    CONFIG::reset_fonts();
    const int num = m_combo_font.get_active_row_number();
    m_fontbutton.set_font_name( CONFIG::get_fontname( m_font_tbl[ num ] ) );

    m_checkbutton_font.set_active( CONFIG::CONF_STRICT_CHAR_WIDTH );

    m_spin_space.set_value( CONFIG::CONF_ADJUST_LINE_SPACE );
    m_spin_ubar.set_value( CONFIG::CONF_ADJUST_UNDERLINE_POS );
    m_label_aafont.set_text( CONF_REGEX_RES_AA_DEFAULT );
}




//
// 色設定の名前と設定値をセット
//
void FontColorPref::set_color_settings( const int colorid, const std::string& name, const std::string& defaultval )
{
#ifdef _DEBUG
    std::cout << " FontColorPref::set_color_settings name = " << name << " id = " << colorid << std::endl;
#endif

    Gtk::TreeModel::Row row;
    row = *( m_liststore_color->append() );

    row[ m_columns_color.m_col_name ]  = name;
    row[ m_columns_color.m_col_color ] = std::string();
    row[ m_columns_color.m_col_colorid ] = colorid;
    row[ m_columns_color.m_col_default ] = defaultval;
}


//
// 行選択
//
void FontColorPref::slot_row_activated( const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column )
{
#ifdef _DEBUG
    std::cout << "FontColorPref::slot_row_activated path = " << path.to_string() << std::endl;
#endif

    slot_change_color();
}

//
// 実際の描画の際に cellrendere のプロパティをセットするスロット関数
//
void FontColorPref::slot_cell_data_name( Gtk::CellRenderer* cell, const Gtk::TreeModel::iterator& it )
{
    Gtk::TreeModel::Row row = *it;

    const int colorid = row[ m_columns_color.m_col_colorid ];
    const std::string defaultcolor = row[ m_columns_color.m_col_default ];
    if( colorid != COLOR_NONE && CONFIG::get_color( colorid ) != defaultcolor ){
        cell->property_cell_background() = CONFIG::get_color( COLOR_BACK_HIGHLIGHT_TREE );
        cell->property_cell_background_set() = true;
    }
    else cell->property_cell_background_set() = false;
}


void FontColorPref::slot_cell_data_color( Gtk::CellRenderer* cell, const Gtk::TreeModel::iterator& it )
{
    Gtk::TreeModel::Row row = *it;

    const int colorid = row[ m_columns_color.m_col_colorid ];
    if( colorid != COLOR_NONE ){
        cell->property_cell_background() = CONFIG::get_color( colorid );
        cell->property_cell_background_set() = true;
    }
    else cell->property_cell_background_set() = false;
}



//
// 選択行の色の変更
//
void FontColorPref::slot_change_color()
{
    Gtk::TreeModel::Path path;
    Gtk::TreeRow row;
    
    std::list< Gtk::TreePath > selection_path = m_treeview_color.get_selection()->get_selected_rows();
    if( selection_path.empty() ) return;

    std::list< Gtk::TreePath >::iterator it = selection_path.begin();

    int colorid = COLOR_NONE;
    if( selection_path.size() == 1 ){
        row = *m_liststore_color->get_iter( *it );
        if( ! row ) return;
        colorid = row[ m_columns_color.m_col_colorid ];
        if( colorid == COLOR_NONE ) return;
    }

    Gtk::ColorSelectionDialog colordiag;
    if( colorid != COLOR_NONE ) colordiag.get_colorsel()->set_current_color( Gdk::Color( CONFIG::get_color( colorid ) ) );
    colordiag.set_transient_for( *CORE::get_mainwindow() );
    const int ret = colordiag.run();

    if( ret == Gtk::RESPONSE_OK ){

        for( ; it != selection_path.end(); ++it ){

            row = *m_liststore_color->get_iter( *it );
            if( ! row ) continue;

            colorid = row[ m_columns_color.m_col_colorid ];
            if( colorid != COLOR_NONE )
                CONFIG::set_color( colorid , MISC::color_to_str( colordiag.get_colorsel()->get_current_color() ) );
        }
    }
}


//
// 選択行の色のリセット
//
void FontColorPref::slot_reset_color()
{
    std::list< Gtk::TreePath > selection_path = m_treeview_color.get_selection()->get_selected_rows();
    if( selection_path.empty() ) return;

    std::list< Gtk::TreePath >::iterator it = selection_path.begin();

    for( ; it != selection_path.end(); ++it ){

        Gtk::TreeRow row = *m_liststore_color->get_iter( *it );
        if( ! row ) continue;

        const int colorid = row[ m_columns_color.m_col_colorid ];
        if( colorid != COLOR_NONE ){

            const std::string defaultcolor = row[ m_columns_color.m_col_default ];
            CONFIG::set_color( colorid , defaultcolor );
            m_treeview_color.queue_draw();
        }
    }
}


//
// 全ての色のリセット
//
void FontColorPref::slot_reset_all_colors()
{
    if( m_chk_use_gtkrc_tree.property_active() )
    {
        SKELETON::MsgDiag mdiag( NULL, WARNING_GTKRC_TREE );
        mdiag.run();
    }
    m_chk_use_gtkrc_tree.set_active( CONFIG::CONF_USE_TREE_GTKRC );
    m_chk_use_gtkrc_selection.set_active( CONFIG::CONF_USE_SELECT_GTKRC );

    CONFIG::reset_colors();

    m_treeview_color.queue_draw();
}
