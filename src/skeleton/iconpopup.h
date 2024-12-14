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

        explicit IconPopup( const int icon_id )
            : Gtk::Window( Gtk::WINDOW_POPUP )
            , m_pixbuf{ ICON::get_pixbuf( icon_id ) }
        {
            m_img.set( m_pixbuf );

            // NOTE: アルファチャンネルが利用できない環境では背景を透過できない
            set_decorated( false );
            set_app_paintable( true );
            auto screen = get_screen();
            auto visual = screen->get_rgba_visual();
            if( visual && screen->is_composited() ) {
                gtk_widget_set_visual( GTK_WIDGET( gobj() ), visual->gobj() );
            }

            add( m_img );
            show_all_children();
        }

        int get_img_width() const { return m_pixbuf->get_width(); }
        int get_img_height() const { return m_pixbuf->get_height(); }
    };

}

#endif
