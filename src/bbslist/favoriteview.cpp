// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "favoriteview.h"
#include "bbslistadmin.h"
#include "selectdialog.h"
#include "jdlib/misctime.h"

#include "cache.h"
#include "sharedbuffer.h"
#include "global.h"

// ルート要素名( fovorite.xml )
#define ROOT_NODE_NAME "favorite"

using namespace BBSLIST;


FavoriteListView::FavoriteListView( const std::string& url,
                                    const std::string& arg1, const std::string& arg2 )
    : BBSListViewBase( url, arg1, arg2 )
{
    // D&D可
    get_treeview().set_reorderable_view( true );
}


FavoriteListView::~FavoriteListView()
{
#ifdef _DEBUG    
    std::cout << "FavoriteList::~FavoriteList : " << get_url() << std::endl;
#endif 

    save_xml( CACHE::path_xml_favorite() );
}


void FavoriteListView::shutdown()
{
#ifdef _DEBUG    
    std::cout << "FavoriteList::shutdown\n";
#endif
    save_xml( CACHE::path_xml_favorite_bkup() );
}


//
// コマンド
//
bool FavoriteListView::set_command( const std::string& command, const std::string& arg )
{
    if( command == "append_favorite" ) append_favorite();
    if( command == "save_favorite" ) save_xml( CACHE::path_xml_favorite() );

    return true;
}



//
// 表示
//
void FavoriteListView::show_view()
{
    std::string xml;
    CACHE::load_rawdata( CACHE::path_xml_favorite(), xml );

    xml2tree( std::string( ROOT_NODE_NAME ), xml );

    // 旧様式のXMLならば別の名前で保存する
    if( ! xml.empty() && ! m_document.get_root_element( std::string( ROOT_NODE_NAME ) ) )
    {
    	// 別のファイル名
        const std::string file = CACHE::path_xml_favorite() + "." + MISC::get_sec_str();

        CACHE::save_rawdata( file, xml );
    }

    update_urls();
}


//
// 内容更新
//
void FavoriteListView::update_item( const std::string& )
{
    update_urls();
}



//
// ポップアップメニュー取得
//
// SKELETON::View::show_popupmenu() を参照すること
//
Gtk::Menu* FavoriteListView::get_popupmenu( const std::string& url )
{
    Gtk::Menu* popupmenu;
    if( url.empty() ) popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_favorite_space" ) );
    else{
        std::list< Gtk::TreeModel::iterator > list_it = get_treeview().get_selected_iterators();
        if( list_it.size() == 1 ){

            int type = path2type( *( get_treeview().get_selection()->get_selected_rows().begin() ) );

            if( type == TYPE_DIR ) popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_favorite_dir" ) );
            else if( type == TYPE_COMMENT ) popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_favorite_com" ) );
            else popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_favorite" ) );
        }
        else popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_favorite_mul" ) );
    }

    return popupmenu;
}


//
// お気に入りにアイテム追加
//
// あらかじめ共有バッファにデータを入れておくこと
//
void  FavoriteListView::append_favorite()
{
    if( CORE::SBUF_size() == 0 ) return;
    
    SelectListDialog diag( get_url(), get_treestore() );
    if( diag.run() != Gtk::RESPONSE_OK ) return;
    append_from_buffer( diag.get_path(), true, true );
}


//
// お気に入り保存
//
void FavoriteListView::save_xml( const std::string& file )
{
    if( get_ready_tree() )
    {
        tree2xml( std::string( ROOT_NODE_NAME ) );

        CACHE::save_rawdata( file, m_document.get_xml() );
    }
}

