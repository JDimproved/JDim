// ライセンス: GPL2

#ifndef _ICONPOPUP_H
#define _ICONPOPUP_H

#include <gtkmm.h>

#include "icons/iconmanager.h"

namespace SKELETON
{
    class IconPopup : public Gtk::Window
    {
        Glib::RefPtr< Gdk::Pixbuf > m_pixbuf;
        Gtk::Image m_img;

      public:

      IconPopup( const int icon_id ) : Gtk::Window( Gtk::WINDOW_POPUP ){

            Glib::RefPtr< Gdk::Pixmap > pixmap;
            Glib::RefPtr< Gdk::Bitmap > bitmap;

            m_pixbuf = ICON::get_icon( icon_id );
            m_img.set( m_pixbuf );

            m_pixbuf->render_pixmap_and_mask( pixmap, bitmap, 255 );
            shape_combine_mask( bitmap, 0, 0 );

            add( m_img );
            show_all_children();
        }

        int get_img_width(){ return m_pixbuf->get_width(); }
        int get_img_height(){ return m_pixbuf->get_height(); }
    };

}

#endif
