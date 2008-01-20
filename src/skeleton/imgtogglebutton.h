// ライセンス: GPL2

// 画像つきドグルボタン

#ifndef _IMGTOGGLEBUTTON_H
#define _IMGTOGGLEBUTTON_H

#include <gtkmm.h>
#include <string>

#include "icons/iconmanager.h"

namespace SKELETON
{
    class ImgToggleButton : public Gtk::ToggleButton
    {
        Gtk::Image* m_img;
        Gtk::Label m_label;
        Gtk::HBox m_hbox;

        void set( const std::string& label )
        {
            if( ! m_img ) return;

            if( label.empty() ) add( *m_img );
            else {
                m_label.set_text( label );
                m_hbox.pack_start( *m_img );
                m_hbox.pack_start( m_label, Gtk::PACK_SHRINK, 2 );
                add( m_hbox );
            }

            set_focus_on_click( false );
        }

      public:

        ImgToggleButton( const int id, const std::string label = std::string() ){

            m_img = Gtk::manage( new Gtk::Image( ICON::get_icon( id ) ) );
            set( label );
        }

        ImgToggleButton( const Gtk::StockID& stock_id,
                         const std::string label = std::string(),
                         const Gtk::BuiltinIconSize icon_size = Gtk::ICON_SIZE_MENU ){

            m_img = Gtk::manage( new Gtk::Image( stock_id, icon_size ) );
            set( label );
        }
    };
}

#endif
