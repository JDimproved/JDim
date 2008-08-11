// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "favoriteview.h"
#include "toolbar.h"

#include "config/globalconf.h"

#include "cache.h"
#include "type.h"

// ルート要素名( bookmark.xml )
#define ROOT_NODE_NAME "favorite"

using namespace BBSLIST;


FavoriteListView::FavoriteListView( const std::string& url,
                                    const std::string& arg1, const std::string& arg2 )
    : BBSListViewBase( url, arg1, arg2 )
{
    set_label( "お気に入り" );

    // D&Dで編集可能
    set_editable( true );

    set_open_only_onedir( CONFIG::get_open_one_favorite() );
}
 

FavoriteListView::~FavoriteListView()
{
#ifdef _DEBUG    
    std::cout << "FavoriteList::~FavoriteList : " << get_url() << std::endl;
#endif 

    save_xml( false );
}


// xml保存
void FavoriteListView::save_xml( bool backup )
{
    std::string file = CACHE::path_xml_favorite();
    if( backup ) file = CACHE::path_xml_favorite_bkup();

    save_xml_impl( file, ROOT_NODE_NAME, "" );
}


//
// 表示
//
void FavoriteListView::show_view()
{
    std::string xml;

    // ファイルが存在しなければ入力を旧ファイル名にする
    std::string file_in = CACHE::path_xml_favorite();
    if( CACHE::file_exists( file_in ) != CACHE::EXIST_FILE )
    {
    	file_in = CACHE::path_xml_favorite_old();
    }

    CACHE::load_rawdata( file_in, xml );

    xml2tree( std::string( ROOT_NODE_NAME ), xml );

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
    if( url.empty() ) popupmenu = id2popupmenu(  "/popup_menu_favorite_space" );
    else{
        std::list< Gtk::TreeModel::iterator > list_it = get_treeview().get_selected_iterators();
        if( list_it.size() == 1 ){

            int type = path2type( *( get_treeview().get_selection()->get_selected_rows().begin() ) );

            if( type == TYPE_DIR ) popupmenu = id2popupmenu(  "/popup_menu_favorite_dir" );
            else if( type == TYPE_COMMENT ) popupmenu = id2popupmenu(  "/popup_menu_favorite_com" );
            else popupmenu = id2popupmenu(  "/popup_menu_favorite" );
        }
        else popupmenu = id2popupmenu(  "/popup_menu_favorite_mul" );
    }

    return popupmenu;
}
