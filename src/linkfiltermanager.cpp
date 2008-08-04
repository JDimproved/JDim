// ライセンス: GPL2

#define _DEBUG
#include "jddebug.h"

#include "linkfiltermanager.h"
#include "usrcmdmanager.h"
#include "cache.h"
#include "type.h"
#include "command.h"

#include "jdlib/jdregex.h"
#include "jdlib/miscutil.h"

#include "xml/document.h"
#include "xml/tools.h"

#define ROOT_NODE_NAME_LINKFILTER "linkfilterlist"

CORE::Linkfilter_Manager* instance_linkfilter_manager = NULL;

CORE::Linkfilter_Manager* CORE::get_linkfilter_manager()
{
    if( ! instance_linkfilter_manager ) instance_linkfilter_manager = new Linkfilter_Manager();
    assert( instance_linkfilter_manager );

    return instance_linkfilter_manager;
}


void CORE::delete_linkfilter_manager()
{
    if( instance_linkfilter_manager ) delete instance_linkfilter_manager;
    instance_linkfilter_manager = NULL;
}

///////////////////////////////////////////////

using namespace CORE;

Linkfilter_Manager::Linkfilter_Manager()
{
    std::string xml;
    if( CACHE::load_rawdata( CACHE::path_linkfilter(), xml ) ) xml2list( xml );
}


//
// xml -> リスト
//
void Linkfilter_Manager::xml2list( const std::string& xml )
{
    m_list_cmd.clear();
    if( xml.empty() ) return;

    XML::Document document( xml );

    XML::Dom* root = document.get_root_element( std::string( ROOT_NODE_NAME_LINKFILTER ) );
    if( ! root ) return;
    XML::DomList domlist = root->childNodes();

#ifdef _DEBUG
    std::cout << "Linkfilter_Manager::xml2list";
    std::cout << " children =" << document.childNodes().size() << std::endl;
#endif

    std::list< XML::Dom* >::iterator it = domlist.begin();
    while( it != domlist.end() ){

        if( ( *it )->nodeType() == XML::NODE_TYPE_ELEMENT ){

            LinkFilterItem item;
            item.url = (*it)->getAttribute( "url" );
            item.cmd = (*it)->getAttribute( "data" );

#ifdef _DEBUG
            std::cout << "url = " << item.url
                      << " cmd =" << item.cmd << std::endl;
#endif

            if( ! item.url.empty() && ! item.cmd.empty() ) m_list_cmd.push_back( item );
        }
        ++it;
    }
}


//
// XML 保存
//
void Linkfilter_Manager::save_xml()
{
    if( ! m_list_cmd.size() ) return;

    XML::Document document;
    XML::Dom* root = document.appendChild( XML::NODE_TYPE_ELEMENT, std::string( ROOT_NODE_NAME_LINKFILTER ) );
    if( ! root ) return;

    std::vector< LinkFilterItem >::iterator it = m_list_cmd.begin();
    while( it != m_list_cmd.end() ){

        const std::string url = ( *it ).url;
        const std::string cmd = ( *it ).cmd;

        if( ! url.empty() && ! cmd.empty() ){

            XML::Dom* node = root->appendChild( XML::NODE_TYPE_ELEMENT, XML::get_name( TYPE_LINKFILTER ) );
            node->setAttribute( "url", url );
            node->setAttribute( "data", cmd );
        }
        ++it;
    }

#ifdef _DEBUG
    std::cout << "Linkfilter_Manager::save_xml\n";
    std::cout << document.get_xml() << std::endl;
#endif

    CACHE::save_rawdata( CACHE::path_linkfilter(), document.get_xml() );
}


//
// 実行
//
// 実行したら true を返す
//
const bool Linkfilter_Manager::exec( const std::string& url, const std::string& link, const std::string& selection )
{
    if( ! m_list_cmd.size() ) return false;

#ifdef _DEBUG
        std::cout << "Linkfilter_Manager::exec\n"
                  << "url = " << url << std::endl
                  << "link = " << link << std::endl
                  << "selection = " << selection << std::endl;
#endif

    JDLIB::Regex regex;
    std::vector< LinkFilterItem >::iterator it = m_list_cmd.begin();
    for( ; it != m_list_cmd.end(); ++it ){

        const std::string query = ( *it ).url;
        std::string cmd = ( *it ).cmd;
#ifdef _DEBUG
        std::cout << "query = " << query << std::endl
                  << "cmd = " << cmd << std::endl;
#endif
        if( ! regex.exec( query, link ) ) continue;

        // queryと一致
        bool use_browser = false;
        if( cmd.find( "$VIEW" ) == 0 ){
            use_browser = true;
            cmd = cmd.substr( 5 );
            cmd = MISC::remove_space( cmd );
        }

        cmd = CORE::get_usrcmd_manager()->replace_cmd( cmd, url, link, selection );

#ifdef _DEBUG
        std::cout << "exec " << cmd << std::endl;
#endif

        if( use_browser ) CORE::core_set_command( "open_url_browser", cmd );
        else Glib::spawn_command_line_async( cmd );

        return true;
    }

    return false;
}
