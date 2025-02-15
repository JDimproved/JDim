// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "usrcmdpref.h"
#include "usrcmdmanager.h"

#include "command.h"
#include "dndmanager.h"
#include "environment.h"
#include "type.h"

#include "control/controlid.h"
#include "control/controlutil.h"

#include "skeleton/msgdiag.h"

#include "config/globalconf.h"

#include "jdlib/miscutil.h"

using namespace CORE;


UsrCmdDiag::UsrCmdDiag( Gtk::Window* parent, const Glib::ustring& name, const Glib::ustring& cmd )
    : SKELETON::PrefDiag( parent, "" ),
      m_label_name( "コマンド名", Gtk::ALIGN_START ),
      m_label_cmd( "実行するコマンド", Gtk::ALIGN_START ),
      m_button_manual( "オンラインマニュアルの置換文字一覧を表示" )
{
    resize( 640, 1 );

    m_entry_name.set_text( name );
    m_entry_cmd.set_text( cmd );

    m_button_manual.signal_clicked().connect( sigc::mem_fun( *this, &UsrCmdDiag::slot_show_manual ) );

    m_vbox.set_spacing( 8 );
    m_vbox.pack_start( m_label_name, Gtk::PACK_SHRINK );
    m_vbox.pack_start( m_entry_name, Gtk::PACK_SHRINK );

    m_hbox_cmd.pack_start( m_label_cmd, Gtk::PACK_SHRINK );
    m_hbox_cmd.pack_end( m_button_manual, Gtk::PACK_SHRINK );
    m_vbox.pack_start( m_hbox_cmd, Gtk::PACK_SHRINK );
    m_vbox.pack_start( m_entry_cmd, Gtk::PACK_SHRINK );

    get_content_area()->set_spacing( 8 );
    get_content_area()->pack_start( m_vbox );

    set_activate_entry( m_entry_name );
    set_activate_entry( m_entry_cmd );

    set_title( "ユーザコマンド設定" );
    show_all_children();
}


void UsrCmdDiag::slot_show_manual()
{
    CORE::core_set_command( "open_url_browser", ENVIRONMENT::get_jdhelpcmd() );
}

///////////////////////////////////////////


UsrCmdPref::UsrCmdPref( Gtk::Window* parent, const std::string& url )
    : SKELETON::PrefDiag( parent, url ),
      m_label( "右クリックしてコンテキストメニューからコマンドの追加と削除が出来ます。編集するにはダブルクリックします。" ),
      m_treeview( url, DNDTARGET_USRCMD, m_columns ),
      m_ckbt_hide_usrcmd( "選択不可のユーザコマンドを非表示にする", true )
{
    m_treestore = Gtk::TreeStore::create( m_columns );
    m_treeview.create_column( 0 );
    m_treeview.set_editable_view( true );

    m_treeview.set_size_request( 640, 400 );
    m_treeview.signal_row_activated().connect( sigc::mem_fun( *this, &UsrCmdPref::slot_row_activated ) );

    m_treeview.sig_button_press().connect( sigc::mem_fun(*this, &UsrCmdPref::slot_button_press ) );
    m_treeview.sig_button_release().connect( sigc::mem_fun(*this, &UsrCmdPref::slot_button_release ) );
    m_treeview.sig_key_press().connect( sigc::mem_fun(*this, &UsrCmdPref::slot_key_press ) );
    m_treeview.sig_key_release().connect( sigc::mem_fun(*this, &UsrCmdPref::slot_key_release ) );

    m_scrollwin.add( m_treeview );
    m_scrollwin.set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS );
    m_scrollwin.set_size_request( 640, 400 );

    get_content_area()->set_spacing( 8 );
    get_content_area()->pack_start( m_label, Gtk::PACK_SHRINK );
    get_content_area()->pack_start( m_scrollwin );

    m_ckbt_hide_usrcmd.set_active( CONFIG::get_hide_usrcmd() );
    get_content_area()->pack_start( m_ckbt_hide_usrcmd, Gtk::PACK_SHRINK );

    // ポップアップメニュー
    m_action_group = Gio::SimpleActionGroup::create();
    m_action_group->add_action( "NewCmd", sigc::mem_fun( *this, &UsrCmdPref::slot_newcmd ) );
    m_action_group->add_action( "NewDir", sigc::mem_fun( *this, &UsrCmdPref::slot_newdir ) );
    m_action_group->add_action( "NewSepa", sigc::mem_fun( *this, &UsrCmdPref::slot_newsepa ) );
    m_action_group->add_action( "Rename", sigc::mem_fun( *this, &UsrCmdPref::slot_rename ) );
    m_action_group->add_action( "Delete", sigc::mem_fun( *this, &UsrCmdPref::slot_delete ) );
    m_treeview.insert_action_group( "usrcmd", m_action_group );

    auto gmenu = Gio::Menu::create();
    gmenu->append( "新規コマンド(_C)", "usrcmd.NewCmd" );
    gmenu->append( "名前変更(_R)", "usrcmd.Rename" );
    gmenu->append( "新規ディレクトリ(_N)", "usrcmd.NewDir" );
    gmenu->append( "区切り(_S)", "usrcmd.NewSepa" );
    auto submenu = Gio::Menu::create();
    submenu->append( "削除する(_D)", "usrcmd.Delete" );
    gmenu->append_submenu( "削除(_D)", submenu );
    m_treeview_menu.bind_model( gmenu, true );
    m_treeview_menu.attach_to_widget( m_treeview );

    m_treeview.xml2tree( CORE::get_usrcmd_manager()->xml_document(), m_treestore, ROOT_NODE_NAME_USRCMD );

    show_all_children();

    set_title( "ユーザコマンド設定" );
}


