// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "historyview.h"
#include "toolbar.h"

#include "config/globalconf.h"

#include "cache.h"
#include "type.h"
#include "global.h"

namespace BBSLIST::hv {
/// ルート要素名
constexpr const char* kRootNodeName = "history";
}


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
    xml2tree( std::string( hv::kRootNodeName ), xml );
    update_urls();
}


// xml保存
void HistoryViewBase::save_xml()
{
    save_xml_impl( m_file_xml, hv::kRootNodeName, "" );
}


//
// ポップアップメニュー取得
//
// SKELETON::View::show_popupmenu() を参照すること
//
Gtk::Menu* HistoryViewBase::get_popupmenu( const std::string& url )
{
    Gtk::Menu* popupmenu = nullptr;
    if( ! url.empty() ){

        std::list< Gtk::TreeModel::iterator > list_it = get_treeview().get_selected_iterators();
        if( list_it.size() == 1 ){

            const int type = path2type( *( get_treeview().get_selection()->get_selected_rows().begin() ) );

            if( type == TYPE_VBOARD ) popupmenu = id2popupmenu( "popup_menu_history_vboard" );
            else popupmenu = id2popupmenu( "popup_menu_history" );
        }
        else popupmenu = id2popupmenu(  "popup_menu_history_mul" );
    }

    return popupmenu;
}


//////////////////////////////////



HistoryThreadView::HistoryThreadView( const std::string& url, const std::string& arg1, const std::string& arg2 )
    : HistoryViewBase( url, CACHE::path_xml_history(), arg1, arg2 )
{
    set_label( ITEM_NAME_HISTVIEW );
}


HistoryCloseView::HistoryCloseView( const std::string& url, const std::string& arg1, const std::string& arg2 )
    : HistoryViewBase( url, CACHE::path_xml_history_close(), arg1, arg2 )
{
    set_label( ITEM_NAME_HIST_CLOSEVIEW );
}

HistoryBoardView::HistoryBoardView( const std::string& url, const std::string& arg1, const std::string& arg2 )
    : HistoryViewBase( url, CACHE::path_xml_history_board(), arg1, arg2 )
{
    set_label( ITEM_NAME_HIST_BOARDVIEW );
}

HistoryCloseBoardView::HistoryCloseBoardView( const std::string& url, const std::string& arg1, const std::string& arg2 )
    : HistoryViewBase( url, CACHE::path_xml_history_closeboard(), arg1, arg2 )
{
    set_label( ITEM_NAME_HIST_CLOSEBOARDVIEW );
}

HistoryCloseImgView::HistoryCloseImgView( const std::string& url, const std::string& arg1, const std::string& arg2 )
    : HistoryViewBase( url, CACHE::path_xml_history_closeimg(), arg1, arg2 )
{
    set_label( ITEM_NAME_HIST_CLOSEIMGVIEW );
}
