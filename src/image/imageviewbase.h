// ライセンス: GPL2

//
// 画像ビューのベースクラス
//

#ifndef _IMAGEVIEWBASE_H
#define _IMAGEVIEWBASE_H

#include "skeleton/view.h"
#include "skeleton/admin.h"

#include "jdlib/constptr.h"

#include <gtkmm.h>

#include <list>

namespace DBIMG
{
    class Img;
}


namespace IMAGE
{
    class ImageAreaBase;

    //
    // ビューのベースクラス
    //
    class ImageViewBase : public SKELETON::View
    {
        JDLIB::ConstPtr< DBIMG::Img > m_img;

        // Gtk::manage で作っているのでdeleteしなくても良い
        JDLIB::ConstPtr< ImageAreaBase > m_imagearea;

        bool m_loading;
        Gtk::EventBox m_event;
        bool m_dblclick;
        bool m_under_mouse;

        bool m_enable_menuslot;

      protected:

        JDLIB::ConstPtr< DBIMG::Img >& get_img(){ return  m_img;}

        JDLIB::ConstPtr< ImageAreaBase >& get_imagearea(){ return  m_imagearea; }
        void set_imagearea( ImageAreaBase* imagearea );
        void remove_imagearea();

        const bool loading() const{ return m_loading; }
        void set_loading( bool loading ){ m_loading = loading; }
        Gtk::EventBox& get_event(){ return  m_event; }

      public:

        ImageViewBase( const std::string& url, const std::string& arg1 = std::string(), const std::string& arg2 = std::string() );
        virtual ~ImageViewBase();

        const bool is_under_mouse() const { return m_under_mouse; }

        // コマンド
        virtual bool set_command( const std::string& command, const std::string& arg = std::string() );

        // SKELETON::View の関数のオーバロード
        virtual void reload();
        virtual void stop();
        virtual void redraw_view();
        virtual void close_view();
        virtual void delete_view();
        virtual void operate_view( const int& control );
        virtual void show_preference();

      protected:

        void setup_common();
        void set_image_to_buffer();

        virtual void activate_act_before_popupmenu( const std::string& url );

        virtual bool slot_button_press( GdkEventButton* event );
        bool slot_button_release( GdkEventButton* event );
        virtual bool slot_motion_notify( GdkEventMotion* event );
        bool slot_key_press( GdkEventKey* event );
        virtual bool slot_scroll_event( GdkEventScroll* event );
        bool slot_enter_notify_event( GdkEventCrossing* event );
        bool slot_leave_notify_event( GdkEventCrossing* event );

      private:

        virtual void show_status(){}
        virtual void add_image(){}
        virtual void switch_icon(){}

        void zoom_in_out( bool zoomin );

        void slot_move_head();
        void slot_move_tail();
        void slot_reload_force();
        void slot_cancel_mosaic();
        void slot_show_large_img();
        void slot_fit_win();
        void slot_zoom_in();
        void slot_zoom_out();
        void slot_resize_image( int size );
        void slot_lock();
        void slot_open_browser();
        void slot_open_ref();
        void slot_copy_url();
        void slot_save();
        void slot_save_all();
        void slot_favorite();
        void slot_toggle_protectimage();
        void slot_abone_img();
        void slot_close_other_views();
        void slot_close_left_views();
        void slot_close_right_views();
        void slot_close_all_views();
    };

}

#endif

