// ライセンス: 最新のGPL

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

      protected:
        virtual void show_status();
        virtual void show_popupmenu();

        virtual bool slot_button_press_imagearea( GdkEventButton* event );
        virtual bool slot_motion_notify_imagearea( GdkEventMotion* event );

      private:
        void set_label();
        void remove_label();
   };
}

#endif

