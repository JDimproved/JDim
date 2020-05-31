// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "imgbutton.h"

#include "icons/iconmanager.h"

using namespace SKELETON;

ImgButton::ImgButton( const int id, const std::string& label )
{
    m_img = Gtk::manage( new Gtk::Image( ICON::get_icon( id ) ) );
    set( label );
}


ImgButton::ImgButton( const Gtk::StockID& stock_id, const std::string& label,
                      const Gtk::BuiltinIconSize icon_size )
{
    m_img = Gtk::manage( new Gtk::Image( stock_id, icon_size ) );
    set( label );
}


ImgButton::~ImgButton() noexcept = default;


void ImgButton::set( const std::string& label )
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
