// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "skeleton/msgdiag.h"

#include "config/globalconf.h"
#include "config/defaultconf.h"

#include "jdlib/miscgtk.h"

#include "control/controlid.h"
#include "control/controlutil.h"

#include "icons/iconmanager.h"

#include "fontcolorpref.h"
#include "colorid.h"
#include "fontid.h"
#include "command.h"

#define WARNING_STRICTCHAR "スレビューのフォント幅の近似計算を厳密に行います\n\nレイアウトが崩れにくくなるかわりにパフォーマンスが著しく低下します。通常は設定しないでください"

#define WARNING_GTKRC_TREE "gtkrc 関係の設定はJDimの再起動後に有効になります"

using namespace CORE;

FontColorPref::FontColorPref( Gtk::Window* parent, const std::string& url )
    : SKELETON::PrefDiag( parent, url, true, true )
    , m_hbox_font{ Gtk::ORIENTATION_HORIZONTAL, 8 }
    , m_label_aafont{ "AAレスと判定する正規表現(_R):", true }
    , m_bt_reset_font{ "フォントの設定を全てデフォルトに戻す(_F)", true }

    , m_label_reset_color{ "選択行の色をデフォルトに戻す:", false }
    , m_bt_change_color{ "選択行の色を設定する(_S)", true }
    , m_bt_reset_color{ "ライト(_R)", true }
    , m_bt_reset_color_dark{ "ダーク(_A)", true }
    , m_hbox_reset_all_colors{ Gtk::ORIENTATION_HORIZONTAL, 8 }
    , m_label_reset_all_colors{ "色の設定を全てデフォルトに戻す:", false }
    , m_bt_reset_all_colors{ "ライトテーマ(_D)", true }
    , m_bt_reset_all_colors_dark{ "ダークテーマ(_K)", true }

    , m_label_gtk_theme{ "GTKテーマ(_T):", true }
    , m_check_system_theme{ "システム設定のGTKテーマを使う(_S)（再起動後に有効になります）", true }
    , m_label_dark_theme{ "ダークテーマ:", false }
    , m_check_dark_theme{ "ダークテーマで表示する(_D)", true }

    , m_label_icon_theme{ "アイコンテーマ(_I):", true }
    , m_check_system_icon{ "システム設定のアイコンテーマを使う(_Y)（再起動後に有効になります）", true }
    , m_label_use_symbolic_icon{ "スタイル:", false }
    , m_check_use_symbolic_icon{ "シンボリックアイコンで表示する(_M)", true }

    , m_label_note{ "　「GTKテーマ」や「アイコンテーマ」をシステム設定に変更するときは、"
                    "アプリケーションの再起動が必要です。\n\n"
                    "　環境変数 GTK_THEME を設定して起動した場合、アプリケーション側の"
                    "GTKテーマ設定は上書きされるため、テーマの変更はできません。\n\n"
                    "　一部のGTKテーマはダークテーマに対応していないため、"
                    "「ダークテーマで表示する」の効果がない場合があります。"
                    "その場合は、ダークテーマに対応したGTKテーマを設定してください。\n\n"
                    "　アイコンテーマによっては、カラーアイコンかシンボリックアイコンの"
                    "どちらかにしか対応していないため、「シンボリックアイコンで表示する」"
                    "設定を切り替えても、アイコンが変わらない場合があります。\n\n"
                    "　書き込みビュー、ツリービュー、スレビューの選択範囲をGTKテーマの配色にするには"
                    "「色の設定」タブで設定を切り替えます。"
                    , false }
{
    CONFIG::bkup_conf();

    pack_widget();

    // フォント設定をセット
    set_font_settings( "スレビュー", FONT_MAIN, "スレビューのフォント" );
    set_font_settings( "メール欄とか", FONT_MAIL, "メール欄とかのフォント" );
    set_font_settings( "ポップアップ", FONT_POPUP, "ポップアップのフォント" );
    set_font_settings( "アスキーアート", FONT_AA, "AA(スレビュー)のフォント" );
    set_font_settings( "板一覧／お気に入り", FONT_BBS, "板一覧／お気に入りのフォント" );
    set_font_settings( "スレ一覧", FONT_BOARD, "スレ一覧のフォント" );
    set_font_settings( "書き込みビュー", FONT_MESSAGE, "書き込みビューのフォント" );

    // 色設定をセット
    set_color_settings( COLOR_NONE, "■ " + CONTROL::get_mode_label( CONTROL::MODE_COMMON ), "" );
    set_color_settings( COLOR_CHAR_HIGHLIGHT_TREE, "板、スレ一覧での検索結果などのハイライトの文字色", CONF_COLOR_CHAR_HIGHLIGHT_TREE );
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
    m_event_font.set_tooltip_text( m_tooltips_font[ 0 ] );
    m_fontbutton.set_tooltip_text( m_tooltips_font[ 0 ] );

    // GTKテーマの追加
    const auto gtk_theme_names = ICON::get_installed_gtk_theme_names();
    for( const std::string& name : gtk_theme_names ) {
        m_combo_theme.append( name, name );
    }

    // アイコンテーマの追加
    const auto icon_names = ICON::get_installed_icon_theme_names();
    for( const std::string& name : icon_names ) {
        m_combo_icon.append( name, name );
    }

    m_check_system_icon.set_active( CONFIG::get_gtk_icon_theme_name().empty() );
    m_check_use_symbolic_icon.set_active( CONFIG::get_use_symbolic_icon() );

    if( auto env_theme = Glib::getenv( "GTK_THEME" ); ! env_theme.empty() ) {
        m_combo_theme.set_tooltip_text( "環境変数 GTK_THEME が設定されているため変更できません。" );
        m_check_system_theme.set_tooltip_text( "環境変数 GTK_THEME が設定されているため変更できません。" );
        m_check_dark_theme.set_tooltip_text( "環境変数 GTK_THEME が設定されているため変更できません。" );

        m_check_system_theme.set_active( false );

        m_combo_theme.set_sensitive( false );
        m_check_system_theme.set_sensitive( false );
        m_check_dark_theme.set_sensitive( false );

        if( const auto sep = env_theme.find( ':' );  sep != std::string::npos ) {
            m_check_dark_theme.set_active( env_theme.compare( sep, 5, ":dark" ) == 0 );
            env_theme.resize( sep );
        }
        else {
            m_check_dark_theme.set_active( false );
        }
        m_combo_theme.set_active_text( env_theme );
    }
    else {
        m_check_system_theme.set_active( CONFIG::get_gtk_theme_name().empty() );
        m_check_dark_theme.set_active( CONFIG::get_use_dark_theme() );

        // ComboBoxText に内容を追加した後でバインドしないとデスクトップのシステム設定と同期しない
        m_binding_theme = Glib::Binding::bind_property( Gtk::Settings::get_default()->property_gtk_theme_name(),
                                                        m_combo_theme.property_active_id(),
                                                        Glib::BINDING_BIDIRECTIONAL | Glib::BINDING_SYNC_CREATE );
        m_binding_system = Glib::Binding::bind_property( m_check_system_theme.property_active(),
                                                         m_combo_theme.property_sensitive(),
                                                         Glib::BINDING_INVERT_BOOLEAN | Glib::BINDING_SYNC_CREATE );
        m_binding_dark = Glib::Binding::bind_property(
            Gtk::Settings::get_default()->property_gtk_application_prefer_dark_theme(),
            m_check_dark_theme.property_active(), Glib::BINDING_BIDIRECTIONAL | Glib::BINDING_SYNC_CREATE );
    }
    m_binding_icon = Glib::Binding::bind_property( Gtk::Settings::get_default()->property_gtk_icon_theme_name(),
                                                   m_combo_icon.property_active_id(),
                                                   Glib::BINDING_BIDIRECTIONAL | Glib::BINDING_SYNC_CREATE );
    m_binding_system_icon = Glib::Binding::bind_property( m_check_system_icon.property_active(),
                                                          m_combo_icon.property_sensitive(),
                                                          Glib::BINDING_INVERT_BOOLEAN | Glib::BINDING_SYNC_CREATE );

    set_title( "フォントと色の詳細設定" );
    // ウインドウの自然なサイズを設定するがディスプレイに合わせて調整される
    set_default_size( 670, 590 );
    show_all_children();
}


