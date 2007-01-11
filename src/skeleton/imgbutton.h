// ライセンス: GPL2

// 画像つきボタン

#ifndef _IMGBUTTON_H
#define _IMGBUTTON_H

#include <gtkmm.h>
#include <string>

#include "icons/iconmanager.h"

namespace SKELETON
{

    class ImgButton : public Gtk::Button
    {
        Gtk::Image* m_img;
        Gtk::Label m_label;
        Gtk::HBox m_hbox;

      public:

        ImgButton( const Gtk::StockID& stock_id, const std::string label = std::string() ){

            m_img = Gtk::manage( new Gtk::Image( stock_id, Gtk::ICON_SIZE_MENU ) );

            if( label.empty() ) add( *m_img );
            else {
                m_label.set_text( label );
                m_hbox.pack_start( *m_img );
                m_hbox.pack_start( m_label, Gtk::PACK_SHRINK, 2 );
                add( m_hbox );
            }

            set_focus_on_click( false );
        }
    };


    class ImgToggleButton : public Gtk::ToggleButton
    {
        Gtk::Image* m_img;

      public:

        ImgToggleButton(){}

        void set_img( const Gtk::StockID& stock_id ){

            m_img = Gtk::manage( new Gtk::Image( stock_id, Gtk::ICON_SIZE_MENU ) );
            add( *m_img );
            set_focus_on_click( false );
        }

        void set_icon( const int id ){

            m_img = Gtk::manage( new Gtk::Image( ICON::get_icon( id ) ) );
            add( *m_img );
            set_focus_on_click( false );
        }
    };



}

#endif
