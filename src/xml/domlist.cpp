// License GPL2

//#define _DEBUG
#include "jddebug.h"

#include "domlist.h"


using namespace XML;


DomList::DomList()
{

}

DomList::~DomList()
{

}

// std::list< Dom* > が代入された場合
DomList& DomList::operator =( const std::list< Dom* >& list )
{
    m_list = list;

    return *this;
}

// 添字によるアクセス
Dom* DomList::operator []( const unsigned int n )
{
    if( m_list.empty() || n > m_list.size() ) return 0;

    size_t count = 0;
    std::list< Dom* >::iterator it = m_list.begin();
    while( it != m_list.end() )
    {
        if( count == n ) return *it;
        ++count;
        ++it;
    }

    return 0;
}


// 以下 std::list の主なメンバ関数

std::list< Dom* >::iterator DomList::begin()
{
    return m_list.begin();
}

std::list< Dom* >::reverse_iterator DomList::rbegin()
{
    return m_list.rbegin();
}

std::list< Dom* >::iterator DomList::end()
{
    return m_list.end();
}

std::list< Dom* >::reverse_iterator DomList::rend()
{
    return m_list.rend();
}

size_t DomList::size() const
{
    return m_list.size();
}

size_t DomList::max_size() const
{
    return m_list.max_size();
}

bool DomList::empty() const
{
    return m_list.empty();
}

// 参照ではなくポインタを返す
Dom* DomList::front()
{
    if( m_list.empty() ) return 0;

    return *m_list.begin();
}

// 参照ではなくポインタを返す
Dom* DomList::back()
{
    if( m_list.empty() ) return 0;

    return *m_list.end();
}

void DomList::push_front( Dom* dom )
{
    m_list.push_front( dom );
}

void DomList::push_back( Dom* dom )
{
    m_list.push_back( dom );
}

void DomList::pop_front()
{
    m_list.pop_front();
}

void DomList::pop_back()
{
    m_list.pop_back();
}

std::list< Dom* >::iterator DomList::insert( std::list< Dom* >::iterator it, Dom* dom )
{
    return m_list.insert( it, dom );
}

std::list< Dom* >::iterator DomList::erase( std::list< Dom* >::iterator it )
{
    return erase( it );
}

void DomList::clear()
{
    m_list.clear();
}

void DomList::splice( std::list< Dom* >::iterator it, DomList& domlist  )
{
    m_list.splice( it, domlist.m_list );
}

void DomList::remove( Dom* dom )
{
    m_list.remove( dom );
}

void DomList::unique()
{
    m_list.unique();
}

void DomList::merge( DomList& domlist )
{
    m_list.merge( domlist.m_list );

}

void DomList::sort()
{
    m_list.sort();
}

void DomList::reverce()
{
    m_list.reverse();
}

void DomList::swap( DomList& domlist )
{
    m_list.swap( domlist.m_list );
}

