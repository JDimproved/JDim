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

        SIG_PANE_MODECHANGED& sig_pane_modechanged() { return m_pctrl.sig_pane_modechanged(); }
        void clock_in(){ m_pctrl.clock_in(); }

        // セパレータの位置の取得やセット
        int get_position(){ return m_pctrl.get_position(); }
        void set_position( int position ){ m_pctrl.set_position( position ); }

        // PANE_MAX_PAGE1 などを指定
        void set_mode( int mode ){ m_pctrl.set_mode( mode ); }
        const int get_mode() const { return m_pctrl.get_mode(); }

        // PANE_CLICK_FOLD_PAGE1 などを指定
        void set_click_fold( int mode ){ m_pctrl.set_click_fold( mode ); }
        const int get_click_fold() const { return m_pctrl.get_click_fold(); }

        // unpack = true の時取り除く
        void add_remove1( bool unpack, Gtk::Widget& child ){ m_pctrl.add_remove1( unpack, child ); }
        void add_remove2( bool unpack, Gtk::Widget& child ){ m_pctrl.add_remove2( unpack, child ); }

      protected:

        virtual bool on_button_press_event( GdkEventButton* event );
        virtual bool on_button_release_event( GdkEventButton* event );
        virtual bool on_motion_notify_event( GdkEventMotion* event );
    };
}

#endif
