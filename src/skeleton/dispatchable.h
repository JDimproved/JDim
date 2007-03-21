// ライセンス: GPL2

//
// Dispatchクラス
//
// Core::DispatchManagerと組み合わせて使う。詳しくはCore::DispatchManagerの説明を見ること
//
// なお派生クラスのデストラクタの中からdispatchを呼ぶと落ちるので、特にスレッドを使用している
// 派生クラスのデストラクタの先頭に set_dispatchable( false ) を入れてdispatch不可にすること
//

#ifndef _DISPATCHABLE_H
#define _DISPATCHABLE_H

namespace CORE
{
    class DispatchManager;
}

namespace SKELETON
{
    class Dispatchable
    {
        friend class CORE::DispatchManager;
        bool m_dispatchable;

      public:

        Dispatchable();
        virtual ~Dispatchable();

      protected:

        void set_dispatchable( bool dispatchable );

        // dispacth()でDispatchManagerに登録されてcallback_disp()が呼び戻される
        //  cancel_dispatch()で呼び出しをキャンセルする
        virtual void callback_dispatch() = 0;

        void dispatch();
        void cancel_dispatch();
    };
}

#endif
