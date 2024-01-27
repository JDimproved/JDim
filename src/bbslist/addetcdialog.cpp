// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "addetcdialog.h"

#include "command.h"
#include "session.h"


using namespace BBSLIST;


AddEtcDialog::AddEtcDialog( const bool move, const std::string& url, const std::string& name, const std::string& id, const std::string& passwd )
    : SKELETON::PrefDiag( nullptr, url, true )
    , m_label_name{ "板名(_N):", true }
    , m_label_url{ "アドレス(_U):", true }
    , m_label_id{ "ID(_I):", true }
    , m_label_pw{ "パスワード(_P):", true }
{
    set_default_size( 600, -1 );

    m_label_name.set_alignment( Gtk::ALIGN_START );
    m_label_url.set_alignment( Gtk::ALIGN_START );
    m_label_id.set_alignment( Gtk::ALIGN_START );
    m_label_pw.set_alignment( Gtk::ALIGN_START );

    m_label_name.set_mnemonic_widget( m_entry_name );
    m_label_url.set_mnemonic_widget( m_entry_url );
    m_label_id.set_mnemonic_widget( m_entry_id );
    m_label_pw.set_mnemonic_widget( m_entry_pw );

    m_entry_name.set_hexpand( true );
    m_entry_url.set_hexpand( true );
    m_entry_id.set_hexpand( true );
    m_entry_pw.set_hexpand( true );

    m_entry_name.set_text( name );
    m_entry_url.set_text( url );
    m_entry_id.set_text( id );
    m_entry_pw.set_text( passwd );

    set_activate_entry( m_entry_name );
    set_activate_entry( m_entry_url );
    set_activate_entry( m_entry_id );
    set_activate_entry( m_entry_pw );

    m_grid.set_column_spacing( 10 );
    m_grid.set_row_spacing( 8 );
    // attach( child, x, y, width, height )
    m_grid.attach( m_label_name, 0, 0, 1, 1 );
    m_grid.attach( m_entry_name, 1, 0, 1, 1 );
    m_grid.attach( m_label_url, 0, 1, 1, 1 );
    m_grid.attach( m_entry_url, 1, 1, 1, 1 );

    m_grid_auth.property_margin() = 8;
    m_grid_auth.set_column_spacing( 10 );
    m_grid_auth.set_row_spacing( 8 );
    m_grid_auth.attach( m_label_id, 0, 0, 1, 1 );
    m_grid_auth.attach( m_entry_id, 1, 0, 1, 1 );
    m_grid_auth.attach( m_label_pw, 0, 1, 1, 1 );
    m_grid_auth.attach( m_entry_pw, 1, 1, 1, 1 );

    m_frame.set_label( "BASIC認証" );
    m_frame.add( m_grid_auth );

    get_content_area()->set_spacing( 8 );
    get_content_area()->property_margin() = 8;
    get_content_area()->pack_start( m_grid );
    get_content_area()->pack_start( m_frame );

    if( move ){
        set_title( "外部板編集" );
    }
    else{
        set_title( "外部板追加" );
    }

    show_all_children();
}


AddEtcDialog::~AddEtcDialog() noexcept = default;


/** @brief コンストラクタ
 *
 * @param[in] parent 親ウインドウ (nullable)
 * @param[in] edit   true なら編集モード
 * @param[in] url    BBSMENUのURLの初期値
 * @param[in] name   BBSMENU名前の初期値
 */
AddEtcBBSMenuDialog::AddEtcBBSMenuDialog( Gtk::Window* parent, const bool edit,
                                          const Glib::ustring& url, const Glib::ustring& name )
    : SKELETON::PrefDiag{ parent, url, true }
    , m_label_supplement{ Glib::ustring{ edit ? "「OK」を押すとBBSMENUをダウンロードして更新します。"
                                              : "「OK」を押すとBBSMENUをダウンロードして板一覧の下部に追加します。" }
                          + "\nこの機能は実験的なサポートのため変更または廃止の可能性があります。" }
    , m_label_name{ "BBSMENU名(_N):", true }
    , m_label_url{ "アドレス(_U):", true }
{
    set_default_size( 600, -1 );

    m_label_name.set_alignment( Gtk::ALIGN_START );
    m_label_url.set_alignment( Gtk::ALIGN_START );
    m_label_name.set_mnemonic_widget( m_entry_name );
    m_label_url.set_mnemonic_widget( m_entry_url );

    m_entry_name.set_hexpand( true );
    m_entry_url.set_hexpand( true );
    m_entry_name.set_text( name );
    m_entry_url.set_text( url );

    m_grid.set_column_spacing( 10 );
    m_grid.set_row_spacing( 8 );
    m_grid.attach( m_label_name, 0, 0, 1, 1 );
    m_grid.attach( m_entry_name, 1, 0, 1, 1 );
    m_grid.attach( m_label_url, 0, 1, 1, 1 );
    m_grid.attach( m_entry_url, 1, 1, 1, 1 );

    get_content_area()->set_spacing( 8 );
    get_content_area()->property_margin() = 8;
    get_content_area()->pack_start( m_label_supplement );
    get_content_area()->pack_start( m_grid );

    set_activate_entry( m_entry_name );
    set_activate_entry( m_entry_url );

    set_title( edit ? "外部BBSMENU編集" : "外部BBSMENU追加" );

    show_all_children();
}
