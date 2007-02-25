// ライセンス: GPL2
//
// VPandeクラス
//

#ifndef _VPANED_H
#define _VPANED_H

#include <gtkmm.h>

namespace SKELETON
{
    enum
    {
        VPANE_NORMAL = 0,
        VPANE_MAX_PAGE1,
        VPANE_MAX_PAGE2
    };

    class JDVPaned : public Gtk::VPaned
    {
        bool m_clicked;
        bool m_drag;

        int m_mode;
        int m_pos;

        int m_pre_height;

      public:

        JDVPaned();
        ~JDVPaned();

        void clock_in();

        int get_position();
        void set_position( int position );

        // unpack = true の時取り除く
        void add_remove1( bool unpack, Gtk::Widget& child );
        void add_remove2( bool unpack, Gtk::Widget& child );

        // ページ最大化切り替え
        // page には VPANE_NORMAL などを指定
        void toggle_maximize( int page );

      protected:
        virtual bool on_button_press_event( GdkEventButton* event );
        virtual bool on_button_release_event( GdkEventButton* event );
        virtual bool on_motion_notify_event( GdkEventMotion* event );
    };
}

#endif
