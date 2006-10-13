// ライセンス: 最新のGPL

#define _DEBUG
#include "jddebug.h"

#include "usrcmdmanager.h"
#include "command.h"

#include "jdlib/miscutil.h"

CORE::Usrcmd_Manager* instance_usrcmd_manager = NULL;


CORE::Usrcmd_Manager* CORE::get_usrcmd_manager()
{
    if( ! instance_usrcmd_manager ) instance_usrcmd_manager = new Usrcmd_Manager();
    assert( instance_usrcmd_manager );

    return instance_usrcmd_manager;
}


void CORE::delete_usrcmd_manager()
{
    if( instance_usrcmd_manager ) delete instance_usrcmd_manager;
    instance_usrcmd_manager = NULL;
}

///////////////////////////////////////////////

using namespace CORE;

Usrcmd_Manager::Usrcmd_Manager()
{
    m_size = 1;
    m_list_label.push_back( "googleで検索" );
    m_list_openbrowser.push_back( true );
}

//
// 実行
//
void Usrcmd_Manager::exec( int num, const std::string& url, const std::string& selection )
{
    if( num >= m_size ) return;

    std::string cmd = "http://www.google.co.jp/search?hl=ja&q=";
    cmd += MISC::url_encode( selection.c_str(), strlen( selection.c_str() ) );
    cmd += "&btnG=Google+%E6%A4%9C%E7%B4%A2&lr=";
       
#ifdef _DEBUG
    std::cout << "Usrcmd_Manager::url = " << url << std::endl;
    std::cout << "selection = " << selection << std::endl;
    std::cout << "exec " << cmd << std::endl;
#endif

    if( m_list_openbrowser[ num ] ) CORE::core_set_command( "open_url_browser", cmd );
}


const std::string Usrcmd_Manager::get_label( int num )
{
    if( num >= m_size ) return std::string();

    std::string label = m_list_label[ num ];

#ifdef _DEBUG
    std::cout << "Usrcmd_Manager::get_label  = " << label << std::endl;
#endif

    return label;
}


bool Usrcmd_Manager::sensitive( int num, const std::string& url, const std::string& selection )
{
    if( num >= m_size ) return false;

    if( selection.empty() ) return false;

    return true;
}
