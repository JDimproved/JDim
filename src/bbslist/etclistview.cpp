// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "etclistview.h"
#include "bbslistadmin.h"

#include "dbtree/interface.h"

using namespace BBSLIST;


EtcListView::EtcListView( const std::string& url,
                                    const std::string& arg1, const std::string& arg2 )
    : BBSListViewBase( url, arg1, arg2 )
{}


EtcListView::~EtcListView()
{
#ifdef _DEBUG    
    std::cout << "EtcListView::~EtcListView : " << get_url() << std::endl;
#endif 
}



//
// 表示
//
void EtcListView::show_view()
{
    xml2tree( DBTREE::get_xml_etc() );
    BBSLIST::get_admin()->set_command( "set_tablabel", get_url(), "外部板" );
}



//
// ポップアップメニュー取得
//
// SKELETON::View::show_popupmenu() を参照すること
//
Gtk::Menu* EtcListView::get_popupmenu( const std::string& url )
{
    if( url.empty() ) return NULL;

    Gtk::Menu* popupmenu;
    std::list< Gtk::TreeModel::iterator > list_it = get_treeview().get_selected_iterators();
    if( list_it.size() == 1 ) popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu" ) );
    else popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_mul" ) );

    return popupmenu;
}
