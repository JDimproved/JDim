// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "toolbar.h"

#include "controlutil.h"
#include "controlid.h"

#include <list>

using namespace SKELETON;


ToolBar::ToolBar() :
    m_toolbar_shown( false ),
    m_button_close( Gtk::Stock::CLOSE )
{
    signal_realize().connect( sigc::mem_fun(*this, &ToolBar::slot_vbox_realize ) );
    signal_style_changed().connect( sigc::mem_fun(*this, &ToolBar::slot_vbox_style_changed ) );

    set_tooltip( m_button_close, CONTROL::get_label_motion( CONTROL::Quit ) );

    m_buttonbar.set_border_width( 1 );
    m_scrwin.add( m_buttonbar );
    m_scrwin.set_policy( Gtk::POLICY_NEVER, Gtk::POLICY_NEVER );

    set_size_request( 8 );
}
        

bool ToolBar::is_empty()
{
    return ( ! m_buttonbar.get_children().size() );
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


// 更新
void ToolBar::update()
{
    bool empty = is_empty();

    unpack_buttons();
    pack_buttons();

    // ツールバーの中身が空の場合は
    // もう一度unpackとpackを繰り返さないと表示されないようだ
    if( empty ){
        unpack_buttons();
        pack_buttons();
    }
}


// ボタンのアンパック
void ToolBar::unpack_buttons()
{
    std::list< Gtk::Widget* > lists = m_buttonbar.get_children();
    std::list< Gtk::Widget* >::iterator it = lists.begin();
    for( ; it != lists.end(); ++it ){
        m_buttonbar.remove( *(*it) );
        if( dynamic_cast< Gtk::VSeparator* >( *it ) ) delete *it;
    }
}


// 区切り追加
void ToolBar::pack_separator()
{
    Gtk::VSeparator *sep = Gtk::manage( new Gtk::VSeparator() ); // delete は unpack_buttons() で行う
    m_buttonbar.pack_start( *sep, Gtk::PACK_SHRINK );
    sep->show();
}
