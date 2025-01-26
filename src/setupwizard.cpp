// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "setupwizard.h"
#include "prefdiagfactory.h"
#include "fontid.h"
#include "session.h"

#include "config/globalconf.h"

#include "icons/iconmanager.h"

using namespace CORE;

enum
{
    SPACING_SIZE = 8
};

PageStart::PageStart()
    : Gtk::Grid()
    , m_icon( ICON::get_icon_manager()->get_pixbuf( ICON::JD48 ) )
    , m_label( "１/６．JDim セットアップ開始", Gtk::ALIGN_START )
    , m_label2( "JDimセットアップウィザードへようこそ\n\n"
                "このウィザードでネットワークとフォント等の設定をおこないます\n\n"
                "設定を始めるには［次へ］を押してください",
                Gtk::ALIGN_START )
{
    set_column_spacing( SPACING_SIZE );
    set_row_spacing( SPACING_SIZE );

    // Gtk::Grid::attach( child, column, row, width, height )
    attach( m_icon, 0, 0, 1, 1 );
    attach( m_label, 1, 0, 1, 1 );
    attach( m_label2, 0, 1, 2, 1 );

    m_label.set_hexpand( true );
}


/////////////////////////////////////////////


PageNet::PageNet( Gtk::Window* parent )
    : Gtk::Grid()
    , m_parent{ parent }
    , m_icon( ICON::get_icon_manager()->get_pixbuf( ICON::JD48 ) )
    , m_label( "２/６．ネットワークの設定をします", Gtk::ALIGN_START )
    , m_proxy( "プロキシ設定(_P)", true )
    , m_browser( "ブラウザ設定(_W)", true )
    , m_frame( "ブラウザ起動コマンド" ) // フレーム
    , m_label_browser( CONFIG::get_command_openurl(), Gtk::ALIGN_START )
{
    m_label_browser.property_margin() = SPACING_SIZE;
    m_frame.add( m_label_browser );

    m_proxy.signal_clicked().connect( sigc::mem_fun( *this, &PageNet::slot_setup_proxy ) );
    m_browser.signal_clicked().connect( sigc::mem_fun( *this, &PageNet::slot_setup_browser ) );

    set_column_spacing( SPACING_SIZE );
    set_row_spacing( SPACING_SIZE );

    // Gtk::Grid::attach( child, column, row, width, height )
    attach( m_icon, 0, 0, 1, 1 );
    attach( m_label, 1, 0, 1, 1 );
    attach( m_proxy, 0, 1, 2, 1 );
    attach( m_browser, 0, 2, 2, 1 );
    attach( m_frame, 0, 3, 2, 1 ); // フレームの追加

    m_label.set_hexpand( true );
}


//
// プロキシ設定
//
void PageNet::slot_setup_proxy()
{
    auto pref = CORE::PrefDiagFactory( m_parent, CORE::PREFDIAG_PROXY, "" );
    pref->run();
}


//
// ブラウザ設定
//
void PageNet::slot_setup_browser()
{
    auto pref = CORE::PrefDiagFactory( m_parent, CORE::PREFDIAG_BROWSER, "" );
    pref->run();

    m_label_browser.set_text( CONFIG::get_command_openurl() );
}



/////////////////////////////////////////////


