// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "historysubmenu.h"
#include "global.h"
#include "command.h"
#include "cache.h"
#include "xml.h"

#include "dbtree/interface.h"

#include "jdlib/miscutil.h"

#include "config/globalconf.h"

#include <sstream>
#include <list>

using namespace CORE;

#define HIST_NONAME "--------------"

// 履歴に表示する文字数(半角)
#define HIST_MAX_LNG 50

HistorySubMenu::HistorySubMenu( const std::string path_xml )
    : Gtk::Menu(),
      m_path_xml( path_xml )
{
    // メニュー項目作成
    for( int i = 0; i < CONFIG::get_history_size(); ++i ){
        Gtk::MenuItem* item = Gtk::manage( new Gtk::MenuItem( HIST_NONAME ) );
        m_itemlist.push_back( item );
        append( *item );
        item->signal_activate().connect( sigc::bind<int>( sigc::mem_fun( *this, &HistorySubMenu::slot_activate ), i ));

        CORE::HIST_ITEM* histitem = new CORE::HIST_ITEM;
        histitem->type = TYPE_UNKNOWN;
        m_histlist.push_back( histitem );
    }

    std::string xml;
    CACHE::load_rawdata( m_path_xml, xml );
    xml2list( xml );
}



HistorySubMenu::~HistorySubMenu()
{
#ifdef _DEBUG
    std::cout << "HistorySubMenu::~HistorySubMenu\n";
#endif

    // XML保存
    CACHE::save_rawdata( m_path_xml, list2xml() );

    std::list< CORE::HIST_ITEM* >::iterator it = m_histlist.begin();
    for(; it != m_histlist.end(); ++it ) delete ( *it );
}


// 履歴のクリア
void HistorySubMenu::clear()
{
    std::list< CORE::HIST_ITEM* >::iterator it = m_histlist.begin();
    for(; it != m_histlist.end(); ++it ){
        (*it)->url = std::string();
        (*it)->name = std::string();
        (*it)->type = TYPE_UNKNOWN;
    }

    std::list< Gtk::MenuItem* >::iterator it_item = m_itemlist.begin();
    for(; it_item != m_itemlist.end(); ++it_item ){
        dynamic_cast< Gtk::Label* >( (*it_item)->get_child() )->set_text( HIST_NONAME );
    }
}


void HistorySubMenu::append_item( const std::string& url, const std::string& name, int type )
{
    std::list< CORE::HIST_ITEM* >::iterator it;
    CORE::HIST_ITEM* item = NULL;

    it = m_histlist.begin();
    for(; it != m_histlist.end(); ++it ){

        item = (*it);
        assert( item );

        // 同じURLがあったら先頭に持ってくる
        if( item->type == type && item->url == url ){
            m_histlist.remove( item );
            m_histlist.push_front( item );
            return;
        }

        // emptyを見付けたら値をセットして先頭に持ってくる
        if( item->url.empty() ){
            item->url = url;
            item->name = name;
            item->type = type;
            m_histlist.remove( item );
            m_histlist.push_front( item );
            return;
        }
    }

    // 一番最後のitemを削除して先頭に持ってくる
    assert( item );
    item->url = url;
    item->name = name;
    item->type = type;
    m_histlist.remove( item );
    m_histlist.push_front( item );
}



//
// XML->list 変換
//
void HistorySubMenu::xml2list( const std::string& xml )
{
#ifdef _DEBUG
    std::cout << "HistorySubMenu::xml2list\n";
    std::cout << xml << std::endl;
#endif

    std::list< std::string > lines = MISC::get_lines( xml );
    if( lines.empty() ) return;

    std::list< CORE::HIST_ITEM* >::iterator it_hist = m_histlist.begin();
    std::list< std::string >::iterator it = lines.begin();
    for( ; it != lines.end(); ++it ){

        std::string url;
        std::string name;
        std::string& line = *( it );

        int type = XML::get_type( line, url, name );
        if( type != TYPE_UNKNOWN && !url.empty() ){
            ( *it_hist )->url = url;
            ( *it_hist )->name = name;
            ( *it_hist )->type = type;
            ++it_hist;
            if( it_hist == m_histlist.end() ) break;
        }
    }
}



//
// list->XML 変換
//
std::string HistorySubMenu::list2xml()
{
    std::stringstream xml;

    std::list< CORE::HIST_ITEM* >::iterator it = m_histlist.begin();
    for(; it != m_histlist.end(); ++it ){

        Glib::ustring url = ( *it )->url;
        Glib::ustring name = ( *it )->name;
        int type = ( *it )->type;

        switch( type ){

            case TYPE_BOARD: // 板
                XML_MAKE_BOARD(url,name);
                break;
                
            case TYPE_THREAD: // スレ
                XML_MAKE_THREAD(url,name);
                break;
        }
    }

#ifdef _DEBUG
    std::cout << "HistoryMenu::list2xml\n";
    std::cout << xml.str() << std::endl;
#endif

    return xml.str();
}



// メニューアイテムがactiveになった
void HistorySubMenu::slot_activate( int i )
{
#ifdef _DEBUG
    std::cout << "HistorySubMenu::slot_activate" << i << std::endl;
#endif
    std::list< CORE::HIST_ITEM* >::iterator it = m_histlist.begin();
    for( int i2 = 0; i2 < i && it != m_histlist.end() ; ++it, ++i2 );
    if( it == m_histlist.end() ) return;

    std::string& url = ( *it )->url;
    int type = ( *it )->type;
    if( !url.empty() ){
#ifdef _DEBUG
        std::cout << "open " << url << std::endl;
#endif
        if( type == TYPE_THREAD ) CORE::core_set_command( "open_article" , url, "true" );
        else if( type == TYPE_BOARD ) CORE::core_set_command( "open_board" , url, "true" );
    }
}



// ラベルをセット
void HistorySubMenu::set_menulabel()
{
#ifdef _DEBUG
    std::cout << "HistorySubMenu::set_menulabel\n";
#endif

    std::list< CORE::HIST_ITEM* >::iterator it_hist = m_histlist.begin();
    std::list< Gtk::MenuItem* >::iterator it_item = m_itemlist.begin();
    for(; it_hist != m_histlist.end(); ++it_hist, ++it_item ){
        if( !( *it_hist )->url.empty() ){

            std::string url = ( *it_hist )->url;
            std::string name = ( *it_hist )->name;
            int type = ( *it_hist )->type;

            if( url.empty() ) name = HIST_NONAME;
            else if( name.empty() ){

                if( type == TYPE_BOARD ) name = DBTREE::board_name( url );
                else if( type == TYPE_THREAD ) name = DBTREE::article_subject( url );

                if( name.empty() ) name = "???";
                else ( *it_hist )->name = name;
            }


            // 履歴に表示する文字数を制限
            unsigned int pos, lng_name;
            int byte = 0;
            for( pos = 0, lng_name = 0; pos < name.length(); pos += byte ){
                MISC::utf8toucs2( name.c_str()+pos, byte );
                if( byte > 1 ) lng_name += 2;
                else ++lng_name;
                if( lng_name >= HIST_MAX_LNG ) break;
            }

            // カットしたら"..."をつける
            if( pos != name.length() ) name = name.substr( 0, pos ) + "...";

            dynamic_cast< Gtk::Label* >( (*it_item)->get_child() )->set_text( name );
        }
    }
}


