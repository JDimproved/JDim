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
{
#ifdef _DEBUG        
    std::cout << "HEAP::HEAP : max = " << m_max << std::endl;
#endif
}


HEAP::~HEAP()
{
#ifdef _DEBUG        
    std::cout << "HEAP::~HEAP : size " << m_total_size << " max =" << m_max << std::endl;
#endif

    clear();
}


void HEAP::clear()
{
#ifdef _DEBUG        
    std::cout << "HEAP::crear max = " << m_max <<  " total = " << m_total_size << std::endl;
#endif

    m_total_size = 0;
    m_used = 0;
    
    std::list< unsigned char* >::iterator it;
    for( it = m_heap_list.begin(); it != m_heap_list.end(); ++it ){
        free( (*it) );
    }
    m_heap_list.clear();
}


unsigned char* HEAP::heap_alloc( long n, long alignment )
{
    assert( n > 0 && n <= m_max );

    if( m_used == 0 || m_used + n + (alignment - 1) > m_max ){

        m_heap_list.push_back( ( unsigned char* )malloc( m_max ) );
        memset( m_heap_list.back(), 0, m_max );
        m_used = 0;

#ifdef _DEBUG
        std::cout << "HEAP::heap_alloc malloc max = " << m_max <<  " total = " << m_total_size + n + 4 << std::endl;
#endif
    }

    unsigned char* heap = m_heap_list.back() + m_used;

    // アライメント調整
    uintptr_t i_heap = reinterpret_cast<uintptr_t>(heap);
    int rem = (i_heap % alignment) ? alignment - (i_heap % alignment) : 0;
    heap += rem;
    m_used += rem;
    m_total_size += rem;

    m_used += n + 4;
    m_total_size += n + 4;
    
    return heap;
}
