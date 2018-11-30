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
        ~ImageWin();

        void pack_remove_tab( bool unpack, Widget& tab );

      protected:

        void switch_admin() override;

        const int get_x_win() override;
        const int get_y_win() override;
        void set_x_win( const int x ) override;
        void set_y_win( const int y ) override;

        const int get_width_win() override;
        const int get_height_win() override;
        void set_width_win( const int width ) override;
        void set_height_win( const int height ) override;

        const bool is_focus_win() override;
        void set_focus_win( const bool set ) override;

        const bool is_maximized_win() override;
        void set_maximized_win( const bool set ) override;

        const bool is_iconified_win() override;
        void set_iconified_win( const bool set ) override;

        const bool is_full_win() override { return false; }
        void set_full_win( const bool set ) override {}

        const bool is_shown_win() override;
        void set_shown_win( const bool set ) override;
    };
}


#endif
