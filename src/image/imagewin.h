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
        Gtk::Widget* m_tab{};

      public:

        ImageWin();
        ~ImageWin() override;

        void pack_remove_tab( bool unpack, Widget& tab );

      protected:

        void switch_admin() override;

        int get_x_win() const override;
        int get_y_win() const override;
        void set_x_win( const int x ) override;
        void set_y_win( const int y ) override;

        int get_width_win() const override;
        int get_height_win() const override;
        void set_width_win( const int width ) override;
        void set_height_win( const int height ) override;

        bool is_focus_win() const override;
        void set_focus_win( const bool set ) override;

        bool is_maximized_win() const override;
        void set_maximized_win( const bool set ) override;

        bool is_iconified_win() const override;
        void set_iconified_win( const bool set ) override;

        bool is_full_win() const override { return false; }
        void set_full_win( const bool set ) override {}

        bool is_shown_win() const override;
        void set_shown_win( const bool set ) override;
    };
}


#endif
