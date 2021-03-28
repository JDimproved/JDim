// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "jdlib/miscutil.h"

#include "compmanager.h"
#include "cache.h"

#include <unistd.h>

enum
{
    MAX_COMPLETION = 50
};

CORE::Completion_Manager* instance_completion_manager = nullptr;


CORE::Completion_Manager* CORE::get_completion_manager()
{
    if( ! instance_completion_manager ) instance_completion_manager = new Completion_Manager();
    assert( instance_completion_manager );

    return instance_completion_manager;
}


void CORE::delete_completion_manager()
{
    if( instance_completion_manager ) delete instance_completion_manager;
    instance_completion_manager = nullptr;
}

///////////////////////////////////////////////

using namespace CORE;

Completion_Manager::Completion_Manager()
    : m_lists( COMP_SIZE )
{
    for( int i = 0; i < COMP_SIZE; ++i ){
        load_info( i );
    }
}


void Completion_Manager::save_session()
{
    for( int i = 0; i < COMP_SIZE; ++i ){
        save_info( i );
    }
}


COMPLIST Completion_Manager::get_list( const int mode, const std::string& query )
{
    COMPLIST complist;

    if( mode < COMP_SIZE ){

#ifdef _DEBUG
        std::cout << "Completion_Manager::get_list mode = " << mode << " query = " << query << std::endl;
#endif

        if( query.empty()
            || query == " "
            || query == "　" // 全角スペース
        ) {
            complist = m_lists[ mode ];
        }
        else{

            const std::string lower_query = MISC::tolower_str( query );

            for( const std::string& str : m_lists[ mode ] ) {
                const std::string lower_str = MISC::tolower_str( str );
                if( lower_str.find( lower_query ) != std::string::npos ) complist.push_back( str );
            }
        }
    }

    return complist;
}


void Completion_Manager::set_query( const int mode, const std::string& query )
{
    if( ! query.empty() && mode < COMP_SIZE ){

#ifdef _DEBUG
        std::cout << "Completion_Manager::set_query mode = " << mode << " query = " << query << std::endl;
#endif

        COMPLIST& complist = m_lists[ mode ];

        complist.remove( query );
        complist.push_front( query );

        if( complist.size() > MAX_COMPLETION ) complist.pop_back();
    }
}


void Completion_Manager::clear( const int mode )
{
    if( mode < COMP_SIZE ){
        m_lists[ mode ].clear();

        std::string path = CACHE::path_completion( mode );
        unlink( to_locale_cstr( path ) );
    }
}


// 情報ファイル読み書き
void Completion_Manager::load_info( const int mode )
{
    std::string path = CACHE::path_completion( mode );
    std::string info;
    CACHE::load_rawdata( path, info );

#ifdef _DEBUG
    std::cout << "Completion_Manager::load_info path = " << path << std::endl
              << info << std::endl;
#endif

    if( ! info.empty() ){
        m_lists[ mode ] = MISC::get_lines( info );
        m_lists[ mode ] = MISC::remove_nullline_from_list( m_lists[ mode ] );
        m_lists[ mode ] = MISC::remove_space_from_list( m_lists[ mode ] );
    }
}


void Completion_Manager::save_info( const int mode )
{
    std::string info;
    COMPLIST& complist = m_lists[ mode ];

    if( ! complist.empty() ) {

        std::string path = CACHE::path_completion( mode );

        for( const std::string& str : complist ) {
            info.append( str );
            info.push_back( '\n' );
        }

#ifdef _DEBUG
        std::cout << "Completion_Manager::save_info path = " << path << std::endl
                  << info << std::endl;
#endif

        CACHE::save_rawdata( path, info );
    }
}
