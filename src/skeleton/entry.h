// ライセンス: GPL2
//
// キーボードフックしたentryクラス
//

#ifndef _ENTRY_H
#define _ENTRY_H

#include "control/control.h"

#include <gtkmm.h>

namespace SKELETON
{
    class JDEntry : public Gtk::Entry
    {
        typedef sigc::signal< void, GdkEventButton* > SIG_BUTTON_PRESS;
        typedef sigc::signal< void, int > SIG_KEY_PRESS;
        typedef sigc::signal< void, int > SIG_OPERATE;

        SIG_BUTTON_PRESS m_sig_button_press;
        SIG_KEY_PRESS m_sig_key_press;
        SIG_OPERATE m_sig_operate;

        // 入力コントローラ
        CONTROL::Control m_control;
        int m_controlid;

      public:

        SIG_BUTTON_PRESS signal_button_press(){ return m_sig_button_press; }
        SIG_KEY_PRESS signal_key_press(){ return m_sig_key_press; }
        SIG_OPERATE signal_operate(){ return m_sig_operate; }

        JDEntry() : Gtk::Entry(){}
        ~JDEntry() noexcept {}

        // CONTROL::Control のモード設定( controlid.h 参照 )
        // キー入力をフックして JDEntry::on_key_release_event() で SIG_OPERATE をemitする
        void add_mode( const int mode ){ m_control.add_mode( mode ); }

      protected:

        // マウスクリックのフック
        bool on_button_press_event( GdkEventButton* event ) override;

        // キー入力のフック
        bool on_key_press_event( GdkEventKey* event ) override;
        bool on_key_release_event( GdkEventKey* event ) override;
    };
}

#endif
