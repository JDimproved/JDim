// ライセンス: GPL2

//#define _DEBUG
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


static CORE::Linkfilter_Manager* instance_linkfilter_manager = nullptr;


CORE::Linkfilter_Manager* CORE::get_linkfilter_manager()
{
    if( ! instance_linkfilter_manager ) instance_linkfilter_manager = new Linkfilter_Manager();
    assert( instance_linkfilter_manager );

    return instance_linkfilter_manager;
}


void CORE::delete_linkfilter_manager()
{
    if( instance_linkfilter_manager ) delete instance_linkfilter_manager;
    instance_linkfilter_manager = nullptr;
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

    const XML::Document document( xml );

    const XML::Dom* root = document.get_root_element( std::string( ROOT_NODE_NAME_LINKFILTER ) );
    if( ! root ) return;

#ifdef _DEBUG
    std::cout << "Linkfilter_Manager::xml2list";
    std::cout << " children =" << document.size() << std::endl;
#endif

    for( const XML::Dom* child : *root ) {

        if( child->nodeType() == XML::NODE_TYPE_ELEMENT ){

            LinkFilterItem item;
            item.url = child->getAttribute( "url" );
            item.cmd = child->getAttribute( "data" );

#ifdef _DEBUG
            std::cout << "url = " << item.url
                      << " cmd =" << item.cmd << std::endl;
#endif

            if( ! item.url.empty() && ! item.cmd.empty() ) m_list_cmd.push_back( item );
        }
    }
}


//
// XML 保存
//
void Linkfilter_Manager::save_xml()
{
    XML::Document document;
    XML::Dom* root = document.appendChild( XML::NODE_TYPE_ELEMENT, std::string( ROOT_NODE_NAME_LINKFILTER ) );
    if( ! root ) return;

    for( const LinkFilterItem& item : m_list_cmd ) {

        const std::string& url = item.url;
        const std::string& cmd = item.cmd;

        if( ! url.empty() && ! cmd.empty() ){

            XML::Dom* node = root->appendChild( XML::NODE_TYPE_ELEMENT, XML::get_name( TYPE_LINKFILTER ) );
            node->setAttribute( "url", url );
            node->setAttribute( "data", cmd );
        }
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
bool Linkfilter_Manager::exec( const std::string& url, const std::string& link, const std::string& selection )
{
    if( ! m_list_cmd.size() ) return false;

#ifdef _DEBUG
        std::cout << "Linkfilter_Manager::exec\n"
                  << "url = " << url << std::endl
                  << "link = " << link << std::endl
                  << "selection = " << selection << std::endl;
#endif

    JDLIB::Regex regex;
    const size_t offset = 0;
    const bool icase = false;
    const bool newline = true;
    const bool usemigemo = false;
    const bool wchar = false;

    for( const LinkFilterItem& item : m_list_cmd ) {

        const std::string& query = item.url;
        const std::string& cmd = item.cmd;

#ifdef _DEBUG
        std::cout << "query = " << query << std::endl
                  << "cmd = " << cmd << std::endl;
#endif
        if( ! regex.exec( query, link, offset, icase, newline, usemigemo, wchar ) ) continue;

        // \0 ... \9 までのcmd文字列を置換
        const std::string cmd_out = regex.replace( cmd );

        // queryと一致したら実行
        CORE::get_usrcmd_manager()->exec( cmd_out, url, link, selection, 0 );

        return true;
    }

    return false;
}