PageFont::PageFont()
    : Gtk::Grid()
    , m_icon( ICON::get_icon_manager()->get_pixbuf( ICON::JD48 ) )
    , m_label( "３/６．フォントの設定をします", Gtk::ALIGN_START )
    , m_label_res( "スレ(_T)", Gtk::ALIGN_START, Gtk::ALIGN_CENTER, true )
    , m_label_mail( "メール(_U)", Gtk::ALIGN_START, Gtk::ALIGN_CENTER, true )
    , m_label_popup( "ポップアップ(_P)", Gtk::ALIGN_START, Gtk::ALIGN_CENTER, true )
    , m_label_tree( "板／スレ一覧(_O)", Gtk::ALIGN_START, Gtk::ALIGN_CENTER, true )
    , m_font_res( "スレフォント" )
    , m_font_mail( "メール欄フォント" )
    , m_font_popup( "ポップアップフォント" )
    , m_font_tree( "板／スレ一覧フォント" )
{
    m_label_res.set_mnemonic_widget( m_font_res );
    m_label_mail.set_mnemonic_widget( m_font_mail );
    m_label_popup.set_mnemonic_widget( m_font_popup );
    m_label_tree.set_mnemonic_widget( m_font_tree );

    m_font_res.set_font_name( CONFIG::get_fontname( FONT_MAIN ) );
    m_font_mail.set_font_name( CONFIG::get_fontname( FONT_MAIL ) );
    m_font_popup.set_font_name( CONFIG::get_fontname( FONT_POPUP ) );
    m_font_tree.set_font_name( CONFIG::get_fontname( FONT_BBS ) );

    m_font_res.signal_font_set().connect( sigc::mem_fun( *this, &PageFont::slot_font_res ) ); 
    m_font_mail.signal_font_set().connect( sigc::mem_fun( *this, &PageFont::slot_font_mail ) ); 
    m_font_popup.signal_font_set().connect( sigc::mem_fun( *this, &PageFont::slot_font_popup ) );
    m_font_tree.signal_font_set().connect( sigc::mem_fun( *this, &PageFont::slot_font_tree ) );

    m_font_res.set_hexpand( true );
    m_font_mail.set_hexpand( true );
    m_font_popup.set_hexpand( true );
    m_font_tree.set_hexpand( true );

    constexpr int width = 1;
    constexpr int height = 1;
    m_table.attach( m_label_res, 0, 0, width, height );
    m_table.attach( m_label_mail, 0, 1, width, height );
    m_table.attach( m_label_popup, 0, 2, width, height );
    m_table.attach( m_label_tree, 0, 3, width, height );

    m_table.attach( m_font_res, 1, 0, width, height );
    m_table.attach( m_font_mail, 1, 1, width, height );
    m_table.attach( m_font_popup, 1, 2, width, height );
    m_table.attach( m_font_tree, 1, 3, width, height );

    set_column_spacing( SPACING_SIZE );
    set_row_spacing( SPACING_SIZE );

    // Gtk::Grid::attach( child, column, row, width, height )
    attach( m_icon, 0, 0, 1, 1 );
    attach( m_label, 1, 0, 1, 1 );
    attach( m_table, 0, 1, 2, 1 );

    m_label.set_hexpand( true );
    m_table.set_hexpand( true );
    m_table.set_row_spacing( 4 );
    m_table.set_column_spacing( 4 );
}


void PageFont::slot_font_res()
{
    CONFIG::set_fontname( FONT_MAIN, m_font_res.get_font_name() );
    CONFIG::set_fontname( FONT_MESSAGE, m_font_res.get_font_name() );
}

void PageFont::slot_font_mail()
{
    CONFIG::set_fontname( FONT_MAIL, m_font_mail.get_font_name() );
}

void PageFont::slot_font_popup()
{
    CONFIG::set_fontname( FONT_POPUP, m_font_popup.get_font_name() );
}

void PageFont::slot_font_tree()
{
    CONFIG::set_fontname( FONT_BBS, m_font_tree.get_font_name() );
    CONFIG::set_fontname( FONT_BOARD, m_font_tree.get_font_name() );
}



/////////////////////////////////////////////


