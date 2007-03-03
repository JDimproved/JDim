// ライセンス: GPL2
//
// キーボードフックしたentryクラス
//

#ifndef _ENTRY_H
#define _ENTRY_H

#include "control.h"

#include <gtkmm.h>

namespace SKELETON
{
    class JDEntry : public Gtk::Entry
    {
        typedef sigc::signal< void, int > SIG_OPERATE;

        SIG_OPERATE m_sig_operate;

        // 入力コントローラ
        CONTROL::Control m_control;

      public:

        SIG_OPERATE signal_operate(){ return m_sig_operate; }

        JDEntry(){}

        void add_mode( int mode ){ m_control.add_mode( mode ); }

      protected:

        // キー入力のフック
        virtual bool on_key_release_event( GdkEventKey* event );
    };
}

#endif
