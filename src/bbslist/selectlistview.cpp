// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "selectlistview.h"

using namespace BBSLIST;

SelectListView::SelectListView( const std::string& url, const std::string& arg1, const std::string& arg2)
    : BBSListViewBase( url, arg1, arg2 )
{
    // D&D可
    get_treeview().set_reorderable_view( true );
}


//
// ポップアップメニュー
//
void SelectListView::show_popupmenu( const Gtk::TreePath& path )
{
    Gtk::Menu* popupmenu;

    if( path.empty() ) popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_favorite_space" ) );
    else{
        std::list< Gtk::TreeModel::iterator > list_it = get_treeview().get_selected_iterators();
        if( list_it.size() == 1 ) popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_select" ) );
        else popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_favorite_mul" ) );
    }

    if( popupmenu ) popupmenu->popup( 0, gtk_get_current_event_time() );
}
