// ライセンス: GPL2

//
// Dispatchの管理クラス
//
// Glib::Dispatcher::emit()後に呼出先がdeleteされると segmentation fault で落ちるので
// Dispatchを一元管理して安全にDispatchする
//

#ifndef _DISPATCHMANAGER_H
#define _DISPATCHMANAGER_H

#include <gtkmm.h>
#include <list>

namespace SKELETON
{
   class Dispatchable;
}


namespace CORE
{
    class DispatchManagerBase
    {
        std::list< SKELETON::Dispatchable* > m_children;

      public:

        DispatchManagerBase();
        virtual ~DispatchManagerBase();

        void add( SKELETON::Dispatchable* child );
        void remove( SKELETON::Dispatchable* child );

      protected:

        void slot_dispatch();

      private:

        virtual void emit(){}
    };


    //////////////////////////////////////

    // メインループ用マネージャ
    class DispatchManager : public DispatchManagerBase
    {
        Glib::Dispatcher m_dispatch;

      public:

        DispatchManager();
        virtual ~DispatchManager(){}

      private:

        virtual void emit() { m_dispatch.emit(); }
    };


    //////////////////////////////////////

    // ダイアログ用マネージャ
    // ダイアログでは Glib::Dispatcher が動かないのでタイマーを使う
    // 擬似的にdispatchする
    class DispatchManagerForDialog : public DispatchManagerBase
    {
        sigc::connection m_conn;

      public:

        DispatchManagerForDialog();
        virtual ~DispatchManagerForDialog();

      private:

        virtual bool slot_timeout( int timer_number );
    };


    //////////////////////////////////////


    CORE::DispatchManagerBase* get_dispmanager();
    void delete_dispatchmanager();

    // ダイアログを開いたときに下の2つの関数を呼び出すこと
    // skeleton/prefdiag.cpp を参照せよ
    void enable_dialog_dispmanager();
    void disable_dialog_dispmanager();
}

#endif