void UsrCmdPref::timeout()
{
    m_treeview.clock_in();
}


void UsrCmdPref::slot_ok_clicked()
{
    CONFIG::set_hide_usrcmd( m_ckbt_hide_usrcmd.get_active() );

    m_treeview.tree2xml( CORE::get_usrcmd_manager()->xml_document(), ROOT_NODE_NAME_USRCMD );
    CORE::get_usrcmd_manager()->analyze_xml();
    CORE::get_usrcmd_manager()->save_xml();
    CORE::core_set_command( "reset_article_popupmenu" );
}


//
// マウスボタン押した
//
bool UsrCmdPref::slot_button_press( GdkEventButton* event )
{
#ifdef _DEBUG
    std::cout << "UsrCmdPref::slot_button_press\n";
#endif

    return true;
}


//
// マウスボタン離した
//
bool UsrCmdPref::slot_button_release( GdkEventButton* event )
{
    m_path_selected = m_treeview.get_path_under_xy( static_cast<int>(event->x), static_cast<int>(event->y) );

#ifdef _DEBUG
    std::cout << "UsrCmdPref::slot_button_release path = " << m_path_selected.to_string() << std::endl;
#endif

    if( m_control.button_alloted( event, CONTROL::PopupmenuButton ) ) show_popupmenu();

    // ディレクトリの開閉
    else if( ! m_path_selected.empty() && m_control.button_alloted( event, CONTROL::ClickButton ) ){

        Gtk::TreeModel::Row row = *( m_treestore->get_iter( m_path_selected ) );
        if( row && row[ m_columns.m_type ] == TYPE_DIR ){
            if( ! m_treeview.row_expanded( m_path_selected ) ) m_treeview.expand_row( m_path_selected, false );
            else m_treeview.collapse_row( m_path_selected );
        }
    }

    return true;
}


//
// キーを押した
//
bool UsrCmdPref::slot_key_press( GdkEventKey* event )
{
    // 行の名前を編集中なら何もしない
    if( m_treeview.is_renaming_row() ) return false;

    const int control = m_control.key_press( event );

#ifdef _DEBUG
    std::cout << "UsrCmdPref::slot_key_press key = " << control << std::endl;
#endif

    if( control == CONTROL::Delete ) delete_row();

    return true;
}


//
// キー上げた
//
bool UsrCmdPref::slot_key_release( GdkEventKey* event )
{
    // 行の名前を編集中なら何もしない
    if( m_treeview.is_renaming_row() ) return false;

#ifdef _DEBUG
    std::cout << "UsrCmdPref::slot_key_release\n";
#endif

    return true;
}


