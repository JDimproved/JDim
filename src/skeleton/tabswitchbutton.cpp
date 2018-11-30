// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "tabswitchbutton.h"
#include "dragnote.h"

using namespace SKELETON;


TabSwitchButton::TabSwitchButton( DragableNoteBook* parent )
    : Gtk::Notebook(),
      m_parent( parent ),
      m_arrow( Gtk::ARROW_DOWN, Gtk::SHADOW_NONE ),
      m_shown( false )
{
    set_border_width( 0 );

    m_button.add( m_arrow );
    m_button.show_all_children();
    m_button.set_relief( Gtk::RELIEF_NONE );
    m_button.set_focus_on_click( false );

    // フォーカス時にボタンの枠がはみ出さないようにする
    Glib::RefPtr< Gtk::RcStyle > rcst = m_button.get_modifier_style();
    rcst->set_ythickness( 0 );
    m_button.modify_style( rcst );

    m_vbox.pack_start( m_button, Gtk::PACK_SHRINK );

    set_show_tabs( false );
}


TabSwitchButton::~TabSwitchButton() noexcept
{}


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


//
// 描画イベント
//
// 自前でビュー領域の枠を描画する
//
bool TabSwitchButton::on_expose_event( GdkEventExpose* event )
{
    if( ! m_shown ) return Gtk::Notebook::on_expose_event( event );

    // 枠描画
    m_parent->draw_box( this, event );

    // ボタン描画
    propagate_expose( m_vbox, event );

    return true;
}
