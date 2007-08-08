// ライセンス: GPL2

//
// 画像ウィンドウ
//

#ifndef _IMAGEWIN_H
#define _IMAGEWIN_H

#include <gtkmm.h>

#include "skeleton/window.h"

namespace IMAGE
{
    class ImageWin : public SKELETON::JDWindow
    {
        Gtk::Widget* m_tab;

      public:

        ImageWin();
        virtual ~ImageWin();

        void pack_remove_tab( bool unpack, Widget& tab );

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
    };
}


#endif
