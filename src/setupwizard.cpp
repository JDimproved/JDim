// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "setupwizard.h"
#include "prefdiagfactory.h"
#include "fontid.h"

#include "config/globalconf.h"

#include "icons/iconmanager.h"

using namespace CORE;

enum
{
    SPACING_SIZE = 8
};

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
                     m_proxy( "プロキシ設定(_P)", true ),
                     m_browser( "ブラウザ設定(_W)", true ),
                     m_frame( "ブラウザ起動コマンド" ),   // フレーム
                     m_label_browser( CONFIG::get_command_openurl(), Gtk::ALIGN_LEFT )
{

    m_icon.set( ICON::get_icon_manager()->get_icon( ICON::JD48 ) );
    m_hbox_label.set_spacing( SPACING_SIZE );
    m_hbox_label.pack_start( m_icon, Gtk::PACK_SHRINK );
    m_hbox_label.pack_start( m_label );

    m_vbox.set_spacing( SPACING_SIZE );
    m_vbox.pack_start( m_proxy, Gtk::PACK_SHRINK );
    m_vbox.pack_start( m_browser, Gtk::PACK_SHRINK );    

    // m_hbox_commnad に m_label_browser を格納
    m_hbox_command.set_spacing( SPACING_SIZE );
    m_hbox_command.pack_start( m_label_browser, Gtk::PACK_SHRINK );
    m_hbox_command.set_border_width( SPACING_SIZE );

    // フレーム内にHBoxを格納
    m_frame.add( m_hbox_command );
   
    m_proxy.signal_clicked().connect( sigc::mem_fun( *this, &PageNet::slot_setup_proxy ) );
    m_browser.signal_clicked().connect( sigc::mem_fun( *this, &PageNet::slot_setup_browser ) );

    set_spacing( SPACING_SIZE );
    pack_start( m_hbox_label, Gtk::PACK_SHRINK );
    pack_start( m_vbox, Gtk::PACK_SHRINK );

    // フレームの追加
    pack_start( m_frame, Gtk::PACK_SHRINK );
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
                       m_label_res( "スレ(_T)", Gtk::ALIGN_LEFT, Gtk::ALIGN_TOP, true ),
                       m_label_popup( "ポップアップ(_P)", Gtk::ALIGN_LEFT, Gtk::ALIGN_TOP, true ),
                       m_label_tree( "板／スレ一覧(_O)", Gtk::ALIGN_LEFT, Gtk::ALIGN_TOP,  true ),
                       m_font_res( "スレフォント" ),
                       m_font_popup( "ポップアップフォント" ),
                       m_font_tree( "板／スレ一覧フォント" )
{
    m_label_res.set_mnemonic_widget( m_font_res );
    m_label_popup.set_mnemonic_widget( m_font_popup );
    m_label_tree.set_mnemonic_widget( m_font_tree );

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
    CONFIG::set_fontname( FONT_MESSAGE, m_font_res.get_font_name() );
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
    : Gtk::Dialog(), m_back( "<< 戻る(_B)", true ), m_next( "次へ(_N) >>", true )
{
    set_title( "JD セットアップウィザード" );
    set_keep_above( true );
    set_resizable( false );

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
    m_notebook.append_page( m_page_end );

    m_notebook.set_border_width( SPACING_SIZE );
    m_notebook.set_show_border( false );
    m_notebook.set_show_tabs( false );
    m_sigc_switch_page = m_notebook.signal_switch_page().connect( sigc::mem_fun( *this, &SetupWizard::slot_switch_page ) );

    get_vbox()->pack_start( m_notebook, Gtk::PACK_EXPAND_PADDING, SPACING_SIZE );

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
            m_fin->set_sensitive( false );
            break;
            
        case 1:
            m_back.set_sensitive( true );
            m_next.set_sensitive( true );
            m_fin->set_sensitive( false );
            break;

        case 2:
            m_back.set_sensitive( true );
            m_next.set_sensitive( true );
            m_fin->set_sensitive( false );
            break;

        case 3:
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
