// ライセンス: 最新のGPL
//
// HPandeクラス
//
// 仕切りをクリックすると閉じるhpaned
//

#ifndef _HPANED_H
#define _HPANED_H

#include <gtkmm.h>

namespace SKELETON
{
    class JDHPaned : public Gtk::HPaned
    {
        bool m_clicked;
        bool m_drag;
        int m_pos;

      public:
        JDHPaned();
        ~JDHPaned();

        int get_position();
        void set_position( int position );

      protected:
        virtual bool on_button_press_event( GdkEventButton* event );
        virtual bool on_button_release_event( GdkEventButton* event );
        virtual bool on_motion_notify_event( GdkEventMotion* event );
    };
}

#endif
