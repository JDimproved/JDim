// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "entry.h"

#include "control/controlid.h"

#include <gtk/gtkentry.h>

using namespace SKELETON;


// ボタン入力のフック
bool JDEntry::on_button_press_event( GdkEventButton* event )
{
#ifdef _DEBUG    
    std::cout << "JDEntry::on_button_press_event\n";
#endif

    m_sig_button_press.emit( event );

    return Gtk::Entry::on_button_press_event( event );
}


// キー入力のフック
bool JDEntry::on_key_press_event( GdkEventKey* event )
{
#ifdef _DEBUG    
    std::cout << "JDEntry::on_key_press_event key = " << event->keyval << std::endl;
#endif

    m_controlid = CONTROL::None;

    const guint key = event->keyval;
    const bool ctrl = ( event->state ) & GDK_CONTROL_MASK;
    const bool up = ( key == GDK_Up || ( ctrl && key == 'p' ) );
    const bool down = ( key == GDK_Down || ( ctrl && key == 'n' ) );
    const bool esc = ( key == GDK_Escape );

    // 上下をキャンセル
    // gtkentry.cpp からのハック。環境やバージョンによっては問題が出るかもしれないので注意
    if( up || down || esc ){

#if GTKMM_CHECK_VERSION(3,0,0)
        if( im_context_filter_keypress( event ) ) {
#ifdef _DEBUG
            std::cout << "gtk_im_context_filter_keypress\n";
#endif
            reset_im_context();
            return TRUE;
        }
#else
        GtkEntry *entry = gobj();
        if( gtk_im_context_filter_keypress( entry->im_context, event ) )
        {
#ifdef _DEBUG    
            std::cout << "gtk_im_context_filter_keypress\n";
#endif
            entry->need_im_reset = TRUE;

            return TRUE;
        }
#endif // GTKMM_CHECK_VERSION(3,0,0)
        else if( up || down ){

            if( up ) m_sig_operate.emit( CONTROL::Up );
            else m_sig_operate.emit( CONTROL::Down );

            return TRUE;
        }
    }

    m_sig_key_press.emit( event->keyval );
    m_controlid = m_control.key_press( event );

    return Gtk::Entry::on_key_press_event( event );
}


bool JDEntry::on_key_release_event( GdkEventKey* event )
{
    const bool ret = Gtk::Entry::on_key_release_event( event );

#ifdef _DEBUG    
    std::cout << "JDEntry::on_key_release_event id = " << m_controlid << std::endl;
#endif

    switch( m_controlid ){

        case CONTROL::DrawOutAnd:
        case CONTROL::SearchCache:
        case CONTROL::Cancel:
            m_sig_operate.emit( m_controlid );
            break;
    }

    m_controlid = CONTROL::None;

    return ret;
}
