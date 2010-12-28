// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "imgtoolbutton.h"

#include "icons/iconmanager.h"

#include "config/globalconf.h"

using namespace SKELETON;

ImgToolButton::ImgToolButton( const int id )
{
    m_img = Gtk::manage( new Gtk::Image( ICON::get_icon( id ) ) );
    set_icon_widget( *m_img );
}
