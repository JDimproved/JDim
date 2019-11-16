// ライセンス: GPL2

//
// ロック付きリファレンスクラスのテンプレート
//
//  SKELETON::Lockable を継承したクラスを RefPtr_Lock 経由で呼ぶことによってロックを掛ける
//

#ifndef _REFPTR_LOCK_H
#define _REFPTR_LOCK_H

namespace JDLIB
{
    template < typename T >
    class RefPtr_Lock
    {
        T *m_p;

    public:
    
        void clear(){
            if( m_p ){
                m_p->unlock();
                m_p = nullptr;
            }
        }

        void set( T *p ){
            clear();
            m_p = p;
            if( m_p ) m_p->lock();
        }        


        T* operator -> () noexcept { return m_p; }
        const T* operator -> () const noexcept { return m_p; }
        bool operator == ( const T *p ) const { return( m_p == p ); }
        bool operator != ( const T *p ) const { return( m_p != p ); }
        bool operator ! () const { return ( m_p == nullptr ); }
        operator bool () const { return ( m_p != nullptr ); }
    
        RefPtr_Lock< T >& operator = ( const RefPtr_Lock< T >& a ){ set( a.m_p ); return *this; }
        RefPtr_Lock< T >& operator = ( RefPtr_Lock< T >& a ){ set( a.m_p ); return *this; }
        RefPtr_Lock< T >& operator = ( const T *p ){ set( p ); return *this; }    
        RefPtr_Lock< T >& operator = ( T *p ){ set( p ); return *this; }
    
        RefPtr_Lock() : m_p (nullptr){}
        RefPtr_Lock( const RefPtr_Lock< T >& a ): m_p (nullptr){ set( a.m_p ); }
        RefPtr_Lock( RefPtr_Lock< T >& a ) : m_p (nullptr){ set( a.m_p ); }
        RefPtr_Lock( T *p ) : m_p (nullptr){ set( p ); }
        RefPtr_Lock( const T *p ) : m_p (nullptr){ set( p ); }
    
        virtual ~RefPtr_Lock(){ clear();}
    };
}

#endif
