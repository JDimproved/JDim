// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "historysubmenu.h"
#include "global.h"
#include "command.h"
#include "cache.h"
#include "prefdiagfactory.h"

#include "dbtree/interface.h"

#include "jdlib/miscutil.h"
#include "jdlib/misctime.h"

#include "config/globalconf.h"

#include "xml/document.h"
#include "xml/tools.h"


#include <sstream>
#include <list>

using namespace CORE;

#define HIST_NONAME "--------------"

// ルート要素名( history.xml, history_board.xml history_close.xml )
#define ROOT_NODE_NAME "history"

// 履歴に表示する文字数(半角)
#define HIST_MAX_LNG 50

HistorySubMenu::HistorySubMenu( const std::string path_xml )
    : Gtk::Menu(),
      m_path_xml( path_xml )
{
    Gtk::MenuItem* item;

    // メニュー項目作成
    for( int i = 0; i < CONFIG::get_history_size(); ++i ){
        item = Gtk::manage( new Gtk::MenuItem( HIST_NONAME ) );
        m_itemlist.push_back( item );
        append( *item );
        item->signal_button_press_event().connect( sigc::bind<int>( sigc::mem_fun( *this, &HistorySubMenu::slot_button_press ), i ));

        CORE::HIST_ITEM* histitem = new CORE::HIST_ITEM;
        histitem->type = TYPE_UNKNOWN;
        m_histlist.push_back( histitem );
    }

    // ポップアップメニュー作成
    m_popupmenu.signal_deactivate().connect( sigc::mem_fun( *this, &HistorySubMenu::deactivate ) );

    item = Gtk::manage( new Gtk::MenuItem( "開く" ) );
    item->signal_activate().connect( sigc::mem_fun( *this, &HistorySubMenu::slot_open_history ) );
    m_popupmenu.append( *item );

    item = Gtk::manage( new Gtk::SeparatorMenuItem() );
    m_popupmenu.append( *item );

    item = Gtk::manage( new Gtk::MenuItem( "履歴から削除" ) );
    item->signal_activate().connect( sigc::mem_fun( *this, &HistorySubMenu::slot_remove_history ) );
    m_popupmenu.append( *item );

    item = Gtk::manage( new Gtk::SeparatorMenuItem() );
    m_popupmenu.append( *item );

    item = Gtk::manage( new Gtk::MenuItem( "プロバティ" ) );
    item->signal_activate().connect( sigc::mem_fun( *this, &HistorySubMenu::slot_show_property ) );
    m_popupmenu.append( *item );

    m_popupmenu.show_all_children();

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


// 上から num 版目の HIST_ITEM 取得
CORE::HIST_ITEM* HistorySubMenu::get_item( int num )
{
    CORE::HIST_ITEM* item = NULL;

    if( m_histlist.size() > ( size_t ) num ){

        std::list< CORE::HIST_ITEM* >::iterator it = m_histlist.begin();
        for( int i = 0; i < num && it != m_histlist.end() ; ++it, ++i );
        if( it != m_histlist.end() ) item = *it;
    }

    return item;
}


void HistorySubMenu::append_item( const std::string& url, const std::string& name, int type )
{
#ifdef _DEBUG
    std::cout << "HistorySubMenu::append_item"
              << " url = " << url
              << " name = " << name
              << " type = " << type << std::endl;
#endif   

    std::list< CORE::HIST_ITEM* >::iterator it;
    CORE::HIST_ITEM* item = NULL;

    if( ! m_histlist.size() ) return;

    it = m_histlist.begin();
    for(; it != m_histlist.end(); ++it ){

        item = (*it);
        assert( item );

        // 同じURLがあったら先頭に持ってくる
        if( item->type == type && item->url == url ){
#ifdef _DEBUG
            std::cout << "found in list\n";
#endif
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
// 移転などでURLを更新する
//
void HistorySubMenu::update()
{
#ifdef _DEBUG
    std::cout << "HistorySubMenu::update\n";
#endif

    std::list< CORE::HIST_ITEM* >::iterator it = m_histlist.begin();
    for(; it != m_histlist.end(); ++it ){

        CORE::HIST_ITEM* item = (*it);
        assert( item );

        if( item->type == TYPE_THREAD ) item->url = DBTREE::url_dat( item->url );
        else if( item->type == TYPE_BOARD ) item->url = DBTREE::url_subject( item->url );
    }
}


//
// XML->list 変換
//
void HistorySubMenu::xml2list( const std::string& xml )
{
    if( xml.empty() ) return;

    XML::Document document( xml );

    XML::Dom* root = document.get_root_element( std::string( ROOT_NODE_NAME ) );

    // ルート要素の有無で処理を分ける( 旧様式=無, 新様式=有 )
    XML::DomList domlist;
    if( root ) domlist = root->childNodes();
    else
    {
        domlist = document.childNodes();

        // 別のファイル名
        const std::string file = m_path_xml + "." + MISC::get_sec_str();

        // 旧様式のXMLを別の名前で保存する
        CACHE::save_rawdata( file, xml );
    }


#ifdef _DEBUG
    std::cout << "HistoryMenu::xml2list\n";
    std::cout << " 子ノード数=" << documen.childNodes().size() << std::endl;
#endif

    std::list< CORE::HIST_ITEM* >::iterator it_hist = m_histlist.begin();
    std::list< XML::Dom* >::iterator it = domlist.begin();
    while( it != domlist.end() && it_hist != m_histlist.end() )
    {
        if( (*it)->nodeType() == XML::NODE_TYPE_ELEMENT )    
        {
            const int type = XML::get_type( (*it)->nodeName() );
            const std::string name = (*it)->getAttribute( "name" );
            const std::string url = (*it)->getAttribute( "url" );

            if( type != TYPE_UNKNOWN && ! name.empty() && ! url.empty() )
            {
                ( *it_hist )->url = url;
                ( *it_hist )->name = name;
                ( *it_hist )->type = type;
                ++it_hist;
                if( it_hist == m_histlist.end() ) break;
            }
        }
        ++it;
    }
}



//
// list->XML 変換
//
std::string HistorySubMenu::list2xml()
{
	// Domノードを作成
    XML::Document document;

    // ルート要素を追加
    XML::Dom* root = document.appendChild( XML::NODE_TYPE_ELEMENT, std::string( ROOT_NODE_NAME ) );

    std::list< CORE::HIST_ITEM* >::iterator it = m_histlist.begin();
    while( it != m_histlist.end() )
    {
        const Glib::ustring name = ( *it )->name;
        const Glib::ustring url = ( *it )->url;
        const int type = ( *it )->type;
        const std::string node_name = XML::get_name( type );

        if( type != TYPE_UNKNOWN && ! name.empty() && ! url.empty() )
        {
            XML::Dom* node = root->appendChild( XML::NODE_TYPE_ELEMENT, node_name );
            node->setAttribute( "name", name );
            node->setAttribute( "url", url );
        }
        ++it;
    }

    std::string xml;

    if( root->hasChildNodes() ) xml = document.get_xml();

#ifdef _DEBUG
    std::cout << "HistoryMenu::list2xml\n";
    std::cout << xml << std::endl;
#endif

    return xml;
}



// 履歴を開く
void HistorySubMenu::open_history( int i )
{
    CORE::HIST_ITEM* item = get_item( i );
    if( ! item ) return;

    std::string& url = item->url;
    int type = item->type;
    if( !url.empty() ){
#ifdef _DEBUG
        std::cout << "open " << url << std::endl;
#endif
        if( type == TYPE_THREAD ) CORE::core_set_command( "open_article" , url, "true", "" );
        else if( type == TYPE_BOARD ) CORE::core_set_command( "open_board" , url, "true", "" );
    }
}



// メニューアイテムがactiveになった
bool HistorySubMenu::slot_button_press( GdkEventButton* event, int i )
{
#ifdef _DEBUG
    std::cout << "HistorySubMenu::slot_button_press button = " << event->button << " no = " << i << std::endl;
#endif

    m_number_menuitem = i;

    // ポップアップメニュー表示
    if( event->button == 3 ){
        m_popupmenu.popup( 0, gtk_get_current_event_time() );
        return true;
    }

    open_history( i );
    return true;
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

        dynamic_cast< Gtk::Label* >( (*it_item)->get_child() )->set_text( MISC::cut_str( name, HIST_MAX_LNG ) );
    }
}


// 指定した履歴を開く
// これを呼ぶ前に m_number_menuitem に番号をセットしておく
void HistorySubMenu::slot_open_history()
{
#ifdef _DEBUG
    std::cout << "HistorySubMenu::slot_open_history no = " << m_number_menuitem << std::endl;
#endif

    open_history( m_number_menuitem );
}


// 指定した履歴を削除
// これを呼ぶ前に m_number_menuitem に番号をセットしておく
void HistorySubMenu::slot_remove_history()
{
#ifdef _DEBUG
    std::cout << "HistorySubMenu::slot_remove_history no = " << m_number_menuitem << std::endl;
#endif 

    CORE::HIST_ITEM* item = get_item( m_number_menuitem );
    if( ! item ) return;

    m_histlist.remove( item );

#ifdef _DEBUG
    std::cout << "remove " << item->name << std::endl;
#endif

    item->url = std::string();
    item->name = std::string();
    item->type = TYPE_UNKNOWN;
    m_histlist.push_back( item );
}


// プロバティ表示
// これを呼ぶ前に m_number_menuitem に番号をセットしておく
void HistorySubMenu::slot_show_property()
{
#ifdef _DEBUG
    std::cout << "HistorySubMenu::slot_show_property no = " << m_number_menuitem << std::endl;
#endif

    CORE::HIST_ITEM* item = get_item( m_number_menuitem );
    if( ! item ) return;

    std::string& url = item->url;
    int type = item->type;
    if( !url.empty() ){

#ifdef _DEBUG
        std::cout << "open " << url << std::endl;
#endif

        SKELETON::PrefDiag* pref = NULL;
        if( type == TYPE_THREAD ) pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_ARTICLE, DBTREE::url_dat( url ) ); 
        else if( type == TYPE_BOARD ) pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_BOARD, DBTREE::url_subject( url ) );

        if( pref ){
            pref->run();
            delete pref;
        }
    }
}