PageTheme::PageTheme()
    : Gtk::Grid()
    , m_icon( ICON::get_icon_manager()->get_pixbuf( ICON::JD48 ) )
    , m_label( "４/６．テーマの設定をします", Gtk::ALIGN_START )
    , m_system_theme( m_radiogroup, "システム設定(_S)", true )
    , m_dark_theme( m_radiogroup, "ダークテーマ(_D)", true )
    , m_label_inst( "", Gtk::ALIGN_START )
{
    m_label_inst.set_line_wrap( true );
    m_label_inst.set_hexpand( false );

    if( CONFIG::get_use_dark_theme() ) {
        m_dark_theme.set_active( true );
        slot_dark_theme();
    }
    else {
        m_system_theme.set_active( true );
        slot_system_theme();
    }

    m_system_theme.signal_toggled().connect( sigc::mem_fun( *this, &PageTheme::slot_system_theme ) );
    m_dark_theme.signal_toggled().connect( sigc::mem_fun( *this, &PageTheme::slot_dark_theme ) );

    set_column_spacing( SPACING_SIZE );
    set_row_spacing( SPACING_SIZE );

    // Gtk::Grid::attach( child, column, row, width, height )
    attach( m_icon, 0, 0, 1, 1 );
    attach( m_label, 1, 0, 1, 1 );
    attach( m_system_theme, 0, 1, 2, 1 );
    attach( m_dark_theme, 0, 2, 2, 1 );
    attach( m_label_inst, 0, 3, 2, 1 );

    m_label.set_hexpand( true );
}


/**
 * @brief システム設定を選択します。
 */
void PageTheme::slot_system_theme()
{
    CONFIG::set_use_dark_theme( false );
    m_label_inst.set_text( "システム設定のGTKテーマで表示します。" );
}


/**
 * @brief ダークテーマを選択します。
 */
void PageTheme::slot_dark_theme()
{
    CONFIG::set_use_dark_theme( true );
    m_label_inst.set_text( "ダークテーマ（暗いカラーパターン）で表示します。" );
}


/////////////////////////////////////////////


PagePane::PagePane()
    : Gtk::Grid()
    , m_icon( ICON::get_icon_manager()->get_pixbuf( ICON::JD48 ) )
    , m_label( "５/６．ペイン表示設定をします", Gtk::ALIGN_START )
    , m_2pane( m_radiogroup, "２ペイン表示(_2)", true )
    , m_3pane( m_radiogroup, "３ペイン表示(_3)", true )
    , m_v3pane( m_radiogroup, "縦３ペイン表示(_V)", true )
    , m_label_inst( "", Gtk::ALIGN_START )
{
    switch( SESSION::get_mode_pane() ){
        case SESSION::MODE_2PANE: m_2pane.set_active( true ); slot_2pane(); break;
        case SESSION::MODE_3PANE: m_3pane.set_active( true ); slot_3pane(); break;
        case SESSION::MODE_V3PANE: m_v3pane.set_active( true ); slot_v3pane(); break;
    }

    m_2pane.signal_toggled().connect( sigc::mem_fun( *this, &PagePane::slot_2pane ) );
    m_3pane.signal_toggled().connect( sigc::mem_fun( *this, &PagePane::slot_3pane ) );
    m_v3pane.signal_toggled().connect( sigc::mem_fun( *this, &PagePane::slot_v3pane ) );

    set_column_spacing( SPACING_SIZE );
    set_row_spacing( SPACING_SIZE );

    // Gtk::Grid::attach( child, column, row, width, height )
    attach( m_icon, 0, 0, 1, 1 );
    attach( m_label, 1, 0, 1, 1 );
    attach( m_2pane, 0, 1, 2, 1 );
    attach( m_3pane, 0, 2, 2, 1 );
    attach( m_v3pane, 0, 3, 2, 1 );
    attach( m_label_inst, 0, 4, 2, 1 );

    m_label.set_hexpand( true );
}


void PagePane::slot_2pane()
{
    SESSION::set_mode_pane( SESSION::MODE_2PANE );
    m_label_inst.set_text( "ウィンドウの左に板一覧、右にスレ一覧とスレビューを切り替え表示" );
}


void PagePane::slot_3pane()
{
    SESSION::set_mode_pane( SESSION::MODE_3PANE );
    m_label_inst.set_text( "ウィンドウの左に板一覧、右上にスレ一覧、右下にスレビューを表示" );
}


void PagePane::slot_v3pane()
{
    SESSION::set_mode_pane( SESSION::MODE_V3PANE );
    // ウインドウの幅を固定するため空白で長さを揃える
    m_label_inst.set_text( "ウィンドウの左に板一覧、中央にスレ一覧、右にスレビューを表示　" );
}


