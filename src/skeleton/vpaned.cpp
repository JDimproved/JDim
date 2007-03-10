// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "vpaned.h"

using namespace SKELETON;


JDVPaned::JDVPaned( int fixmode )
    : Gtk::VPaned(),
      m_pctrl( *this, fixmode )
{}


void JDVPaned::on_realize()
{
    Gtk::VPaned::on_realize();
    m_pctrl.update_position();
}


bool JDVPaned::on_button_press_event( GdkEventButton* event )
{
    m_pctrl.button_press_event( event );
    return Gtk::VPaned::on_button_press_event( event );
}


bool JDVPaned::on_button_release_event( GdkEventButton* event )
{
    m_pctrl.button_release_event( event );
    return Gtk::VPaned::on_button_release_event( event );
}
    

bool JDVPaned::on_motion_notify_event( GdkEventMotion* event )
{
    m_pctrl.motion_notify_event( event );
    return Gtk::VPaned::on_motion_notify_event( event );
}
