// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "setupwizard.h"
#include "prefdiagfactory.h"
#include "fontid.h"

#include "config/globalconf.h"

#include "icons/iconmanager.h"

using namespace CORE;

#define SPACING_SIZE 8

PageStart::PageStart() : Gtk::VBox(),
                         m_label( "１/４．JD セットアップ開始", Gtk::ALIGN_LEFT ),
                         m_label2( "JDセットアップウィザードへようこそ\n\nこのウィザードでネットワークとフォントの設定をおこないます\n\n設定を始めるには［次へ］を押してください", Gtk::ALIGN_LEFT )
{
    m_icon.set( ICON::get_icon_manager()->get_icon( ICON::JD48 ) );
    m_hbox_label.set_spacing( SPACING_SIZE );
    m_hbox_label.pack_start( m_icon, Gtk::PACK_SHRINK );
    m_hbox_label.pack_start( m_label );

    set_spacing( SPACING_SIZE );
    pack_start( m_hbox_label, Gtk::PACK_SHRINK );
    pack_start( m_label2, Gtk::PACK_SHRINK );
}


/////////////////////////////////////////////


PageNet::PageNet() : Gtk::VBox(),
                     m_label( "２/４．ネットワークの設定をします", Gtk::ALIGN_LEFT ),
                     m_proxy( "プロキシ設定" ),
                     m_browser( "ブラウザ設定" ),
                     m_label_browser( CONFIG::get_command_openurl(), Gtk::ALIGN_LEFT )
{
    m_icon.set( ICON::get_icon_manager()->get_icon( ICON::JD48 ) );
    m_hbox_label.set_spacing( SPACING_SIZE );
    m_hbox_label.pack_start( m_icon, Gtk::PACK_SHRINK );
    m_hbox_label.pack_start( m_label );

    m_vbox.set_spacing( SPACING_SIZE );
    m_vbox.pack_start( m_proxy, Gtk::PACK_SHRINK );
    m_vbox.pack_start( m_browser, Gtk::PACK_SHRINK );
    m_vbox.pack_start( m_label_browser, Gtk::PACK_SHRINK );

    m_proxy.signal_clicked().connect( sigc::mem_fun( *this, &PageNet::slot_setup_proxy ) );
    m_browser.signal_clicked().connect( sigc::mem_fun( *this, &PageNet::slot_setup_browser ) );

    set_spacing( SPACING_SIZE );
    pack_start( m_hbox_label, Gtk::PACK_SHRINK );
    pack_start( m_vbox, Gtk::PACK_SHRINK );
}


//
// プロキシ設定
//
void PageNet::slot_setup_proxy()
{
    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_PROXY, "" );
    pref->run();
    delete pref;
}


//
// ブラウザ設定
//
void PageNet::slot_setup_browser()
{
    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_BROWSER, "" );
    pref->run();
    delete pref;

    m_label_browser.set_text( CONFIG::get_command_openurl() );
}



/////////////////////////////////////////////


PageFont::PageFont() : Gtk::VBox(),
                       m_label( "３/４．フォントの設定をします", Gtk::ALIGN_LEFT ),
                       m_table( 2, 3 ),
                       m_label_res( "スレ", Gtk::ALIGN_LEFT ),
                       m_label_popup( "ポップアップ", Gtk::ALIGN_LEFT ),
                       m_label_tree( "板／スレ一覧", Gtk::ALIGN_LEFT ),
                       m_font_res( "スレフォント" ),
                       m_font_popup( "ポップアップフォント" ),
                       m_font_tree( "板／スレ一覧フォント" )
{
    m_icon.set( ICON::get_icon_manager()->get_icon( ICON::JD48 ) );
    m_hbox_label.set_spacing( SPACING_SIZE );
    m_hbox_label.pack_start( m_icon, Gtk::PACK_SHRINK );
    m_hbox_label.pack_start( m_label );

    m_font_res.set_font_name( CONFIG::get_fontname( FONT_MAIN ) );
    m_font_popup.set_font_name( CONFIG::get_fontname( FONT_POPUP ) );
    m_font_tree.set_font_name( CONFIG::get_fontname( FONT_BBS ) );

    m_font_res.signal_font_set().connect( sigc::mem_fun( *this, &PageFont::slot_font_res ) ); 
    m_font_popup.signal_font_set().connect( sigc::mem_fun( *this, &PageFont::slot_font_popup ) );
    m_font_tree.signal_font_set().connect( sigc::mem_fun( *this, &PageFont::slot_font_tree ) );

    m_table.set_spacings( 4 );
    m_table.attach( m_label_res, 0, 1, 0, 1, Gtk::FILL, Gtk::SHRINK );
    m_table.attach( m_label_popup, 0, 1, 1, 2, Gtk::FILL, Gtk::SHRINK );
    m_table.attach( m_label_tree, 0, 1, 2, 3, Gtk::FILL, Gtk::SHRINK );

    m_table.attach( m_font_res, 1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK );
    m_table.attach( m_font_popup, 1, 2, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK );
    m_table.attach( m_font_tree, 1, 2, 2, 3, Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK );

    set_spacing( SPACING_SIZE );
    pack_start( m_hbox_label, Gtk::PACK_SHRINK );
    pack_start( m_table, Gtk::PACK_EXPAND_WIDGET );
}


