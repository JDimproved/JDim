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
    class DispatchManager
    {
        Glib::Dispatcher m_dispatch;

        std::list< SKELETON::Dispatchable* > m_children;

      public:

        DispatchManager();
        virtual ~DispatchManager();

        Glib::Dispatcher& get_dispatch(){ return m_dispatch; }

        void add( SKELETON::Dispatchable* child );
        void remove( SKELETON::Dispatchable* child );

      private:

        void slot_dispatch();
    };


    CORE::DispatchManager* get_dispmanager();
    void delete_dispatchmanager();
}

#endif
