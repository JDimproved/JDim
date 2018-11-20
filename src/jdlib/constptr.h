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
        T *m_p;

    public:

        T* operator -> () const noexcept { return m_p; }
        bool operator == ( const T *p ) const { return( m_p == p ); }
        bool operator != ( const T *p ) const { return( m_p != p ); }
        bool operator ! () const { return ( m_p == NULL ); }
        T& operator * () const { return *m_p; }
        operator bool () const { return ( m_p != NULL ); }
        T& operator [] ( const int i ){ return m_p[ i ]; }

        ConstPtr< T >& operator = ( const ConstPtr< T >& a ){ m_p = a.m_p; return *this; }
        ConstPtr< T >& operator = ( ConstPtr< T >& a ){ m_p = a.m_p; return *this; }
        ConstPtr< T >& operator = ( const T *p ){ m_p = p; return *this; }    
        ConstPtr< T >& operator = ( T *p ){ m_p = p; return *this; }

        void reset() { m_p = NULL; }

        // clear は deleteも実行
        void clear(){
            if( m_p ) delete m_p;
            reset();
        }

        ConstPtr() : m_p (0){}
        ConstPtr( T *p ) : m_p (p){}
        ConstPtr( const T *p ) : m_p (p){}
    };
}

#endif
