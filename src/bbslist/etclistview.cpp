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
    BBSLIST::get_admin()->set_command( "set_tablabel", get_url(), "Etc", "fix" );
}



//
// ポップアップメニュー
//
void EtcListView::show_popupmenu( const Gtk::TreePath& path )
{
    if( path.empty() ) return;

    Gtk::Menu* popupmenu;
    std::list< Gtk::TreeModel::iterator > list_it = get_treeview().get_selected_iterators();
    if( list_it.size() == 1 ) popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu" ) );
    else popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_mul" ) );

    if( popupmenu ) popupmenu->popup( 0, gtk_get_current_event_time() );
}
