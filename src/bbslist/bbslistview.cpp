// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "bbslistview.h"
#include "bbslistadmin.h"

#include "skeleton/msgdiag.h"

#include "dbtree/interface.h"

#include "config/globalconf.h"

#include "jdlib/misctime.h"

#include "cache.h"
#include "type.h"
#include "command.h"

using namespace BBSLIST;

// ルート要素名( boards.xml )
#define ROOT_NODE_NAME "boardlist"

// メインビュー

BBSListViewMain::BBSListViewMain( const std::string& url,
                                  const std::string& arg1, const std::string& arg2 )
    : BBSListViewBase( url, arg1, arg2 )
{
    set_label( "板一覧" );

    set_open_only_onedir( CONFIG::get_open_one_category() );
}



BBSListViewMain::~BBSListViewMain()
{
#ifdef _DEBUG
    std::cout << "BBSListViewMain::~BBSListViewMain : " << get_url() << std::endl;
#endif
}


// xml保存
void BBSListViewMain::save_xml()
{
    const std::string file = CACHE::path_xml_listmain();
    save_xml_impl( file, ROOT_NODE_NAME, SUBDIR_ETCLIST );
}


//
// 表示
//
void BBSListViewMain::show_view()
{
#ifdef _DEBUG
    std::cout << "BBSListViewMain::show_view : " << get_url() << std::endl;
#endif

    // BBSListViewBase::m_document に Root::m_xml_document を代入
    set_document( DBTREE::get_xml_document() );

    if( get_document().hasChildNodes() ) update_view();

    // 板一覧のDomノードが空ならサーバからbbsmenuを取得
    // 取得が終わったらBBSListViewMain::update_view()が呼び出される
    else DBTREE::download_bbsmenu();
}


//
// 表示更新
//
void BBSListViewMain::update_view()
{
    XML::Document document;
    document = DBTREE::get_xml_document();

    // 空なら更新しない
    if( ! document.hasChildNodes() ) return;

    // BBSListViewBase::m_document に Root::m_document を代入
    set_document( document );

    // ルート要素を取得
    XML::Dom* root = get_document().get_root_element( std::string( ROOT_NODE_NAME ) );

    //----------------------------------
    // 外部板追加

    // <subdir>を挿入
    // ルート要素の有無で処理を分ける( 旧様式=無, 新様式=有 )
    XML::Dom* subdir = nullptr;
    if( root ) subdir = root->emplace_front( XML::NODE_TYPE_ELEMENT, "subdir" );
    else subdir = get_document().emplace_front( XML::NODE_TYPE_ELEMENT, "subdir" );
    subdir->setAttribute( "name", std::string( SUBDIR_ETCLIST ) );

    // 子要素( <board> )を追加
    std::list< DBTREE::ETCBOARDINFO > list_etc = DBTREE::get_etcboards(); // 外部板情報( dbtree/etcboardinfo.h )
    if( ! list_etc.empty() )
    {
        std::list< DBTREE::ETCBOARDINFO >::iterator it = list_etc.begin();
        while( it != list_etc.end() )
        {
            XML::Dom* board = subdir->appendChild( XML::NODE_TYPE_ELEMENT, "board" );
            board->setAttribute( "name", (*it).name );
            board->setAttribute( "url", (*it).url );
            ++it;
        }
    }

    // 外部板追加ここまで
    //----------------------------------

    // BBSListViewBase::xml2tree() m_document -> tree
    xml2tree( std::string( ROOT_NODE_NAME ) );
    set_status( std::string() );
    BBSLIST::get_admin()->set_command( "set_status", get_url(), get_status() );
}


