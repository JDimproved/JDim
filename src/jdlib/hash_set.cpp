// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "hash_set.h"

#include "dbtree/interface.h"

#include <cstdlib>


using namespace JDLIB;


simple_hash_set::simple_hash_set( const int hash_size )
    : m_hash_size( hash_size )
{}


simple_hash_set::~simple_hash_set()
{
#ifdef _DEBUG
    std::cout << "simple_hash_set::~simple_hash_set\n";
    for( size_t i = 0; i < m_hash.size(); ++i ) if( m_hash[ i ].size() ) std::cout << "[" << i << "] " << m_hash[ i ].size() << std::endl;
#endif
}


void simple_hash_set::clear()
{
    m_hash.clear();
}


void simple_hash_set::insert( const std::string& item )
{
    if( ! m_hash.size() ) m_hash.resize( m_hash_size );

    const int key = get_key( item );
    m_hash[ key ].insert( item );
}


void simple_hash_set::erase( const std::string& item )
{
    if( ! m_hash.size() ) return;

    const int key = get_key( item );
    m_hash[ key ].erase( item );
}


bool simple_hash_set::find_if( const std::string& item )
{
    if( ! m_hash.size() ) return false;

    const int key = get_key( item );
    return ( m_hash[ key ].find( item ) != m_hash[ key ].end() );
}



//////////////////////////////


// スレッドのアドレス用 hash_set

enum
{
    HASH_TBLSIZE = 1024
};


hash_set_thread::hash_set_thread()
    : JDLIB::simple_hash_set( HASH_TBLSIZE )
{}


int hash_set_thread::get_key( const std::string& url )
{

    const int lng = DBTREE::url_datbase( url ).length();
    const int key = atoi(  url.substr( lng < (int) url.length() ? lng : 0  ).c_str() ) % size();

#ifdef _DEBUG
    std::cout << "hash_set_thread::get_key url = " << url << " key = " << key << std::endl;
#endif

    return key ;
}
