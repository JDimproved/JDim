// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "aboutdiag.h"

#include "command.h"
#include "environment.h"
#include "session.h"

#include "config/globalconf.h"
#include "icons/iconmanager.h"
#include "jdlib/miscgtk.h"

using namespace SKELETON;

enum
{
    MARGIN = 8
};


AboutDiag::AboutDiag( const Glib::ustring& title )
    : Gtk::Dialog( title ),
      m_button_copy_environment( "クリップボードへコピー" )
{
    set_transient_for( *CORE::get_mainwindow() );
    set_resizable( false );

    Gtk::Button* button = add_button( Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE );
    button->signal_clicked().connect( sigc::mem_fun( *this, &AboutDiag::slot_close_clicked ) );

    set_default_response( Gtk::RESPONSE_CLOSE );

    set_logo( ICON::get_icon( ICON::JD96 ) );
    set_version( ENVIRONMENT::get_jdversion() );
    set_comments( ENVIRONMENT::get_jdcomments() );
    set_website( CONFIG::get_url_jdimhp() );
    set_copyright( ENVIRONMENT::get_jdcopyright() );
    set_license( ENVIRONMENT::get_jdlicense() );
    set_environment_list();
}


//
// run()
//
int AboutDiag::run()
{
#ifdef _DEBUG
    std::cout << "AboutDiag::run start\n";
#endif

    init();

    SESSION::set_dialog_shown( true );
    CORE::core_set_command( "dialog_shown" );

    int ret = Gtk::Dialog::run();

    SESSION::set_dialog_shown( false );
    CORE::core_set_command( "dialog_hidden" );

#ifdef _DEBUG
    std::cout << "AboutDiag::run fin\n";
#endif
    return ret;
}


//
// 各ウィジェットを配置して初期化
//
void AboutDiag::init()
{
    // 情報タブの追加
    m_label_tab_info.set_label( "情報" );
    m_vbox_info.set_spacing( MARGIN );
    m_vbox_info.set_border_width( MARGIN );
    // ロゴ
    if( get_logo() )
    {
        m_vbox_info.pack_start( m_image_logo, Gtk::PACK_SHRINK );
    }
    // バージョン 
    if( ! get_version().empty() )
    {
        m_vbox_info.pack_start( m_label_version, Gtk::PACK_EXPAND_WIDGET, MARGIN );
    }
    // コメント
    if( ! get_comments().empty() )
    {
        m_vbox_info.pack_start( m_label_comments, Gtk::PACK_SHRINK );
    }
    // コピーライト
    if( ! get_copyright().empty() )
    {
        m_label_copyright.set_justify( Gtk::JUSTIFY_CENTER );
        m_vbox_info.pack_start( m_label_copyright, Gtk::PACK_SHRINK );
    }
    // Webサイト
    if( ! get_website_label().empty() )
    {
        m_hbox_url.pack_start( m_button_website, Gtk::PACK_EXPAND_PADDING );
        m_vbox_info.pack_start( m_hbox_url, Gtk::PACK_SHRINK );
    }
    m_notebook.append_page( m_vbox_info, m_label_tab_info );

    // ライセンスタブの追加
    if( ! get_license().empty() )
    {
        m_label_tab_license.set_label( "ライセンス" );
        m_notebook.append_page( m_scrollwindow_license, m_label_tab_license );
    }

    // 動作環境タブの追加
    m_label_tab_environment.set_label( "動作環境" );
    m_notebook.append_page( m_vbox_environment, m_label_tab_environment );

    // 動作環境一覧
    m_vbox_environment.pack_start( m_scrollwindow_environment, Gtk::PACK_EXPAND_WIDGET );

    // クリップボードへコピーのボタン
    m_button_copy_environment.signal_clicked().connect( sigc::mem_fun( *this, &AboutDiag::slot_copy_environment ) );
    m_hbuttonbox_environment.set_layout( Gtk::BUTTONBOX_END );
    m_hbuttonbox_environment.pack_start( m_button_copy_environment, Gtk::PACK_SHRINK );
    m_vbox_environment.pack_start( m_hbuttonbox_environment, Gtk::PACK_SHRINK );

    get_vbox()->pack_start( m_notebook, Gtk::PACK_EXPAND_WIDGET, MARGIN );

    show_all_children();

    // バージョンの文字列の長さが短い( x.x.x-YYMMDD )と
    // ウィンドウの幅が狭すぎるので"幅/高さ"いずれかの大
    // きい方に合わせて4:3を保持するようにする。
    int win_w, win_h;
    get_size( win_w, win_h );
    if( win_w * 3 / 4 < win_h ) set_size_request( win_h * 4 / 3, -1 );
    else set_size_request( -1, win_w * 3 / 4 );
}


//
// ロゴ
//
void AboutDiag::set_logo( const Glib::RefPtr< Gdk::Pixbuf >& logo )
{
    m_image_logo.set( logo );
}

