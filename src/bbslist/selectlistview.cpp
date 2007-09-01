// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "selectlistview.h"
#include "global.h"

using namespace BBSLIST;

SelectListView::SelectListView( const std::string& url, const std::string& arg1, const std::string& arg2)
    : BBSListViewBase( url, arg1, arg2 )
{
    // D&D可
    get_treeview().set_reorderable_view( true );

    // ツールバーからラベル削除
    get_toolbar().remove_label();
}


//
// ポップアップメニュー取得
//
// SKELETON::View::show_popupmenu() を参照すること
//
Gtk::Menu* SelectListView::get_popupmenu( const std::string& url )
{
    Gtk::Menu* popupmenu;
    if( url.empty() ) popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_favorite_space" ) );
    else{
        std::list< Gtk::TreeModel::iterator > list_it = get_treeview().get_selected_iterators();
        if( list_it.size() == 1 ){

            int type = path2type( *( get_treeview().get_selection()->get_selected_rows().begin() ) );

            if( type == TYPE_DIR ) popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_favorite_dir" ) );
            else if( type == TYPE_COMMENT ) popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_favorite_com" ) );
            else popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_select" ) );
        }
        else popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_favorite_mul" ) );
    }

    return popupmenu;
}


//
// 選択した行を開く 
//
const bool SelectListView::open_row( Gtk::TreePath& path, const bool tab )
{
    if( ! get_treeview().get_row( path ) ) return false;

    // ディレクトリの開け閉め
    if( path2type( path ) ==  TYPE_DIR ){

        if( ! get_treeview().row_expanded( path ) ) get_treeview().expand_row( path, false );
        else get_treeview().collapse_row( path );
    }

    return true;
}
