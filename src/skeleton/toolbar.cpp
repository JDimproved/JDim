// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "toolbar.h"

#include "controlutil.h"
#include "controlid.h"

using namespace SKELETON;


ToolBar::ToolBar() :
    m_toolbar_shown( false ),
    m_button_close( Gtk::Stock::CLOSE )
{
    signal_realize().connect( sigc::mem_fun(*this, &ToolBar::slot_vbox_realize ) );

    set_tooltip( m_button_close, CONTROL::get_label_motion( CONTROL::Quit ) );

    m_buttonbar.set_border_width( 1 );
    m_scrwin.add( m_buttonbar );
    m_scrwin.set_policy( Gtk::POLICY_NEVER, Gtk::POLICY_NEVER );

    set_size_request( 8 );
}
        

// ツールバーを表示
void ToolBar::show_toolbar()
{
    if( ! m_toolbar_shown ){
        pack_start( m_scrwin, Gtk::PACK_SHRINK );
        show_all_children();
        m_toolbar_shown = true;
    }
}


// ツールバーを隠す
void ToolBar::hide_toolbar()
{
    if( m_toolbar_shown ){
        remove( m_scrwin );
        show_all_children();
        m_toolbar_shown = false;
    }
}


// タブのロック
void ToolBar::lock()
{
    m_button_close.set_sensitive( false );
}


// タブのアンロック
void ToolBar::unlock()
{
    m_button_close.set_sensitive( true );
}
