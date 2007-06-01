// License GPL2

// std::list< Dom* > の代わりのクラス
//
// Domクラスで std::list のメンバ関数の front() と back() の戻り値が
// 存在しないインスタンスへの参照になっていると都合が悪いので、その対策
// のためだけに作られたアホみたいなクラスです。ついでに添字を使えるよう
// にしてあります。

#ifndef _DOMLIST_H
#define _DOMLIST_H

#include "dom.h"


namespace XML
{
    class Dom;

    class DomList
    {
    	std::list< Dom* > m_list;

        //DomList( const DomList& );

      public:

        DomList();
        ~DomList();

        // std::list< Dom* > が代入された場合
        DomList& operator =( std::list< Dom* >& list );

        // 添字によるアクセス
        Dom* operator []( const unsigned int n );

        std::list< Dom* > get_list() { return m_list; }

        std::list< Dom* >::iterator begin();
        std::list< Dom* >::reverse_iterator rbegin();
        std::list< Dom* >::iterator end();
        std::list< Dom* >::reverse_iterator rend();
        size_t size() const;
        size_t max_size() const;
        bool empty() const;
        Dom* front(); // front() と back()は参照ではなくポインタを返す
        Dom* back();
        void push_front( Dom* dom );
        void push_back( Dom* dom );
        void pop_front();
        void pop_back();
        std::list< Dom* >::iterator insert( std::list< Dom* >::iterator it, Dom* dom );
        std::list< Dom* >::iterator erase( std::list< Dom* >::iterator it );
        void clear();
        void splice( std::list< Dom* >::iterator it, DomList& domlist );
        void remove( Dom* dom );
        void unique();
        void merge( DomList& domlist );
        void sort();
        void reverce();
        void swap( DomList& domlist );
    };
}

#endif