FontColorPref::~FontColorPref() noexcept = default;


//
// 各ウィジェットを追加
//
void FontColorPref::pack_widget()
{
    const int mrg = 8;

    m_grid_font.property_margin() = 8;
    m_grid_font.set_column_spacing( 10 );
    m_grid_font.set_row_spacing( 8 );
    m_grid_font.set_vexpand( true );

    // フォント
    m_event_font.add( m_combo_font );
    m_combo_font.set_hexpand( false );
    m_fontbutton.set_hexpand( true );
    m_hbox_font.pack_start( m_event_font, Gtk::PACK_SHRINK );
    m_hbox_font.pack_start( m_fontbutton, Gtk::PACK_EXPAND_WIDGET );

    m_grid_font.attach( m_hbox_font, 0, 0, 2, 1 );

    m_checkbutton_font.add_label( "スレビューでフォント幅の近似計算を厳密に行う(_S)", true ),
    m_checkbutton_font.set_active( CONFIG::get_strict_char_width() );
    m_checkbutton_font.set_tooltip_text( WARNING_STRICTCHAR );
    m_grid_font.attach( m_checkbutton_font, 0, 1, 2, 1 );

    // 行高さ
    m_spin_space.set_digits( 1 );
    m_spin_space.set_range( 0.1, 10.0 );
    m_spin_space.set_increments( 0.1, 0.1 );
    m_spin_space.set_value( CONFIG::get_adjust_line_space() );
    m_spin_space.set_halign( Gtk::ALIGN_START );
    m_spin_space.set_hexpand( true );
    m_label_space.set_text_with_mnemonic( "スレビューの文字列の行の高さ(_H):" );
    m_label_space.set_mnemonic_widget( m_spin_space );
    m_label_space.set_halign( Gtk::ALIGN_START );

    m_grid_font.attach( m_label_space, 0, 2, 1, 1 );
    m_grid_font.attach( m_spin_space, 1, 2, 1, 1 );
    m_spin_space.set_tooltip_text( "スレビューにおいて行の高さを調節します( 標準は 1 )" );

    set_activate_entry( m_spin_space );

    // 下線位置
    m_spin_ubar.set_digits( 1 );
    m_spin_ubar.set_range( 0.1, 10.0 );
    m_spin_ubar.set_increments( 0.1, 0.1 );
    m_spin_ubar.set_value( CONFIG::get_adjust_underline_pos() );
    m_spin_ubar.set_halign( Gtk::ALIGN_START );
    m_spin_ubar.set_hexpand( true );
    m_label_ubar.set_text_with_mnemonic( "スレビューの文字列の下線位置(_U):" );
    m_label_ubar.set_mnemonic_widget( m_spin_ubar );
    m_label_ubar.set_halign( Gtk::ALIGN_START );

    m_grid_font.attach( m_label_ubar, 0, 3, 1, 1 );
    m_grid_font.attach( m_spin_ubar, 1, 3, 1, 1 );
    m_spin_ubar.set_tooltip_text( "スレビューにおいてアンカーなどの下線の位置を調節します( 標準は 1 )" );

    set_activate_entry( m_spin_ubar );

    // AAレスと判定する正規表現
    m_entry_aafont.set_text( CONFIG::get_regex_res_aa() );
    m_label_aafont.set_halign( Gtk::ALIGN_START );
    m_label_aafont.set_mnemonic_widget( m_entry_aafont );

    m_grid_font.attach( m_label_aafont, 0, 4, 1, 1 );
    m_grid_font.attach( m_entry_aafont, 1, 4, 1, 1 );
    constexpr const char* aafont_tooltip =
        "この正規表現に一致したレスは、アスキーアートフォントで表示します( 次に開いたスレから有効 )";
    m_label_aafont.set_tooltip_text( aafont_tooltip );
    m_entry_aafont.set_tooltip_text( aafont_tooltip );

    set_activate_entry( m_entry_aafont );

    // フォントのリセット
    m_bt_reset_font.set_valign( Gtk::ALIGN_END  );
    m_bt_reset_font.set_vexpand( true );
    m_bt_reset_font.signal_clicked().connect( sigc::mem_fun( *this, &FontColorPref::slot_reset_font ) );
    m_grid_font.attach( m_bt_reset_font, 0, 10, 2, 1 );

    m_notebook.append_page( m_grid_font, "フォントの設定" );

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
    m_scrollwin_color.set_min_content_height( 180 );
    m_scrollwin_color.set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS );
    m_vbox_color.pack_start( m_scrollwin_color, Gtk::PACK_EXPAND_WIDGET );

    Gtk::TreeViewColumn* column = Gtk::manage( new Gtk::TreeViewColumn( "設定名", m_columns_color.m_col_name ) );
    column->set_fixed_width( 430 );
    column->set_sizing( Gtk::TREE_VIEW_COLUMN_FIXED );
    column->set_resizable( true );
    m_treeview_color.append_column( *column );
    Gtk::CellRenderer *cell = column->get_first_cell();
    if( cell ) column->set_cell_data_func( *cell, sigc::mem_fun( *this, &FontColorPref::slot_cell_data_name ) );

    column = Gtk::manage( new Gtk::TreeViewColumn( "色", m_columns_color.m_col_color ) );
    m_treeview_color.append_column( *column );
    cell = column->get_first_cell();
    if( cell ) column->set_cell_data_func( *cell, sigc::mem_fun( *this, &FontColorPref::slot_cell_data_color ) );

    m_bt_change_color.signal_clicked().connect( sigc::mem_fun( *this, &FontColorPref::slot_change_color ) );
    m_bt_reset_color.signal_clicked().connect( sigc::mem_fun( *this, &FontColorPref::slot_reset_color ) );
    m_bt_reset_color_dark.signal_clicked().connect( sigc::mem_fun( *this, &FontColorPref::slot_reset_color_dark ) );

    m_hbox_change_color.set_spacing( mrg );
    m_hbox_change_color.pack_end( m_bt_reset_color_dark, Gtk::PACK_SHRINK );
    m_hbox_change_color.pack_end( m_bt_reset_color, Gtk::PACK_SHRINK );
    m_hbox_change_color.pack_end( m_label_reset_color, Gtk::PACK_SHRINK );
    m_hbox_change_color.pack_end( m_bt_change_color , Gtk::PACK_SHRINK );
    m_vbox_color.pack_start( m_hbox_change_color, Gtk::PACK_SHRINK );

    m_chk_use_gtktheme_message.add_label( "書き込みビューの配色設定に GTKテーマ を用いる(_W)", true );
    m_chk_use_gtktheme_message.set_active( CONFIG::get_use_message_gtktheme() );
    m_vbox_color.pack_start( m_chk_use_gtktheme_message, Gtk::PACK_SHRINK );

    m_chk_use_gtkrc_tree.add_label( "ツリービューの背景色設定に GTKテーマ を用いる(_T)", true ),
    m_chk_use_gtkrc_tree.set_active( CONFIG::get_use_tree_gtkrc() );
    m_vbox_color.pack_start( m_chk_use_gtkrc_tree, Gtk::PACK_SHRINK );

    m_chk_use_gtkrc_selection.add_label( "スレビューの文字色、背景色、選択範囲の色設定に GTKテーマ を用いる(_E)", true ),
    m_chk_use_gtkrc_selection.set_active( CONFIG::get_use_select_gtkrc() );
    m_vbox_color.pack_start( m_chk_use_gtkrc_selection, Gtk::PACK_SHRINK );

    m_chk_use_html_color.add_label( "スレビューで HTML タグで指定された文字色を用いる(_H)", true );
    m_chk_use_html_color.set_active( CONFIG::get_use_color_html() );
    m_vbox_color.pack_start( m_chk_use_html_color, Gtk::PACK_SHRINK );

    m_bt_reset_all_colors.signal_clicked().connect( sigc::mem_fun( *this, &FontColorPref::slot_reset_all_colors ) );
    m_bt_reset_all_colors_dark.signal_clicked().connect( sigc::mem_fun( *this, &FontColorPref::slot_reset_all_colors_dark ) );
    m_label_reset_all_colors.set_xalign( Gtk::ALIGN_END );
    m_bt_reset_all_colors.set_hexpand( false );
    m_bt_reset_all_colors_dark.set_hexpand( false );
    m_bt_reset_all_colors_dark.set_tooltip_text(
        "HTMLタグで指定された文字色は、ダークテーマでは視認性が低下する可能性があるため、無効にします。" );

    m_hbox_reset_all_colors.pack_end( m_bt_reset_all_colors_dark, Gtk::PACK_SHRINK );
    m_hbox_reset_all_colors.pack_end( m_bt_reset_all_colors, Gtk::PACK_SHRINK );
    m_hbox_reset_all_colors.pack_end( m_label_reset_all_colors, Gtk::PACK_SHRINK );
    m_vbox_color.pack_end( m_hbox_reset_all_colors, Gtk::PACK_SHRINK );

    // ディスプレイ解像度が小さい環境で表示できるようにスクロール可能にする
    m_scroll_color.add( m_vbox_color );
    m_scroll_color.set_policy( Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC );

    m_notebook.append_page( m_scroll_color, "色の設定" );

    // テーマ
    m_label_gtk_theme.set_halign( Gtk::ALIGN_START );
    m_label_gtk_theme.set_mnemonic_widget( m_combo_theme );
    m_combo_theme.set_hexpand( true );
    m_check_system_theme.set_halign( Gtk::ALIGN_START );
    m_label_dark_theme.set_halign( Gtk::ALIGN_START );

    m_grid_theme.property_margin() = 8;
    m_grid_theme.set_column_spacing( 10 );
    m_grid_theme.set_row_spacing( 8 );
    m_grid_theme.attach( m_label_gtk_theme, 0, 0, 1, 1 );
    m_grid_theme.attach( m_combo_theme, 1, 0, 1, 1 );
    m_grid_theme.attach( m_check_system_theme, 1, 1, 1, 1 );
    m_grid_theme.attach( m_label_dark_theme, 0, 2, 1, 1 );
    m_grid_theme.attach( m_check_dark_theme, 1, 2, 1, 1 );

    // アイコン
    m_label_icon_theme.set_halign( Gtk::ALIGN_START );
    m_label_icon_theme.set_mnemonic_widget( m_combo_icon );
    m_label_use_symbolic_icon.set_halign( Gtk::ALIGN_START );
    m_combo_icon.set_hexpand( true );
    m_check_system_icon.set_halign( Gtk::ALIGN_START );
    m_check_use_symbolic_icon.signal_toggled().connect(
        sigc::mem_fun( *this, &FontColorPref::slot_toggled_symbolic ) );

    m_grid_theme.attach( m_label_icon_theme, 0, 3, 1, 1 );
    m_grid_theme.attach( m_combo_icon, 1, 3, 1, 1 );
    m_grid_theme.attach( m_check_system_icon, 1, 4, 1, 1 );
    m_grid_theme.attach( m_label_use_symbolic_icon, 0, 5, 1, 1 );
    m_grid_theme.attach( m_check_use_symbolic_icon, 1, 5, 1, 1 );

    m_scroll_note.add( m_label_note );
    m_scroll_note.set_policy( Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC );
    m_label_note_title.set_markup(
        "<b>注意</b>\nテーマの設定は実験的なサポートのため、変更または廃止の可能性があります。\n" );
    m_label_note_title.set_halign( Gtk::ALIGN_CENTER );
    m_label_note_title.set_justify( Gtk::JUSTIFY_CENTER );
    m_label_note.set_halign( Gtk::ALIGN_CENTER );
    m_label_note.set_valign( Gtk::ALIGN_CENTER );
    m_label_note.set_line_wrap( true );
    m_label_note.set_vexpand( true );

    m_grid_theme.attach( m_label_note_title, 0, 6, 2, 1 );
    m_grid_theme.attach( m_scroll_note, 0, 7, 2, 1 );

    m_notebook.append_page( m_grid_theme, "テーマの設定" );

    // 全体
    get_content_area()->pack_start( m_notebook );
    get_content_area()->set_spacing( mrg );
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
    CONFIG::set_regex_res_aa( m_entry_aafont.get_text() );

    CONFIG::set_strict_char_width( m_checkbutton_font.property_active() );

    CONFIG::set_use_message_gtktheme( m_chk_use_gtktheme_message.property_active() );
    CONFIG::set_use_tree_gtkrc( m_chk_use_gtkrc_tree.property_active() );
    CONFIG::set_use_select_gtkrc( m_chk_use_gtkrc_selection.property_active() );
    const char* completely = "";
    const bool use_color = m_chk_use_html_color.property_active();
    if( use_color != CONFIG::get_use_color_html() ) {
        CONFIG::set_use_color_html( use_color );
        completely = "completely";
    }

    if( Glib::getenv( "GTK_THEME" ).empty() ) {
        if( m_check_system_theme.get_active() ) {
            // システム設定のGTKテーマを使うため、空文字列をセットする
            CONFIG::set_gtk_theme_name( "" );
        }
        else {
            CONFIG::set_gtk_theme_name( m_combo_theme.get_active_text() );
        }
    }
    CONFIG::set_use_dark_theme( m_check_dark_theme.get_active() );
    // GTKテーマを変更したときは、ビューで使用する文字色と背景色を更新する必要がある
    CONFIG::update_view_colors();

    if( m_check_system_icon.get_active() ) {
        // システム設定のアイコンテーマを使うため、空文字列をセットする
        CONFIG::set_gtk_icon_theme_name( "" );
    }
    else {
        CONFIG::set_gtk_icon_theme_name( m_combo_icon.get_active_text() );
    }
    CONFIG::set_use_symbolic_icon( m_check_use_symbolic_icon.get_active() );

    CORE::core_set_command( "relayout_all_bbslist" );
    CORE::core_set_command( "relayout_all_board" );

    CORE::core_set_command( "init_font_all_article" );
    CORE::core_set_command( "relayout_all_article", "", completely );

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
    m_event_font.set_tooltip_text( m_tooltips_font[ num ] );
    m_fontbutton.set_tooltip_text( m_tooltips_font[ num ] );
}


