// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"
#include "gtkmmversion.h"

#include "usrcmdpref.h"
#include "usrcmdmanager.h"
#include "command.h"
#include "type.h"
#include "jdversion.h"
#include "dndmanager.h"

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

    get_vbox()->set_spacing( 8 );
    get_vbox()->pack_start( m_vbox );

    set_activate_entry( m_entry_name );
    set_activate_entry( m_entry_cmd );

    set_title( "ユーザコマンド設定" );
    show_all_children();
}


void UsrCmdDiag::slot_show_manual()
{
    CORE::core_set_command( "open_url_browser", JDHELPCMD );
}

///////////////////////////////////////////


UsrCmdPref::UsrCmdPref( Gtk::Window* parent, const std::string& url )
    : SKELETON::PrefDiag( parent, url ),
      m_label( "右クリックしてコンテキストメニューからコマンドの追加と削除が出来ます。編集するにはダブルクリックします。" ),
      m_treeview( url, DNDTARGET_USRCMD, m_columns ),
      m_ckbt_hide_usrcmd( "選択不可のユーザコマンドを非表示にする", true )
{
    m_treestore = Gtk::TreeStore::create( m_columns );
#if !GTKMM_CHECK_VERSION(2,7,0)
    // gtkmm26以下にはunset_model()が無いのでここでset_model()しておく
    // それ以上は m_treeview.xml2tree() でセットする
    m_treeview.set_treestore( m_treestore );
#endif
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

    get_vbox()->set_spacing( 8 );
    get_vbox()->pack_start( m_label );
    get_vbox()->pack_start( m_scrollwin );

#if !GTKMM_CHECK_VERSION(2,7,0)
    m_ckbt_hide_usrcmd.set_active( CONFIG::get_hide_usrcmd() );
    get_vbox()->pack_start( m_ckbt_hide_usrcmd, Gtk::PACK_SHRINK );
#endif

    // ポップアップメニュー
    m_action_group = Gtk::ActionGroup::create();
    m_action_group->add( Gtk::Action::create( "NewCmd", "新規コマンド(_C)"), sigc::mem_fun( *this, &UsrCmdPref::slot_newcmd ) );
    m_action_group->add( Gtk::Action::create( "NewDir", "新規ディレクトリ(_N)"), sigc::mem_fun( *this, &UsrCmdPref::slot_newdir ) );
    m_action_group->add( Gtk::Action::create( "NewSepa", "区切り(_S)"), sigc::mem_fun( *this, &UsrCmdPref::slot_newsepa ) );
    m_action_group->add( Gtk::Action::create( "Rename", "名前変更(_R)"), sigc::mem_fun( *this, &UsrCmdPref::slot_rename ) );
    m_action_group->add( Gtk::Action::create( "Delete_Menu", "Delete" ) );
    m_action_group->add( Gtk::Action::create( "Delete", "削除する(_D)"), sigc::mem_fun( *this, &UsrCmdPref::slot_delete ) );

    Glib::ustring str_ui =
    "<ui>"

    "<popup name='popup_menu'>"
    "<menuitem action='NewCmd'/>"
    "<separator/>"
    "<menuitem action='Rename'/>"
    "<menuitem action='NewDir'/>"
    "<menuitem action='NewSepa'/>"
    "<separator/>"
    "<menu action='Delete_Menu'>"
    "<menuitem action='Delete'/>"
    "</menu>"
    "</popup>"

    // 複数選択
    "<popup name='popup_menu_mul'>"
    "<menu action='Delete_Menu'>"
    "<menuitem action='Delete'/>"
    "</menu>"
    "</popup>"

    "</ui>";

    m_ui_manager = Gtk::UIManager::create();
    m_ui_manager->insert_action_group( m_action_group );
    m_ui_manager->add_ui_from_string( str_ui );

    // ポップアップメニューにキーアクセレータを表示
    Gtk::Menu* menu = dynamic_cast< Gtk::Menu* >( m_ui_manager->get_widget( "/popup_menu" ) );
    CONTROL::set_menu_motion( menu );
    menu = dynamic_cast< Gtk::Menu* >( m_ui_manager->get_widget( "/popup_menu_mul" ) );
    CONTROL::set_menu_motion( menu );

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
    m_path_selected = m_treeview.get_path_under_xy( (int)event->x, (int)event->y );

#ifdef _DEBUG
    std::cout << "UsrCmdPref::slot_button_release path = " << m_path_selected.to_string() << std::endl;;
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
    Gtk::Menu* menu = NULL;
    std::list< Gtk::TreeModel::iterator > list_it = m_treeview.get_selected_iterators();
    if( list_it.size() <= 1 ) menu = dynamic_cast< Gtk::Menu* >( m_ui_manager->get_widget( "/popup_menu" ) );
    else menu = dynamic_cast< Gtk::Menu* >( m_ui_manager->get_widget( "/popup_menu_mul" ) );

    if( ! menu ) return;

    Glib::RefPtr< Gtk::Action > act_del, act_rename;
    act_rename = m_action_group->get_action( "Rename" );
    act_del = m_action_group->get_action( "Delete_Menu" );

    if( m_path_selected.empty() ){
        if( act_rename ) act_rename->set_sensitive( false );
        if( act_del ) act_del->set_sensitive( false );
    }
    else{

        Gtk::TreeModel::Row row = *( m_treestore->get_iter( m_path_selected ) );
        int type = row[ m_columns.m_type ];

        if( act_rename ){
            if( type != TYPE_SEPARATOR ) act_rename->set_sensitive( true );
            else act_rename->set_sensitive( false );
        }
        if( act_del ) act_del->set_sensitive( true );
    }

    menu->popup( 0, gtk_get_current_event_time() );
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
        info.parent = NULL;
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
    info.parent = NULL;
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
    SKELETON::MsgDiag mdiag( NULL, "削除しますか？", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
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
    }
}
