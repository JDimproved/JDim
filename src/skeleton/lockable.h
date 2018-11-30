// ライセンス: GPL2

//
// ロック可能クラス (  RefPtr と組み合わせて使う )
//
//  Lockable を継承したクラスを JDLIB::RefPtr_Lock 経由で呼ぶことによってロックを掛ける
//  ロックが外れたら unlook_impl が呼ばれる
//

#ifndef _LOCKABLE_H
#define _LOCKABLE_H

namespace SKELETON
{
    class Lockable
    {
        int m_lock;

    public:

        Lockable() :m_lock( 0 ){}
        
        virtual ~Lockable() noexcept {}

        const int get_lock() const { return m_lock; }
    
        void lock(){ ++m_lock; }
        void unlock(){
            --m_lock;
            if( m_lock == 0 ) unlock_impl();
        }

      private:

        virtual void unlock_impl(){}
    };
}

#endif
