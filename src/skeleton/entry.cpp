// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "entry.h"

#include "controlid.h"

using namespace SKELETON;


// キー入力のフック
bool JDEntry::on_key_release_event( GdkEventKey* event )
{
    bool ret = Gtk::Entry::on_key_release_event( event );
    int controlid = m_control.key_press( event );

#ifdef _DEBUG    
    std::cout << "JDEntry::on_key_release_event id = " << controlid << std::endl;
#endif

    switch( controlid ){

        case CONTROL::DrawOutAnd:
            m_sig_operate.emit( CONTROL::DrawOutAnd );
            break;

        case CONTROL::SearchCache:
            m_sig_operate.emit( CONTROL::SearchCache );
            break;

        case CONTROL::Cancel:
            m_sig_operate.emit( CONTROL::Cancel );
            break;
    }

    return ret;
}
