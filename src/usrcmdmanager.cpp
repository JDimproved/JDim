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

    cmd = replace_cmd( cmd, url, link, selection );
       
#ifdef _DEBUG
    std::cout << "Usrcmd_Manager::url = " << url << std::endl;
    std::cout << "link = " << link << std::endl;
    std::cout << "selection = " << selection << std::endl;
    std::cout << "exec " << cmd << std::endl;
#endif

    if( m_list_openbrowser[ num ] ) CORE::core_set_command( "open_url_browser", cmd );
    else Glib::spawn_command_line_async( cmd );
}


// コマンド置換
// cmdの$URLをurl, $LINKをlink, $TEXT*をtextで置き換えて出力
// text は UTF-8 であること
std::string Usrcmd_Manager::replace_cmd( const std::string& cmd,
                                         const std::string& url, const std::string& link, const std::string& text )
{
    std::string cmd_out = cmd;

    cmd_out = MISC::replace_str( cmd_out, "$URL", DBTREE::url_readcgi( url, 0, 0 ) );
    cmd_out = MISC::replace_str( cmd_out, "$DATURL", DBTREE::url_dat( url ) );
    cmd_out = MISC::replace_str( cmd_out, "$LOCALDAT", CACHE::path_dat( url ) );
    cmd_out = MISC::replace_str( cmd_out, "$LINK", link );
    cmd_out = MISC::replace_str( cmd_out, "$SERVERL", MISC::get_hostname( link ) );
    cmd_out = MISC::replace_str( cmd_out, "$SERVER", MISC::get_hostname( url ) );
    cmd_out = MISC::replace_str( cmd_out, "$HOSTNAMEL", MISC::get_hostname( link, false ) );
    cmd_out = MISC::replace_str( cmd_out, "$HOSTNAME", MISC::get_hostname( url, false ) );

    if( cmd_out.find( "$TEXTU" ) != std::string::npos ){
        cmd_out = MISC::replace_str( cmd_out, "$TEXTU", MISC::charset_url_encode_split( text, "UTF-8" ) );
    }
    if( cmd_out.find( "$TEXTX" ) != std::string::npos ){
        cmd_out = MISC::replace_str( cmd_out, "$TEXTX", MISC::charset_url_encode_split( text, "EUC-JP" ) );
    }
    if( cmd_out.find( "$TEXTE" ) != std::string::npos ){
        cmd_out = MISC::replace_str( cmd_out, "$TEXTE", MISC::charset_url_encode_split( text, "MS932" ) );
    }
    if( cmd_out.find( "$TEXT" ) != std::string::npos ){
        cmd_out = MISC::replace_str( cmd_out, "$TEXT",  text );
    }

#ifdef _DEBUG
    std::cout << "Usrcmd_Manager::replace_cmd\n";
    std::cout << "cmd = " << cmd << std::endl;
    std::cout << "out = " << cmd_out << std::endl;
#endif

    return cmd_out;
}


//
// メニューをアクティブにするか
//
bool Usrcmd_Manager::is_sensitive( int num, const std::string& link, const std::string& selection )
{
    const unsigned int max_selection_str = 1024;

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

        if( selection.empty() || selection.length() > max_selection_str ) return false;
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
