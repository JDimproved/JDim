// ライセンス: GPL2

//
// Dispatchクラス
//
// Core::DispatchManagerと組み合わせて使う。詳しくはCore::DispatchManagerの説明を見ること
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

        // dispacth()でDispatchManagerに登録されてcallback_disp()が呼び戻される
        //  cancel_dispatch()で呼び出しをキャンセルする
        virtual void callback_dispatch() = 0;

        void set_dispatchable( bool dispatchable ){ m_dispatchable = dispatchable; }

        void dispatch( SKELETON::Dispatchable* dest );
        void cancel_dispatch( SKELETON::Dispatchable* dest );

        void dispatch();
        void cancel_dispatch();
    };
}

#endif