//
// チェックボックスの状態が変わった
//
void FontColorPref::slot_checkbutton_font_toggled()
{
    if( m_checkbutton_font.property_active() )
    {
        SKELETON::MsgDiag mdiag( nullptr, WARNING_STRICTCHAR );
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
    m_entry_aafont.set_text( CONF_REGEX_RES_AA_DEFAULT );
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
    static_cast<void>( row ); // cppcheck: unreadVariable
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
    Gtk::CellRendererText* rentext = dynamic_cast<Gtk::CellRendererText*>( cell );
    if( colorid != COLOR_NONE && CONFIG::get_color( colorid ) != defaultcolor ){
        rentext->property_foreground() = CONFIG::get_color( COLOR_CHAR_HIGHLIGHT_TREE );
        rentext->property_foreground_set() = true;
        rentext->property_cell_background() = CONFIG::get_color( COLOR_BACK_HIGHLIGHT_TREE );
        rentext->property_cell_background_set() = true;
    }
    else {
        rentext->property_foreground_set() = false;
        rentext->property_cell_background_set() = false;
    }
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
    Gtk::TreeRow row;

    std::vector< Gtk::TreePath > selection_path = m_treeview_color.get_selection()->get_selected_rows();
    if( selection_path.empty() ) return;

    int colorid = COLOR_NONE;
    if( selection_path.size() == 1 ){
        row = *m_liststore_color->get_iter( selection_path.front() );
        if( ! row ) return;
        colorid = row[ m_columns_color.m_col_colorid ];
        if( colorid == COLOR_NONE ) return;
    }

    Gtk::ColorSelectionDialog colordiag;
    if( colorid != COLOR_NONE ) {
        Gtk::ColorSelection* sel = colordiag.get_color_selection();
        sel->set_current_rgba( Gdk::RGBA( CONFIG::get_color( colorid ) ) );
    }
    colordiag.set_transient_for( *CORE::get_mainwindow() );
    const int ret = colordiag.run();

    if( ret == Gtk::RESPONSE_OK ){

        for( const Gtk::TreePath& path : selection_path ) {

            row = *m_liststore_color->get_iter( path );
            if( ! row ) continue;

            colorid = row[ m_columns_color.m_col_colorid ];
            if( colorid != COLOR_NONE ) {
                Gtk::ColorSelection* sel = colordiag.get_color_selection();
                CONFIG::set_color( colorid, MISC::color_to_str( sel->get_current_rgba() ) );
            }
        }
    }
}


//
// 選択行の色のリセット
//
void FontColorPref::slot_reset_color()
{
    std::vector< Gtk::TreePath > selection_path = m_treeview_color.get_selection()->get_selected_rows();
    if( selection_path.empty() ) return;

    for( const Gtk::TreePath& path : selection_path ) {

        Gtk::TreeRow row = *m_liststore_color->get_iter( path );
        if( ! row ) continue;

        const int colorid = row[ m_columns_color.m_col_colorid ];
        if( colorid != COLOR_NONE ){

            const std::string defaultcolor = row[ m_columns_color.m_col_default ];
            CONFIG::set_color( colorid , defaultcolor );
            m_treeview_color.queue_draw();
        }
    }
}


/**
 * @brief 選択行の色をダークテーマ用のデフォルト値にリセット
 */
void FontColorPref::slot_reset_color_dark()
{
    std::vector<Gtk::TreePath> selection_paths = m_treeview_color.get_selection()->get_selected_rows();
    if( selection_paths.empty() ) return;

    for( const Gtk::TreePath& path : selection_paths ) {

        Gtk::TreeRow row = *m_liststore_color->get_iter( path );
        if( ! row ) continue;

        const int colorid = row[ m_columns_color.m_col_colorid ];
        if( colorid != COLOR_NONE ) {

            const std::string default_dark = CONFIG::kDarkColors[ colorid ];
            CONFIG::set_color( colorid, default_dark );
            m_treeview_color.queue_draw();
        }
    }
}


//
// 全ての色のリセット
//
void FontColorPref::slot_reset_all_colors()
{
    m_chk_use_gtktheme_message.set_active( CONFIG::CONF_USE_MESSAGE_GTKTHEME );
    m_chk_use_gtkrc_tree.set_active( CONFIG::CONF_USE_TREE_GTKRC );
    m_chk_use_gtkrc_selection.set_active( CONFIG::CONF_USE_SELECT_GTKRC );
    m_chk_use_html_color.set_active( CONFIG::CONF_USE_COLOR_HTML );

    CONFIG::reset_colors();

    m_treeview_color.queue_draw();
}


/** @brief 全ての色をダークテーマ用のデフォルト値にリセット
 *
 * @details HTMLタグで指定された文字色は、ダークテーマでは視認性が低下する可能性があるため、無効にします。
 */
void FontColorPref::slot_reset_all_colors_dark()
{
    m_chk_use_gtktheme_message.set_active( CONFIG::CONF_USE_MESSAGE_GTKTHEME );
    m_chk_use_gtkrc_tree.set_active( CONFIG::CONF_USE_TREE_GTKRC );
    m_chk_use_gtkrc_selection.set_active( CONFIG::CONF_USE_SELECT_GTKRC );
    m_chk_use_html_color.set_active( false );

    CONFIG::reset_colors_dark_theme();

    m_treeview_color.queue_draw();
}


/**
 * @brief 「シンボリックアイコンで表示する」設定を切り替えたときアイコンを再読み込みする
 */
void FontColorPref::slot_toggled_symbolic()
{
    const bool use_symbolic = m_check_use_symbolic_icon.get_active();
    CONFIG::set_use_symbolic_icon( use_symbolic );
    ICON::get_icon_manager()->reload_themed_icons( use_symbolic );
    CORE::core_set_command( "reload_ui_icon" );
}
