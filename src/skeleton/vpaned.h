// ライセンス: GPL2
//
// VPanedクラス
//

#ifndef _VPANED_H
#define _VPANED_H

#include <gtkmm.h>

#include "panecontrol.h"

namespace SKELETON
{
    class JDVPaned : public Gtk::VPaned
    {
        VPaneControl m_pctrl;

      public:

        JDVPaned();
        virtual ~JDVPaned(){}

        VPaneControl& get_ctrl(){ return m_pctrl; }

      protected:

        virtual bool on_button_press_event( GdkEventButton* event );
        virtual bool on_button_release_event( GdkEventButton* event );
        virtual bool on_motion_notify_event( GdkEventMotion* event );
    };
}

#endif
