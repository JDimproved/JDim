// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "vpaned.h"

using namespace SKELETON;


JDVPaned::JDVPaned( const int fixmode )
    : Gtk::Paned( Gtk::ORIENTATION_VERTICAL )
    , m_pctrl( *this, fixmode )
{}


JDVPaned::~JDVPaned() noexcept = default;


void JDVPaned::on_realize()
{
    Gtk::Paned::on_realize();
    m_pctrl.update_position();
}


bool JDVPaned::on_button_press_event( GdkEventButton* event )
{
    m_pctrl.button_press_event( event );
    return Gtk::Paned::on_button_press_event( event );
}


bool JDVPaned::on_button_release_event( GdkEventButton* event )
{
    m_pctrl.button_release_event( event );
    return Gtk::Paned::on_button_release_event( event );
}
    

bool JDVPaned::on_motion_notify_event( GdkEventMotion* event )
{
    m_pctrl.motion_notify_event( event );
    return Gtk::Paned::on_motion_notify_event( event );
}

bool JDVPaned::on_enter_notify_event( GdkEventCrossing* event )
{
    m_pctrl.enter_notify_event( event );
    return Gtk::Paned::on_enter_notify_event( event );
}

bool JDVPaned::on_leave_notify_event( GdkEventCrossing* event )
{
    m_pctrl.leave_notify_event( event );
    return Gtk::Paned::on_leave_notify_event( event );
}
