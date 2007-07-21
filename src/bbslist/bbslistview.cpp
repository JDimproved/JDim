// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "bbslistview.h"
#include "bbslistadmin.h"

#include "dbtree/interface.h"

#include "cache.h"
#include "global.h"

using namespace BBSLIST;

#define SUBDIR_ETCLIST "外部板"

// ルート要素名( list_main.xml )
#define ROOT_NODE_NAME "boardlist"

// メインビュー

BBSListViewMain::BBSListViewMain( const std::string& url,
                                  const std::string& arg1, const std::string& arg2 )
    : BBSListViewBase( url, arg1, arg2 ), m_load_etc( false )
{
    set_expand_collapse( true );

    get_toolbar().set_combo( COMBO_BBSLIST );
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

    save_xml_impl( file , ROOT_NODE_NAME, ( m_load_etc ? SUBDIR_ETCLIST : "" ) );
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
    if( ! m_document.hasChildNodes() ) m_document = DBTREE::get_xml_document();

    // 外部板のペア( 名前, URL )を取得
    m_load_etc = false;
    std::map< std::string, std::string > list_etc = DBTREE::get_xml_etc();

    // ルート要素を取得
    XML::Dom* root = m_document.get_root_element( std::string( ROOT_NODE_NAME ) );

    // 外部板挿入
    if( ! list_etc.empty() )
    {
        m_load_etc = true;

        // <subdir>を挿入
        XML::Dom* subdir = 0;

        // ルート要素の有無で処理を分ける( 旧様式=無, 新様式=有 )
        if( root ) subdir = root->insertBefore( XML::NODE_TYPE_ELEMENT, "subdir", root->firstChild() );
        else subdir = m_document.insertBefore( XML::NODE_TYPE_ELEMENT, "subdir", m_document.firstChild() );

        subdir->setAttribute( "name", std::string( SUBDIR_ETCLIST ) );

        // 子要素( <board> )を追加
        std::map< std::string, std::string >::iterator it = list_etc.begin();
        while( it != list_etc.end() )
        {
            XML::Dom* board = subdir->appendChild( XML::NODE_TYPE_ELEMENT, "board" );
            board->setAttribute( "name", (*it).first );
            board->setAttribute( "url", (*it).second );

            ++it;
        }
    }

    // BBSListViewBase::xml2tree() m_document -> tree
    xml2tree( std::string( ROOT_NODE_NAME ) );
    set_status( std::string() );
    BBSLIST::get_admin()->set_command( "set_status", get_url(), get_status() );
}


//
// ポップアップメニュー取得
//
// SKELETON::View::show_popupmenu() を参照すること
//
Gtk::Menu* BBSListViewMain::get_popupmenu( const std::string& url )
{
    if( url.empty() ) return NULL;

    Gtk::Menu* popupmenu;
    std::list< Gtk::TreeModel::iterator > list_it = get_treeview().get_selected_iterators();
    if( list_it.size() == 1 ){
            int type = path2type( *( get_treeview().get_selection()->get_selected_rows().begin() ) );

            if( type == TYPE_DIR ) popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_dir" ) );
            else popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu" ) );
    }
    else popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_mul" ) );

    return popupmenu;
}
