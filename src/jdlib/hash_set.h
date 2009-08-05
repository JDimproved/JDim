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

      simple_hash_set( const int hash_size )
      : m_hash_size( hash_size )
        {}

        virtual ~simple_hash_set()
        {
#ifdef _DEBUG
            std::cout << "simple_hash_set::~simple_hash_set\n";
            for( size_t i = 0; i < m_hash.size(); ++i ) if( m_hash[ i ].size() ) std::cout << "[" << i << "] " << m_hash[ i ].size() << std::endl;
#endif
        }

        const int size(){ return m_hash_size; }

        void clear()
        {
            m_hash.clear();
        }

        void insert( const std::string& item )
        {
            if( ! m_hash.size() ) m_hash.resize( m_hash_size );

            const int key = get_key( item );
            m_hash[ key ].insert( item );
        }

        void erase( const std::string& item )
        {
            if( ! m_hash.size() ) return;

            const int key = get_key( item );
            m_hash[ key ].erase( item );
        }

        const bool find_if( const std::string& item )
        {
            if( ! m_hash.size() ) return false;

            const int key = get_key( item );
            return ( m_hash[ key ].find( item ) != m_hash[ key ].end() );
        }

      private:

        virtual const int get_key( const std::string& item )
        {
            return ( atoi( item.c_str() ) % m_hash_size );
        }
    };
}

#endif
