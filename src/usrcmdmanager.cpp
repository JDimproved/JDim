// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "usrcmdmanager.h"
#include "command.h"
#include "cache.h"

#include "jdlib/miscutil.h"

#include <gtkmm.h>
#include "dbtree/interface.h"

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
    // 設定ファイル(usrcmd.txt)からユーザーコマンドを読み込み
    std::string file_usrcmd = CACHE::path_usrcmd();
    std::string usrcmd;
    std::string label;
    std::string cmd;
    if( CACHE::load_rawdata( file_usrcmd, usrcmd ) ){

        std::list< std::string > list_usrcmd = MISC::get_lines( usrcmd );
        list_usrcmd = MISC::remove_commentline_from_list( list_usrcmd );
        list_usrcmd = MISC::remove_space_from_list( list_usrcmd );
        list_usrcmd = MISC::remove_nullline_from_list( list_usrcmd );
        std::list< std::string >::iterator it;
        for( it = list_usrcmd.begin(); it != list_usrcmd.end(); ++it ){

            label = *( it++ );
            if( it == list_usrcmd.end() ) break;
            cmd = *it;
            set_cmd( label, cmd );
        }
    }

    // デフォルト設定を保存
    else{

        label = "googleで検索";
        cmd = "$VIEW http://www.google.co.jp/search?hl=ja&q=$TEXTU&btnG=Google+%E6%A4%9C%E7%B4%A2&lr=";
        set_cmd( label, cmd );
        CACHE::save_rawdata( file_usrcmd, label + "\n" + cmd + "\n" );
    }

    m_size = m_list_label.size();
}



void Usrcmd_Manager::set_cmd( const std::string& label, const std::string& cmd )
{
    std::string label2 = MISC::remove_space( label );
    m_list_label.push_back( label2 );

    std::string cmd2 = MISC::remove_space( cmd );

    if( cmd2.find( "$VIEW" ) == 0 ){
        cmd2 = cmd2.substr( 5 );
        cmd2 = MISC::remove_space( cmd2 );
        m_list_openbrowser.push_back( true );
    }
    else m_list_openbrowser.push_back( false );

    m_list_cmd.push_back( cmd2 );

#ifdef _DEBUG
    std::cout << "Usrcmd_Manager::set_cmd\n";
    std::cout << "label " << label2 << std::endl;
    std::cout << "cmd " << cmd2 << std::endl;
#endif
}



//
// 実行
//
void Usrcmd_Manager::exec( int num, const std::string& url, const std::string& link, const std::string& selection )
{
    if( num >= m_size ) return;

    std::string cmd = m_list_cmd[ num ];

    cmd = MISC::replace_str( cmd, "$URL", DBTREE::url_readcgi( url, 0, 0 ) );
    cmd = MISC::replace_str( cmd, "$DATURL", DBTREE::url_dat( url ) );
    cmd = MISC::replace_str( cmd, "$LOCALDAT", CACHE::path_dat( url ) );
    cmd = MISC::replace_str( cmd, "$LINK", link );
    cmd = MISC::replace_str( cmd, "$SERVERL", MISC::get_hostname( link ) );
    cmd = MISC::replace_str( cmd, "$SERVER", MISC::get_hostname( url ) );
    cmd = MISC::replace_str( cmd, "$HOSTNAMEL", MISC::get_hostname( link, false ) );
    cmd = MISC::replace_str( cmd, "$HOSTNAME", MISC::get_hostname( url, false ) );

    cmd = MISC::replace_str( cmd, "$TEXTU", MISC::charset_url_encode( selection, "UTF-8" ) );
    cmd = MISC::replace_str( cmd, "$TEXTX", MISC::charset_url_encode( selection, "EUC-JP" ) );
    cmd = MISC::replace_str( cmd, "$TEXTE", MISC::charset_url_encode( selection, "MS932" ) );
    cmd = MISC::replace_str( cmd, "$TEXT", selection );
       
#ifdef _DEBUG
    std::cout << "Usrcmd_Manager::url = " << url << std::endl;
    std::cout << "link = " << link << std::endl;
    std::cout << "selection = " << selection << std::endl;
    std::cout << "exec " << cmd << std::endl;
#endif

    if( m_list_openbrowser[ num ] ) CORE::core_set_command( "open_url_browser", cmd );
    else Glib::spawn_command_line_async( cmd );
}


//
// メニューをアクティブにするか
//
bool Usrcmd_Manager::is_sensitive( int num, const std::string& link, const std::string& selection )
{
    if( num >= m_size ) return false;

    std::string cmd = m_list_cmd[ num ];

    if( cmd.find( "$LINK" ) != std::string::npos
        || cmd.find( "$SERVERL" ) != std::string::npos
        || cmd.find( "$HOSTNAMEL" ) != std::string::npos
        ){
        if( link.empty() ) return false;
    }

    if( cmd.find( "$TEXT" ) != std::string::npos
        || cmd.find( "$TEXTU" ) != std::string::npos
        || cmd.find( "$TEXTX" ) != std::string::npos
        || cmd.find( "$TEXTE" ) != std::string::npos
        ){

        if( selection.empty() ) return false;
    }
    
    return true;
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
