// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "tabswitchbutton.h"


using namespace SKELETON;


TabSwitchButton::TabSwitchButton( DragableNoteBook* )
    : Gtk::Notebook()
{
    set_border_width( 0 );

    m_button.add( m_arrow );
    m_button.show_all_children();
    m_button.set_relief( Gtk::RELIEF_NONE );
    m_button.set_focus_on_click( false );

    // フォーカス時にボタンの枠がはみ出さないようにする
    m_button.set_margin_top( 0 );
    m_button.set_margin_bottom( 0 );

    m_vbox.pack_start( m_button, Gtk::PACK_SHRINK );

    set_show_tabs( false );
}


TabSwitchButton::~TabSwitchButton() noexcept = default;


void TabSwitchButton::show_button()
{
    if( m_shown ) return;
    append_page( m_vbox );
    show_all_children();
    m_shown = true;
}


void TabSwitchButton::hide_button()
{
    if( ! m_shown ) return;
    remove_page( m_vbox );
    m_shown = false;
}
