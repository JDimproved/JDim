// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "heap.h"

#include <cstdlib>
#include <cstring>

using namespace JDLIB;

HEAP::HEAP( long blocksize )
    : m_max( blocksize ),
      m_used( 0 ),
      m_total_size( 0 )
{}


HEAP::~HEAP()
{
#ifdef _DEBUG        
    std::cout << "HEAP::~HEAP : size " << m_size / 1024 << "k\n";
#endif

    clear();
}


void HEAP::clear()
{
    m_total_size = 0;
    m_used = 0;
    
    std::list< unsigned char* >::iterator it;
    for( it = m_heap_list.begin(); it != m_heap_list.end(); ++it ){
        free( (*it) );
    }
    m_heap_list.clear();
}



unsigned char* HEAP::heap_alloc( long n )
{
    assert( n > 0 && n <= m_max );

    if( m_used == 0 || m_used + n > m_max ){
        m_heap_list.push_back( ( unsigned char* )malloc( m_max ) );
        memset( m_heap_list.back(), 0, m_max );
        m_used = 0;
    }

    unsigned char* heap = m_heap_list.back() + m_used;
    m_used += n + 4;
    m_total_size += n + 4;
    
    return heap;
}
