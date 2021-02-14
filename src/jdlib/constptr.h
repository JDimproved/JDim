// ライセンス: GPL2

//
// constなスマートポインタ
//

#ifndef _CONSTPTR_H
#define _CONSTPTR_H

namespace JDLIB
{
    template < typename T >
    class ConstPtr
    {
        T* m_p{};

    public:

        T* operator -> () noexcept { return m_p; }
        const T* operator -> () const noexcept { return m_p; }
        bool operator == ( const T* p ) const noexcept { return ( m_p == p ); }
        bool operator != ( const T* p ) const noexcept { return ( m_p != p ); }
        bool operator ! () const noexcept { return ( m_p == nullptr ); }
        operator bool () const noexcept { return ( m_p != nullptr ); }
        T& operator * () { return *m_p; }
        const T& operator * () const { return *m_p; }
        T& operator [] ( int i ) { return m_p[i]; }
        const T& operator [] ( int i ) const { return m_p[i]; }

        ConstPtr& operator = ( const ConstPtr& a ) { m_p = a.m_p; return *this; }
        ConstPtr& operator = ( T* p ) { m_p = p; return *this; }

        void reset() { m_p = nullptr; }

        // clear は deleteも実行
        void clear(){
            if( m_p ) delete m_p;
            reset();
        }

        ConstPtr() noexcept = default;
        ConstPtr( T* p ) : m_p( p ) {}
    };
}

#endif
