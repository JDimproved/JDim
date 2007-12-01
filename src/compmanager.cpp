// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "jdlib/miscutil.h"

#include "compmanager.h"
#include "cache.h"

enum
{
    MAX_COMPLETION = 50
};

CORE::Completion_Manager* instance_completion_manager = NULL;


CORE::Completion_Manager* CORE::get_completion_manager()
{
    if( ! instance_completion_manager ) instance_completion_manager = new Completion_Manager();
    assert( instance_completion_manager );

    return instance_completion_manager;
}


void CORE::delete_completion_manager()
{
    if( instance_completion_manager ) delete instance_completion_manager;
    instance_completion_manager = NULL;
}

///////////////////////////////////////////////

using namespace CORE;

Completion_Manager::Completion_Manager()
{
    for( int i = 0; i < COMP_SIZE; ++i ){
        m_lists.push_back( new COMPLIST );
        load_info( i );
    }
}


Completion_Manager::~Completion_Manager()
{
    for( int i = 0; i < COMP_SIZE; ++i ){
        save_info( i );
        delete m_lists[ i ];
    }
}


COMPLIST Completion_Manager::get_list( int mode, std::string& query )
{
    COMPLIST complist;

    if( mode < COMP_SIZE ){

#ifdef _DEBUG
        std::cout << "Completion_Manager::get_list mode = " << mode << " query = " << query << std::endl;
#endif

        if( query.empty()
            || query == " "
            || query == "　" // 全角スペース
            ) complist = *m_lists[ mode ];
        else{

            std::string tmp_query = MISC::tolower_str( query );

            CORE::COMPLIST_ITERATOR it = m_lists[ mode ]->begin();
            for( ; it != m_lists[ mode ]->end(); ++it ){

                std::string tmp_str = MISC::tolower_str( *it );
                if( tmp_str.find( tmp_query ) != std::string::npos ) complist.push_back( *it );
            }
        }
    }

    return complist;
}


void Completion_Manager::set_query( int mode, std::string& query )
{
    if( mode < COMP_SIZE ){

#ifdef _DEBUG
        std::cout << "Completion_Manager::set_query mode = " << mode << " query = " << query << std::endl;
#endif

        COMPLIST* complist = m_lists[ mode ];

        complist->remove( query );
        complist->push_front( query );

        if( complist->size() > MAX_COMPLETION ) complist->pop_back();
    }
}


void Completion_Manager::clear( int mode )
{
    if( mode < COMP_SIZE ){
        m_lists[ mode ]->clear();

        std::string path = CACHE::path_completion( mode );
        unlink( path.c_str() );
    }
}


// 情報ファイル読み書き
void Completion_Manager::load_info( int mode )
{
    std::string path = CACHE::path_completion( mode );
    std::string info;
    CACHE::load_rawdata( path, info );

#ifdef _DEBUG
    std::cout << "Completion_Manager::load_info path = " << path << std::endl
              << info << std::endl;
#endif

    if( ! info.empty() ){
        *m_lists[ mode ] = MISC::get_lines( info );
        *m_lists[ mode ] = MISC::remove_nullline_from_list( *m_lists[ mode ] );
        *m_lists[ mode ] = MISC::remove_space_from_list( *m_lists[ mode ] );
    }
}


void Completion_Manager::save_info( int mode )
{
    std::string info;
    COMPLIST* complist = m_lists[ mode ];

    if( complist->size() ){

        std::string path = CACHE::path_completion( mode );

        COMPLIST_ITERATOR it = complist->begin();
        for(; it != complist->end(); ++it ) info += (*it) + "\n";

#ifdef _DEBUG
        std::cout << "Completion_Manager::save_info path = " << path << std::endl
                  << info << std::endl;
#endif

        CACHE::save_rawdata( path, info );
    }
}
