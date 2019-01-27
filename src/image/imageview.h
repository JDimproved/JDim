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
        bool m_update_status;
        bool m_show_label;

        std::string m_status_local;

        int m_pre_width;
        int m_pre_height;
        int m_redraw_count;

        bool m_do_resizing;
        bool m_scrolled;

      public:
        ImageViewMain( const std::string& url );
        ~ImageViewMain();

        void clock_in() override;
        void show_view() override;
        void scroll_up() override;
        void scroll_down() override;
        void scroll_left() override;
        void scroll_right() override;

        bool operate_view( const int control ) override;

      protected:

        Gtk::Menu* get_popupmenu( const std::string& url ) override;

        bool slot_button_press( GdkEventButton* event ) override;
        bool slot_motion_notify( GdkEventMotion* event ) override;

      private:

        void set_status_local( const std::string& status ){ m_status_local = status; }
        const std::string& get_status_local(){ return m_status_local; }

        void show_status() override;
        void update_status() override;
        void add_tab_number();

        void show_instruct_diag();
        void set_label();
        void remove_label();
   };
}

#endif