/////////////////////////////////////////////


PageEnd::PageEnd()
    : Gtk::Grid()
    , m_icon( ICON::get_icon_manager()->get_pixbuf( ICON::JD48 ) )
    , m_label( "６/６．JDim セットアップ完了", Gtk::ALIGN_START )
    , m_label2( "その他の設定は起動後に設定及び表示メニューからおこなって下さい\n\n"
                "完了を押すとJDimを起動して板一覧のリストをロードします\n"
                "板一覧が表示されるまでしばらくお待ち下さい",
                Gtk::ALIGN_START )
{
    set_column_spacing( SPACING_SIZE );
    set_row_spacing( SPACING_SIZE );

    // Gtk::Grid::attach( child, column, row, width, height )
    attach( m_icon, 0, 0, 1, 1 );
    attach( m_label, 1, 0, 1, 1 );
    attach( m_label2, 0, 1, 2, 1 );

    m_label.set_hexpand( true );
}


/////////////////////////////////////////////


SetupWizard::SetupWizard()
    : Gtk::Dialog()
    , m_page_network{ this }
    , m_back( "<< 戻る(_B)", true )
    , m_next( "次へ(_N) >>", true )
{
    set_title( "JDim セットアップウィザード" );
    set_keep_above( true );
    set_resizable( false );
    set_position( Gtk::WIN_POS_CENTER ); // 配置はデスクトップ環境次第

    // ボタン
    get_action_area()->set_spacing( SPACING_SIZE / 2 );
    get_action_area()->pack_start( m_back, Gtk::PACK_SHRINK );
    get_action_area()->pack_start( m_next, Gtk::PACK_SHRINK );
    m_fin = add_button( "完了(_C)", Gtk::RESPONSE_OK );

    m_back.set_sensitive( false );
    m_next.set_sensitive( true );
    m_fin->set_sensitive( false );

    m_back.signal_clicked().connect( sigc::mem_fun( *this, &SetupWizard::slot_back ) );
    m_next.signal_clicked().connect( sigc::mem_fun( *this, &SetupWizard::slot_next ) );

    // ページ
    m_notebook.append_page( m_page_start );
    m_notebook.append_page( m_page_network );
    m_notebook.append_page( m_page_font );
    m_notebook.append_page( m_page_theme );
    m_notebook.append_page( m_page_pane );
    m_notebook.append_page( m_page_end );

    m_notebook.set_border_width( SPACING_SIZE );
    m_notebook.set_show_border( false );
    m_notebook.set_show_tabs( false );
    m_sigc_switch_page = m_notebook.signal_switch_page().connect( sigc::mem_fun( *this, &SetupWizard::slot_switch_page ) );

    get_content_area()->pack_start( m_notebook, Gtk::PACK_EXPAND_PADDING, SPACING_SIZE );

    show_all_children();
}


SetupWizard::~SetupWizard()
{
    if( m_sigc_switch_page.connected() ) m_sigc_switch_page.disconnect(); 
}


void SetupWizard::slot_switch_page( Gtk::Widget* notebookpage, guint page )
{
    switch( page ){

        case 0:
            m_back.set_sensitive( false );
            m_next.set_sensitive( true );
            m_fin->set_sensitive( false );
            break;
            
        case 1:
        case 2:
        case 3:
        case 4:
            m_back.set_sensitive( true );
            m_next.set_sensitive( true );
            m_fin->set_sensitive( false );
            break;

        case 5:
            m_back.set_sensitive( true );
            m_next.set_sensitive( false );
            m_fin->set_sensitive( true );
            m_fin->grab_focus();
            break;
    }
}

void SetupWizard::slot_back()
{
    int page = m_notebook.get_current_page();
    m_notebook.set_current_page( page - 1 );
}

void SetupWizard::slot_next()
{
    int page = m_notebook.get_current_page();
    m_notebook.set_current_page( page + 1 );
}
