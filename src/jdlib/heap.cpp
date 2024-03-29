// ライセンス: GPL2

// NOTE: unsigned charでメモリを確保する根拠 (C++規格ドラフト)
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3337.pdf#section.3.10 (3.10.10)
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3337.pdf#section.3.11 (3.11.6)

//#define _DEBUG
#include "jddebug.h"

#include "heap.h"

#include <cstring>


using namespace JDLIB;

HEAP::HEAP( std::size_t blocksize ) noexcept
    : m_blocksize{ blocksize }
    , m_block_iter{ m_heap_list.end() }
{}


HEAP::~HEAP() noexcept = default;


/**
 * @brief ブロックを確保したまま利用状況を初期化する
 */
void HEAP::clear()
{
    m_space_avail = 0;
    m_ptr_head = nullptr;
    m_block_iter = m_heap_list.begin();
}


void* HEAP::heap_alloc( std::size_t size_bytes, std::size_t alignment )
{
    assert( m_blocksize > size_bytes && size_bytes > 0 );
    assert( size_bytes >= alignment );

    while(1) {
        if( !m_ptr_head || m_space_avail < size_bytes || m_space_avail >= m_blocksize ) {
            if( m_block_iter != m_heap_list.end() ) {
                // メモリブロックは使う前にzero-fillする
                m_ptr_head = &(*m_block_iter)[0];
                std::memset( m_ptr_head, 0, sizeof(unsigned char) * m_blocksize );
                ++m_block_iter;
            }
            else {
                // std::listは要素追加でイテレーターが無効にならない
                // イテレーターがendを指しているときはブロックを追加
                // 確保したメモリブロックはゼロ初期化する
                m_heap_list.emplace_back( new unsigned char[m_blocksize]{} );
                m_ptr_head = &m_heap_list.back()[0];
            }
            m_space_avail = m_blocksize - 1;
        }

        // アライメント調整された指定サイズのバッファを探す
        if(std::align( alignment, size_bytes, m_ptr_head, m_space_avail )) {
            // 見つかったアドレスを保存してポインターと空きスペースを次の検索開始位置に合わせる
            void* found = m_ptr_head;
            m_ptr_head = std::next( reinterpret_cast<unsigned char*>(m_ptr_head), size_bytes );
            m_space_avail -= size_bytes;
            return found;
        }
        else {
            // 見つからなかった場合は新しいメモリブロックを追加する
            m_ptr_head = nullptr;
            m_space_avail = 0;
        }
    }
}
