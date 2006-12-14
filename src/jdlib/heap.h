// ライセンス: GPL2

// ヒープクラス

#ifndef _HEAP_H
#define _HEAP_H

#include <string>
#include <list>

namespace JDLIB
{
    class HEAP
    {
        std::list< unsigned char* >m_heap_list;
        long m_max;  // ブロックサイズ
        long m_used; // ブロック内の使用量
        long m_total_size; // トータルサイズ
        
      public:
        HEAP( long blocksize );
        ~HEAP();

        void clear();

        unsigned char* heap_alloc( long n );
    };
}

#endif
