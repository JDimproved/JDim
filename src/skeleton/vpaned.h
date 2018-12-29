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

        JDVPaned( const int fixmode );
        ~JDVPaned() noexcept {}

        VPaneControl& get_ctrl(){ return m_pctrl; }

      protected:

        void on_realize() override;
        bool on_button_press_event( GdkEventButton* event ) override;
        bool on_button_release_event( GdkEventButton* event ) override;
        bool on_motion_notify_event( GdkEventMotion* event ) override;
        bool on_enter_notify_event( GdkEventCrossing* event ) override;
        bool on_leave_notify_event( GdkEventCrossing* event ) override;
    };
}

#endif
