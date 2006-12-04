// ライセンス: GPL2

//
// 画像ビュークラス
//

#ifndef _IMAGEVIEW_H
#define _IMAGEVIEW_H

#include "imageviewbase.h"

namespace IMAGE
{
    class ImageViewMain : public ImageViewBase
    {
        Gtk::ScrolledWindow* m_scrwin;
        Gtk::Label m_label;
        gdouble m_x_motion;
        gdouble m_y_motion;
        size_t m_length_prev;
        bool m_show_status;
        bool m_show_label;

      public:
        ImageViewMain( const std::string& url );

        virtual void clock_in();
        virtual void show_view();
        virtual void scroll_left();
        virtual void scroll_right();

      protected:
        virtual void show_status();

        virtual Gtk::Menu* get_popupmenu( const std::string& url );

        virtual bool slot_button_press( GdkEventButton* event );
        virtual bool slot_motion_notify( GdkEventMotion* event );

      private:
        void set_label();
        void remove_label();
   };
}

#endif