Glib::RefPtr< Gdk::Pixbuf > AboutDiag::get_logo()
{
   return m_image_logo.get_pixbuf();
}


//
// バージョン表示
//
void AboutDiag::set_version( const Glib::ustring& version )
{
    m_label_version.set_label( version );

#if GTKMM_CHECK_VERSION(3,0,0)
    Pango::FontDescription font_discription_version = m_label_version.get_style_context()->get_font();
#else
    Pango::FontDescription font_discription_version = m_label_version.get_style()->get_font();
#endif
    const int label_version_font_size = font_discription_version.get_size();
    font_discription_version.set_size( label_version_font_size * 5 / 3 );
    font_discription_version.set_weight( Pango::WEIGHT_BOLD );
#if GTKMM_CHECK_VERSION(3,0,0)
    m_label_version.override_font( font_discription_version );
#else
    m_label_version.modify_font( font_discription_version );
#endif
}

Glib::ustring AboutDiag::get_version()
{
    return m_label_version.get_label();
}


//
// コメント
//
void AboutDiag::set_comments( const Glib::ustring& comments )
{
    m_label_comments.set_label( comments );
}

Glib::ustring AboutDiag::get_comments()
{
    return m_label_comments.get_label();
}


//
// コピーライト
//
void AboutDiag::set_copyright( const Glib::ustring& copyright )
{
    m_label_copyright.set_label( copyright );
}

Glib::ustring AboutDiag::get_copyright()
{
    return m_label_copyright.get_label();
}


//
// Webサイト
//
void AboutDiag::set_website( const Glib::ustring& website )
{
    if( m_button_website.get_label().empty() ) m_button_website.set_label( website );
    m_button_website.set_relief( Gtk::RELIEF_NONE );
    m_button_website.signal_clicked().connect( sigc::mem_fun( this, &AboutDiag::slot_button_website_clicked ) );

    m_website_url = website;
}

Glib::ustring AboutDiag::get_website()
{
    return m_website_url;
}


//
// Webサイトラベル
//
void AboutDiag::set_website_label( const Glib::ustring& website_label )
{
    m_button_website.set_label( website_label );
}

Glib::ustring AboutDiag::get_website_label()
{
    return m_button_website.get_label();
}


//
// Webサイトのボタンがクリックされた
//
void AboutDiag::slot_button_website_clicked()
{
    CORE::core_set_command( "open_url_browser", m_website_url );
}


//
// ライセンス
//
void AboutDiag::set_license( const Glib::ustring& license )
{
    m_textview_license.get_buffer()->set_text( license );
    m_textview_license.set_editable( false );
    m_textview_license.set_cursor_visible( false );
    m_textview_license.set_accepts_tab( false );
    m_textview_license.set_wrap_mode( Gtk::WRAP_WORD_CHAR );
    m_scrollwindow_license.set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC );
    m_scrollwindow_license.add( m_textview_license );
}

Glib::ustring AboutDiag::get_license()
{
    return m_textview_license.get_buffer()->get_text();
}


//
// 動作環境一覧
//
void AboutDiag::set_environment_list()
{
    Gtk::TreeModelColumn< Glib::ustring > column_name;
    Gtk::TreeModelColumn< Glib::ustring > column_value;
    Gtk::TreeModel::ColumnRecord record;
    Glib::RefPtr< Gtk::ListStore > liststore;

    record.add( column_name );
    record.add( column_value );
    liststore = Gtk::ListStore::create( record );
    m_treeview_environment.set_model( liststore );
    Gtk::TreeModel::Row row;

    m_treeview_environment.append_column( "項目", column_name );
    m_treeview_environment.append_column( "内容", column_value );

    row = *( liststore->append() );
    row[ column_name ] = "JDimのバージョン";
    row[ column_value ] = ENVIRONMENT::get_jdversion();

    row = *( liststore->append() );
    row[ column_name ] = "ディストリビューション";
    row[ column_value ] = ENVIRONMENT::get_distname();

    row = *( liststore->append() );
    row[ column_name ] = "デスクトップ環境等";
    row[ column_value ] = ENVIRONMENT::get_wm_str();

    row = *( liststore->append() );
    row[ column_name ] = "gtkmmのバージョン";
    row[ column_value ] = ENVIRONMENT::get_gtkmm_version();

    row = *( liststore->append() );
    row[ column_name ] = "glibmmのバージョン";
    row[ column_value ] = ENVIRONMENT::get_glibmm_version();

    row = *( liststore->append() );
    row[ column_name ] = "主なオプション";
    row[ column_value ] = ENVIRONMENT::get_configure_args( ENVIRONMENT::CONFIGURE_OMITTED );

    m_scrollwindow_environment.add( m_treeview_environment );
}


//
// 動作環境をクリップボードにコピー
//
void AboutDiag::slot_copy_environment()
{
    std::string jdinfo = ENVIRONMENT::get_jdinfo();

    if( ! jdinfo.empty() ) MISC::CopyClipboard( jdinfo );
}

