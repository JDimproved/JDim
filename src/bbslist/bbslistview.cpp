// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "bbslistview.h"
#include "bbslistadmin.h"
#include "toolbar.h"

#include "skeleton/msgdiag.h"

#include "dbtree/interface.h"

#include "cache.h"
#include "global.h"
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

    set_expand_collapse( true );
}



BBSListViewMain::~BBSListViewMain()
{
#ifdef _DEBUG    
    std::cout << "BBSListViewMain::~BBSListViewMain : " << get_url() << std::endl;
#endif

    save_xml( false );
}


// xml保存
void BBSListViewMain::save_xml( bool backup )
{
    std::string file = CACHE::path_xml_listmain();
    if( backup ) file = CACHE::path_xml_listmain_bkup();

    save_xml_impl( file , ROOT_NODE_NAME, SUBDIR_ETCLIST );
}


//
// 表示
//
void BBSListViewMain::show_view()
{
#ifdef _DEBUG
    std::cout << "BBSListViewMain::show_view : " << get_url() << std::endl;
#endif    

    // BBSListViewBase::m_document に Root::m_document を代入
    m_document = DBTREE::get_xml_document();

    // 板一覧のDomノードが空ならサーバから取得
    // 更新が終わったらBBSListViewMain::update_view()が呼ばれる
    if( ! m_document.hasChildNodes() )
    {
        DBTREE::download_bbsmenu();
        set_status( "loading..." );
        BBSLIST::get_admin()->set_command( "set_status", get_url(), get_status() );
    }

    else update_view();
}


//
// 表示更新
//
void BBSListViewMain::update_view()
{
    // BBSListViewBase::m_document に Root::m_document を代入
    m_document = DBTREE::get_xml_document();

    // ルート要素を取得
    XML::Dom* root = m_document.get_root_element( std::string( ROOT_NODE_NAME ) );

    //----------------------------------
    // 外部板追加

    // <subdir>を挿入
    // ルート要素の有無で処理を分ける( 旧様式=無, 新様式=有 )
    XML::Dom* subdir = 0;
    if( root ) subdir = root->insertBefore( XML::NODE_TYPE_ELEMENT, "subdir", root->firstChild() );
    else subdir = m_document.insertBefore( XML::NODE_TYPE_ELEMENT, "subdir", m_document.firstChild() );
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
            SKELETON::MsgDiag mdiag( NULL, "通常の板は削除出来ません", false, Gtk::MESSAGE_ERROR );
            mdiag.run();
            return;
        }
    }

    SKELETON::MsgDiag mdiag( NULL, "削除しますか？", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
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

    delete_selected_rows();

    // etc.txt保存
    DBTREE::save_etc();
}


//
// ポップアップメニュー取得
//
// SKELETON::View::show_popupmenu() を参照すること
//
Gtk::Menu* BBSListViewMain::get_popupmenu( const std::string& url )
{
    Gtk::Menu* popupmenu = NULL;

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
        else if( type == TYPE_BOARD ){
            if( is_etcboard( path ) ) popupmenu = id2popupmenu(  "/popup_menu_etc" );
            else popupmenu = id2popupmenu(  "/popup_menu" );
        }
    }
    else popupmenu = id2popupmenu(  "/popup_menu_mul" );

    return popupmenu;
}
