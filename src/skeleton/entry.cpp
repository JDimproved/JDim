// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "entry.h"

#include "controlid.h"

#include "gtk/gtkentry.h"

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

    // 上下をキャンセル
    // gtkentry.cpp からのハック。環境やバージョンによっては問題が出るかもしれないので注意
    if( event->keyval == GDK_Up || event->keyval == GDK_Down ){

        GtkEntry *entry = gobj();
        if( gtk_im_context_filter_keypress( entry->im_context, event ) )
        {
#ifdef _DEBUG    
            std::cout << "gtk_im_context_filter_keypress\n";
#endif
            entry->need_im_reset = TRUE;
        }
        else{
            if( event->keyval == GDK_Up ) m_sig_operate.emit( CONTROL::Up );
            else m_sig_operate.emit( CONTROL::Down );
        }

        return TRUE;
    }

    m_sig_key_press.emit( event->keyval );

    return Gtk::Entry::on_key_press_event( event );
}


bool JDEntry::on_key_release_event( GdkEventKey* event )
{
    bool ret = Gtk::Entry::on_key_release_event( event );
    int controlid = m_control.key_press( event );

#ifdef _DEBUG    
    std::cout << "JDEntry::on_key_release_event id = " << controlid << std::endl;
#endif

    switch( controlid ){

        case CONTROL::DrawOutAnd:
        case CONTROL::SearchCache:
        case CONTROL::Cancel:
            m_sig_operate.emit( controlid );
            break;
    }

    return ret;
}
