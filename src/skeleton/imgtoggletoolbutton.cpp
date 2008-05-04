// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "imgtoggletoolbutton.h"

#include "icons/iconmanager.h"

using namespace SKELETON;

ImgToggleToolButton::ImgToggleToolButton( const int id )
{
    m_img = Gtk::manage( new Gtk::Image( ICON::get_icon( id ) ) );
    set_icon_widget( *m_img );
}


ImgToggleToolButton::ImgToggleToolButton( const Gtk::StockID& stock_id, const Gtk::BuiltinIconSize icon_size )
{
    // (注) 直接 Gtk::Image( stock_id, icon_size ) とするとアイコンが大きくなってしまう
    m_img = Gtk::manage( new Gtk::Image( Gtk::Widget::render_icon( stock_id, icon_size ) ) );
    set_icon_widget( *m_img );
}
