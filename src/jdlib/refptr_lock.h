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
        T* m_p{};

    public:

        void clear(){
            if( m_p ){
                m_p->unlock();
                m_p = nullptr;
            }
        }

        void set( T* p ){
            clear();
            m_p = p;
            if( m_p ) m_p->lock();
        }

        T* operator -> () noexcept { return m_p; }
        const T* operator -> () const noexcept { return m_p; }
        bool operator == ( const T* p ) const noexcept { return ( m_p == p ); }
        bool operator != ( const T* p ) const noexcept { return ( m_p != p ); }
        bool operator ! () const noexcept { return ( m_p == nullptr ); }
        operator bool () const noexcept { return ( m_p != nullptr ); }

        RefPtr_Lock& operator = ( const RefPtr_Lock& a ) { set( a.m_p ); return *this; }
        RefPtr_Lock& operator = ( T* p ) { set( p ); return *this; }

        RefPtr_Lock() noexcept = default;
        RefPtr_Lock( const RefPtr_Lock& a ) { set( a.m_p ); }
        RefPtr_Lock( T* p ) { set( p ); }

        virtual ~RefPtr_Lock(){ clear();}
    };
}

#endif
