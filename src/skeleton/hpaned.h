// ライセンス: GPL2
//
// HPaneeクラス
//

#ifndef _HPANED_H
#define _HPANED_H

#include <gtkmm.h>

#include "panecontrol.h"

namespace SKELETON
{
    class JDHPaned : public Gtk::HPaned
    {
        HPaneControl m_pctrl;

      public:

        JDHPaned();
        virtual ~JDHPaned(){}

        HPaneControl& get_ctrl(){ return m_pctrl; }

      protected:

        virtual void on_realize();
        virtual bool on_button_press_event( GdkEventButton* event );
        virtual bool on_button_release_event( GdkEventButton* event );
        virtual bool on_motion_notify_event( GdkEventMotion* event );
    };
}

#endif