void PageFont::slot_font_res()
{
    CONFIG::set_fontname( FONT_MAIN, m_font_res.get_font_name() );
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


PageEnd::PageEnd() : Gtk::VBox(),
                     m_label( "４/４．JD セットアップ完了", Gtk::ALIGN_LEFT ),
                     m_label2( "その他の設定は起動後に設定メニューからおこなって下さい\n\n完了を押すとJDを起動して板のリストをロードします\nリストが表示されるまでしばらくお待ち下さい" , Gtk::ALIGN_LEFT )
{
    m_icon.set( ICON::get_icon_manager()->get_icon( ICON::JD48 ) );
    m_hbox_label.set_spacing( SPACING_SIZE );
    m_hbox_label.pack_start( m_icon, Gtk::PACK_SHRINK );
    m_hbox_label.pack_start( m_label );

    set_spacing( SPACING_SIZE );
    pack_start( m_hbox_label, Gtk::PACK_SHRINK );
    pack_start( m_label2, Gtk::PACK_SHRINK );
}


/////////////////////////////////////////////


SetupWizard::SetupWizard()
    : Gtk::Dialog(), m_fin( "完了(_C)", true ), m_back( "<< 戻る(_B)", true ), m_next( "次へ(_N) >>", true )
{
    set_title( "JD セットアップウィザード" );

    // ボタン
    m_hbox_buttons.set_spacing( SPACING_SIZE / 2 );
    m_hbox_buttons.pack_end( m_fin, Gtk::PACK_SHRINK );
    m_hbox_buttons.pack_end( m_next, Gtk::PACK_SHRINK );
    m_hbox_buttons.pack_end( m_back, Gtk::PACK_SHRINK );

    m_back.set_sensitive( false );
    m_next.set_sensitive( true );
    m_fin.set_sensitive( false );

    m_back.signal_clicked().connect( sigc::mem_fun( *this, &SetupWizard::slot_back ) );
    m_next.signal_clicked().connect( sigc::mem_fun( *this, &SetupWizard::slot_next ) );
    m_fin.signal_clicked().connect( sigc::mem_fun( *this, &SetupWizard::slot_fin ) );

    // ページ
    m_notebook.append_page( m_page_start );
    m_notebook.append_page( m_page_network );
    m_notebook.append_page( m_page_font );
    m_notebook.append_page( m_page_end );

    m_notebook.set_border_width( SPACING_SIZE );
    m_notebook.set_show_border( false );
    m_notebook.set_show_tabs( false );
    m_sigc_switch_page = m_notebook.signal_switch_page().connect( sigc::mem_fun( *this, &SetupWizard::slot_switch_page ) );

    get_vbox()->pack_start( m_notebook, Gtk::PACK_EXPAND_PADDING, SPACING_SIZE );
    get_vbox()->pack_start( m_hbox_buttons, Gtk::PACK_SHRINK );

    show_all_children();
}


SetupWizard::~SetupWizard()
{
    if( m_sigc_switch_page.connected() ) m_sigc_switch_page.disconnect(); 
}


void SetupWizard::slot_switch_page( GtkNotebookPage* notebookpage, guint page )
{
    switch( page ){

        case 0:
            m_back.set_sensitive( false );
            m_next.set_sensitive( true );
            m_fin.set_sensitive( false );
            break;
            
        case 1:
            m_back.set_sensitive( true );
            m_next.set_sensitive( true );
            m_fin.set_sensitive( false );
            break;

        case 2:
            m_back.set_sensitive( true );
            m_next.set_sensitive( true );
            m_fin.set_sensitive( false );
            break;

        case 3:
            m_back.set_sensitive( true );
            m_next.set_sensitive( false );
            m_fin.set_sensitive( true );
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

void SetupWizard::slot_fin()
{
    response( Gtk::RESPONSE_YES );
}
