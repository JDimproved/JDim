// ライセンス: GPL2
//
// VPandeクラス
//

#ifndef _VPANED_H
#define _VPANED_H

#include <gtkmm.h>

namespace SKELETON
{
    class JDVPaned : public Gtk::VPaned
    {
        bool m_clicked;
        bool m_drag;
        int m_pos;

      public:

        JDVPaned();
        ~JDVPaned();

        int get_position();
        void set_position( int position );

        // unpack = true の時取り除く
        void add_remove1( bool unpack, Gtk::Widget& child );
        void add_remove2( bool unpack, Gtk::Widget& child );

        // ページ最大化切り替え
        // page = 0 の時は元に戻す
        void toggle_maximize( int page );

      protected:
        virtual bool on_button_press_event( GdkEventButton* event );
        virtual bool on_button_release_event( GdkEventButton* event );
        virtual bool on_motion_notify_event( GdkEventMotion* event );
    };
}

#endif