//
// 削除
//
// BBSListViewMainの場合は外部板の削除
//
void BBSListViewMain::delete_view()
{
    // 選択範囲に通常の板が含まれていないか確認
    std::list< Gtk::TreeModel::iterator > list_it = get_treeview().get_selected_iterators();
    std::list< Gtk::TreeModel::iterator >::iterator it = list_it.begin();
    for( ; it != list_it.end(); ++it ){
        if( ! is_etcboard( *it ) ){
            SKELETON::MsgDiag mdiag( get_parent_win(), "通常の板は削除出来ません", false, Gtk::MESSAGE_ERROR );
            mdiag.run();
            return;
        }
    }

    SKELETON::MsgDiag mdiag( get_parent_win(), "削除しますか？", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
    if( mdiag.run() != Gtk::RESPONSE_YES ) return;

    delete_view_impl();
}

// BBSListViewMainの場合は外部板の削除
void BBSListViewMain::delete_view_impl()
{
#ifdef _DEBUG
    std::cout << "BBSListViewMain::delete_view_impl\n";
#endif

    // データベースから外部板削除
    std::list< Gtk::TreeModel::iterator > list_it = get_treeview().get_selected_iterators();
    std::list< Gtk::TreeModel::iterator >::iterator it = list_it.begin();
    for( ; it != list_it.end(); ++it ){
        if( is_etcboard( *it ) ){
            Gtk::TreePath path = get_treestore()->get_path( *it );
            std::string url = path2rawurl( path );
            std::string name = path2name( path );

#ifdef _DEBUG
            std::cout << "path = " << path.to_string() << std::endl
                      << "url = " << url << std::endl
                      << "name = " << name << std::endl;
#endif
            DBTREE::remove_etc( url , name );
        }
    }

    const bool force = true; // 強制的に削除
    get_treeview().delete_selected_rows( force );

    // etc.txt保存
    DBTREE::save_etc();
}


void BBSListViewMain::show_preference()
{
    std::string modified = "最終更新日時 ：";

    if( DBTREE::get_date_modified().empty() ) modified +=  "未取得";
    else modified += 
             MISC::timettostr( DBTREE::get_time_modified(), MISC::TIME_WEEK )
             + " ( " + MISC::timettostr( DBTREE::get_time_modified(), MISC::TIME_PASSED ) + " )";

    SKELETON::MsgDiag mdiag( get_parent_win(), modified, false );
    mdiag.set_title( "板一覧のプロパティ" );
    mdiag.run();
}



//
// ポップアップメニュー取得
//
// SKELETON::View::show_popupmenu() を参照すること
//
Gtk::Menu* BBSListViewMain::get_popupmenu( const std::string& url )
{
    Gtk::Menu* popupmenu = nullptr;

    if( url.empty() ) return popupmenu;

#ifdef _DEBUG
    std::cout << "BBSListViewMain::get_popupmenu\n";
#endif

    std::list< Gtk::TreeModel::iterator > list_it = get_treeview().get_selected_iterators();
    if( list_it.size() == 1 ){

        Gtk::TreePath path = *( get_treeview().get_selection()->get_selected_rows().begin() );
        int type = path2type( path );

#ifdef _DEBUG
        std::cout << "path = " << path.to_string() << " type = " << type << std::endl;
#endif

        if( type == TYPE_DIR ){
            if( is_etcdir( path ) ) popupmenu = id2popupmenu(  "/popup_menu_etcdir" );
            else popupmenu = id2popupmenu(  "/popup_menu_dir" );
        }
        else if( type == TYPE_BOARD || type == TYPE_BOARD_UPDATE ){
            if( is_etcboard( path ) ) popupmenu = id2popupmenu(  "/popup_menu_etc" );
            else popupmenu = id2popupmenu(  "/popup_menu" );
        }
    }
    else{

        bool have_etc = true;

        std::list< Gtk::TreeModel::iterator >::iterator it = list_it.begin();
        for( ; it != list_it.end(); ++it ){
            if( ! is_etcboard( *it ) ){
                have_etc = false;
                break;
            }
        }

        if( have_etc ) popupmenu = id2popupmenu(  "/popup_menu_mul_etc" );
        else popupmenu = id2popupmenu(  "/popup_menu_mul" );
    }

    return popupmenu;
}
