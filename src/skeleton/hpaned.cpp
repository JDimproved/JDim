// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "hpaned.h"

using namespace SKELETON;


JDHPaned::JDHPaned( const int fixmode )
    : Gtk::HPaned(),
      m_pctrl( *this, fixmode )
{}


JDHPaned::~JDHPaned() noexcept = default;


void JDHPaned::on_realize()
{
    Gtk::HPaned::on_realize();
    m_pctrl.update_position();
}


bool JDHPaned::on_button_press_event( GdkEventButton* event )
{
    m_pctrl.button_press_event( event );
    return Gtk::HPaned::on_button_press_event( event );
}


bool JDHPaned::on_button_release_event( GdkEventButton* event )
{
    m_pctrl.button_release_event( event );   
    return Gtk::HPaned::on_button_release_event( event );
}


bool JDHPaned::on_motion_notify_event( GdkEventMotion* event )
{
    m_pctrl.motion_notify_event( event );
    return Gtk::HPaned::on_motion_notify_event( event );
}


bool JDHPaned::on_enter_notify_event( GdkEventCrossing* event )
{
    m_pctrl.enter_notify_event( event );
    return Gtk::HPaned::on_enter_notify_event( event );
}

bool JDHPaned::on_leave_notify_event( GdkEventCrossing* event )
{
    m_pctrl.leave_notify_event( event );
    return Gtk::HPaned::on_leave_notify_event( event );
}
