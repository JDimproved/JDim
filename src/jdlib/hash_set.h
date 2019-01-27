// ライセンス: GPL2

// 簡易 hash_set

#ifndef _HASH_SET_H
#define _HASH_SET_H

#include <vector>
#include <set>
#include <string>

namespace JDLIB
{
    typedef std::set< std::string > HASH_SET_ITEM;

    class simple_hash_set
    {
        int m_hash_size;
        std::vector< HASH_SET_ITEM > m_hash;

      public:

        simple_hash_set( const int hash_size );
        virtual ~simple_hash_set();

        int size() const { return m_hash_size; }

        void clear();
        void insert( const std::string& item );
        void erase( const std::string& item );
        bool find_if( const std::string& item );

      private:

        virtual int get_key( const std::string& item ) = 0;
    };


    /////////////////////////////////////


    // スレッドのアドレス用 hash_set

    class hash_set_thread : public JDLIB::simple_hash_set
    {
      public:
        hash_set_thread();

      private:

        virtual int get_key( const std::string& url );
    };
}

#endif
