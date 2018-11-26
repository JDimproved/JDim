// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "selectlistview.h"
#include "toolbar.h"

#include "control/controlid.h"

#include "type.h"

using namespace BBSLIST;

SelectListView::SelectListView( const std::string& url, const std::string& arg1, const std::string& arg2)
    : BBSListViewBase( url, arg1, arg2 )
{
    // D&Dで編集可能
    set_editable( true );
}


void SelectListView::close_view()
{
#ifdef _DEBUG
    std::cout << "SelectListView::close_view\n";
#endif

    // ダイアログを閉じる
    m_sig_close_dialog.emit();
}


bool SelectListView::operate_view( const int control )
{
    bool ret = true;

    // ESCでダイアログを閉じる
    if( control == CONTROL::Cancel ){
        close_view();
    }
    else if( control == CONTROL::Search ){
        set_search_invert( false );
        m_sig_focus_entry_search.emit();
    }
    else if( control == CONTROL::SearchInvert ){
        set_search_invert( true );
        m_sig_focus_entry_search.emit();
    }
    else ret = BBSListViewBase::operate_view( control );

    return ret;
}


//
// ポップアップメニュー取得
//
// SKELETON::View::show_popupmenu() を参照すること
//
Gtk::Menu* SelectListView::get_popupmenu( const std::string& url )
{
    Gtk::Menu* popupmenu;
    if( url.empty() ) popupmenu = id2popupmenu(  "/popup_menu_favorite_space" );
    else{
        std::list< Gtk::TreeModel::iterator > list_it = get_treeview().get_selected_iterators();
        if( list_it.size() == 1 ){

            int type = path2type( *( get_treeview().get_selection()->get_selected_rows().begin() ) );

            if( type == TYPE_DIR ) popupmenu = id2popupmenu(  "/popup_menu_favorite_dir" );
            else if( type == TYPE_COMMENT ) popupmenu = id2popupmenu(  "/popup_menu_favorite_com" );
            else popupmenu = id2popupmenu(  "/popup_menu_select" );
        }
        else popupmenu = id2popupmenu(  "/popup_menu_favorite_mul" );
    }

    return popupmenu;
}


//
// 選択した行を開く 
//
bool SelectListView::open_row( Gtk::TreePath& path, const bool tab )
{
    if( ! get_treeview().get_row( path ) ) return false;

    // ディレクトリの開け閉め
    if( path2type( path ) ==  TYPE_DIR ){

        if( ! get_treeview().row_expanded( path ) ) get_treeview().expand_row( path, false );
        else get_treeview().collapse_row( path );
    }

    return true;
}
