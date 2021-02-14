// ライセンス: GPL2

// ヒープクラス

#ifndef HEAP_H
#define HEAP_H

#include <list>
#include <memory>


namespace JDLIB
{
    class HEAP
    {
        std::list< std::unique_ptr<unsigned char[]> > m_heap_list;
        std::size_t m_blocksize; // ブロックサイズ
        std::size_t m_space_avail{}; // ブロックの未使用サイズ
        void* m_ptr_head{}; // 検索開始位置

      public:
        explicit HEAP( std::size_t blocksize ) noexcept;
        ~HEAP();

        HEAP( const HEAP& ) = delete;
        HEAP& operator=( const HEAP& ) = delete;
        HEAP( HEAP&& ) noexcept = default;
        HEAP& operator=( HEAP&& ) = default;

        void clear();

        // 戻り値はunsigned char*のエイリアス
        void* heap_alloc( std::size_t size_bytes, std::size_t alignment );

        template<typename T>
        T* heap_alloc( std::size_t length = 1 )
        {
            return reinterpret_cast<T*>( heap_alloc( sizeof(T) * length, alignof(T) ) );
        }
    };
}

#endif
