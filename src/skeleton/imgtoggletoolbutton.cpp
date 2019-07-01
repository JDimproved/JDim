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


ImgToggleToolButton::~ImgToggleToolButton() noexcept = default;
