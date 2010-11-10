// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "historyview.h"
#include "toolbar.h"

#include "config/globalconf.h"

#include "cache.h"
#include "type.h"

// ルート要素名
#define ROOT_NODE_NAME "history"

using namespace BBSLIST;


HistoryViewBase::HistoryViewBase( const std::string& url, const std::string& file_xml,
                          const std::string& arg1, const std::string& arg2 )
    : BBSListViewBase( url, arg1, arg2 ), m_file_xml( file_xml )
{}
 

HistoryViewBase::~HistoryViewBase()
{
#ifdef _DEBUG    
    std::cout << "HistoryViewBase::~HistoryViewBase : " << get_url() << std::endl;
#endif 
}


//
// 表示
//
void HistoryViewBase::show_view()
{
    std::string xml;
    CACHE::load_rawdata( m_file_xml, xml );
    xml2tree( std::string( ROOT_NODE_NAME ), xml );
    update_urls();
}


// xml保存
void HistoryViewBase::save_xml()
{
    save_xml_impl( m_file_xml, ROOT_NODE_NAME, "" );
}


//
// ポップアップメニュー取得
//
// SKELETON::View::show_popupmenu() を参照すること
//
Gtk::Menu* HistoryViewBase::get_popupmenu( const std::string& url )
{
    Gtk::Menu* popupmenu = NULL;
    if( ! url.empty() ){

        std::list< Gtk::TreeModel::iterator > list_it = get_treeview().get_selected_iterators();
        if( list_it.size() == 1 ){

            const int type = path2type( *( get_treeview().get_selection()->get_selected_rows().begin() ) );

            if( type == TYPE_VBOARD ) popupmenu = id2popupmenu( "/popup_menu_history_vboard" );
            else popupmenu = id2popupmenu( "/popup_menu_history" );
        }
        else popupmenu = id2popupmenu(  "/popup_menu_history_mul" );
    }

    return popupmenu;
}


//////////////////////////////////



HistoryThreadView::HistoryThreadView( const std::string& url, const std::string& arg1, const std::string& arg2 )
    : HistoryViewBase( url, CACHE::path_xml_history(), arg1, arg2 )
{
    set_label( "スレ履歴" );
}


HistoryCloseView::HistoryCloseView( const std::string& url, const std::string& arg1, const std::string& arg2 )
    : HistoryViewBase( url, CACHE::path_xml_history_close(), arg1, arg2 )
{
    set_label( "最近閉じたスレ" );
}

HistoryBoardView::HistoryBoardView( const std::string& url, const std::string& arg1, const std::string& arg2 )
    : HistoryViewBase( url, CACHE::path_xml_history_board(), arg1, arg2 )
{
    set_label( "板履歴" );
}
