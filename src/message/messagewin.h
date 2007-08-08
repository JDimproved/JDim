// ライセンス: GPL2

#ifndef _MESSAGEWIN_H
#define _MESSAGEWIN_H

#include <gtkmm.h>

#include "skeleton/window.h"

namespace MESSAGE
{
    class MessageWin : public SKELETON::JDWindow
    {
      public:

        MessageWin();
        virtual ~MessageWin();

      protected:

        virtual void switch_admin();

        virtual const int get_x_win();
        virtual const int get_y_win();
        virtual void set_x_win( int x );
        virtual void set_y_win( int y );

        virtual const int get_width_win();
        virtual const int get_height_win();
        virtual void set_width_win( int width );
        virtual void set_height_win( int height );

        virtual const bool is_focus_win();
        virtual void set_focus_win( bool set );

        virtual const bool is_maximized_win();
        virtual void set_maximized_win( bool set );

        virtual const bool is_iconified_win();
        virtual void set_iconified_win( bool set );

        virtual const bool is_shown_win();
        virtual void set_shown_win( bool set );

        virtual bool on_delete_event( GdkEventAny* event );
    };
}


#endif