// ポップアップメニュー表示
void UsrCmdPref::show_popupmenu()
{
    std::list< Gtk::TreeModel::iterator > list_it = m_treeview.get_selected_iterators();
    const bool multi_selected{ list_it.size() > 1 };

    auto get_action = [this]( const char* name ) {
        return Glib::RefPtr<Gio::SimpleAction>::cast_dynamic( m_action_group->lookup_action( name ) );
    };
    get_action( "NewCmd" )->set_enabled( ! multi_selected );
    get_action( "NewDir" )->set_enabled( ! multi_selected );
    get_action( "NewSepa" )->set_enabled( ! multi_selected );

    auto act_rename = get_action( "Rename" );
    auto act_delete = get_action( "Delete" );
    if( m_path_selected.empty() ){
        act_rename->set_enabled( false );
        act_delete->set_enabled( false );
    }
    else{
        Gtk::TreeModel::Row row = *( m_treestore->get_iter( m_path_selected ) );
        int type = row[ m_columns.m_type ];

        act_rename->set_enabled( type != TYPE_SEPARATOR && ! multi_selected );
        act_delete->set_enabled( true );
    }

    // Specify the current event by nullptr.
    m_treeview_menu.popup_at_pointer( nullptr );
}


// コマンド作成
void UsrCmdPref::slot_newcmd()
{
    UsrCmdDiag diag( this, "", "" );
    if( diag.run() == Gtk::RESPONSE_OK ){

        if( diag.get_name().empty() || diag.get_cmd().empty() ) return;

        CORE::DATA_INFO_LIST list_info;
        CORE::DATA_INFO info;
        info.type = TYPE_USRCMD;
        info.parent = nullptr;
        info.url = std::string();
        info.name = diag.get_name();
        info.data = diag.get_cmd();
        if( m_path_selected.empty() ) info.path = Gtk::TreePath( "0" ).to_string();
        else info.path = m_path_selected.to_string();
        list_info.push_back( info );

        const bool subdir = true;
        const bool scroll = false;
        const bool force = false;
        const bool cancel_undo_commit = false;
        const int check_dup = 0; // 項目の重複チェックをしない
        m_treeview.append_info( list_info, m_path_selected, subdir, scroll, force, cancel_undo_commit, check_dup );
        m_path_selected = m_treeview.get_current_path();
    }
}


// ディレクトリ作成
void UsrCmdPref::slot_newdir()
{
    m_path_selected = m_treeview.create_newdir( m_path_selected );
}


// 区切り作成
void UsrCmdPref::slot_newsepa()
{
    CORE::DATA_INFO_LIST list_info;
    CORE::DATA_INFO info;
    info.type = TYPE_SEPARATOR;
    info.parent = nullptr;
    info.url = std::string();
    info.name = "--- 区切り ---";
    info.data = std::string();
    if( m_path_selected.empty() ) info.path = Gtk::TreePath( "0" ).to_string();
    else info.path = m_path_selected.to_string();
    list_info.push_back( info );

    const bool subdir = true;
    const bool scroll = false;
    const bool force = false;
    const bool cancel_undo_commit = false;
    const int check_dup = 0; // 項目の重複チェックをしない
    m_treeview.append_info( list_info, m_path_selected, subdir, scroll, force, cancel_undo_commit, check_dup );
    m_path_selected = m_treeview.get_current_path();
}


//
// 削除
//
void UsrCmdPref::delete_row()
{
    SKELETON::MsgDiag mdiag( nullptr, "削除しますか？", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
    if( mdiag.run() != Gtk::RESPONSE_YES ) return;

    slot_delete();
}

void UsrCmdPref::slot_delete()
{
    const bool force = false;
    m_treeview.delete_selected_rows( force );
}


// 名前変更
void UsrCmdPref::slot_rename()
{
    m_treeview.rename_row( m_path_selected );
}


void UsrCmdPref::slot_row_activated( const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column )
{
#ifdef _DEBUG
    std::cout << "UsrCmdPref::slot_row_activated path = " << path.to_string() << std::endl;
#endif

    Gtk::TreeModel::Row row = *( m_treestore->get_iter( path ) );
    if( ! row ) return;
    if( row[ m_columns.m_type ] != TYPE_USRCMD ) return;

    UsrCmdDiag diag( this, row[ m_columns.m_name ], row[ m_columns.m_data ] );
    if( diag.run() == Gtk::RESPONSE_OK ){
        row[ m_columns.m_name ] = diag.get_name();
        row[ m_columns.m_data ] = diag.get_cmd();
        static_cast<void>( row ); // cppcheck: unreadVariable
    }
}
