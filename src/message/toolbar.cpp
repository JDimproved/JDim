// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "toolbar.h"

#include "icons/iconmanager.h"

#include "controlutil.h"
#include "controlid.h"
#include "session.h"
#include "global.h"

using namespace MESSAGE;

MessageToolBar::MessageToolBar( const std::string& boardname ) :
    SKELETON::ToolBar(),
    m_button_write( ICON::WRITE ),
    m_button_open( Gtk::Stock::OPEN ),
    m_button_undo( Gtk::Stock::UNDO ),
    m_button_not_close( Gtk::Stock::CANCEL ),
    m_button_preview( ICON::THREAD ),
    m_entry_subject( false, " [ " + boardname + " ]  ", "" )
{
    m_button_not_close.set_active( ! SESSION::get_close_mes() );

    set_tooltip( m_button_write, CONTROL::get_label_motion( CONTROL::ExecWrite ) + "\n\nTabキーで書き込みボタンにフォーカスを移すことも可能" );
    set_tooltip( get_close_button(), CONTROL::get_label_motion( CONTROL::CancelWrite ) );
    set_tooltip( m_button_open, CONTROL::get_label_motion( CONTROL::InsertText ) );
    set_tooltip( m_button_undo, CONTROL::get_label_motion( CONTROL::UndoEdit ) );
    set_tooltip( m_button_not_close, CONTROL::get_label_motion( CONTROL::NotClose ) );
    set_tooltip( m_button_preview, CONTROL::get_label_motion( CONTROL::Preview )
                 + "\n\nタブ移動のショートカットでも表示の切り替えが可能\n\n"
                 + CONTROL::get_label_motion( CONTROL::TabRight ) + "\n\n"+ CONTROL::get_label_motion( CONTROL::TabLeft ) );

    pack_buttons();
}


// ボタンのパッキング
void MessageToolBar::pack_buttons()
{
    int num = 0;
    for(;;){
        int item = SESSION::get_item_msg_toolbar( num );
        if( item == ITEM_END ) break;
        switch( item ){
            case ITEM_PREVIEW: get_buttonbar().pack_start( m_button_preview, Gtk::PACK_SHRINK ); break;
            case ITEM_WRITEMSG:  get_buttonbar().pack_start( m_button_write, Gtk::PACK_SHRINK ); break;
            case ITEM_NAME: get_buttonbar().pack_start( m_entry_subject, Gtk::PACK_EXPAND_WIDGET, 2 ); break;
            case ITEM_UNDO: get_buttonbar().pack_start( m_button_undo, Gtk::PACK_SHRINK ); break;
            case ITEM_INSERTTEXT: get_buttonbar().pack_start( m_button_open, Gtk::PACK_SHRINK ); break;
            case ITEM_NOTCLOSE: get_buttonbar().pack_start( m_button_not_close, Gtk::PACK_SHRINK ); break;
            case ITEM_QUIT: get_buttonbar().pack_start( get_close_button(), Gtk::PACK_SHRINK ); break;
        }
        ++num;
    }
}    

void MessageToolBar::unpack_buttons()
{
    get_buttonbar().remove( m_button_preview );
    get_buttonbar().remove( m_button_write );
    get_buttonbar().remove( m_entry_subject );
    get_buttonbar().remove( m_button_undo );
    get_buttonbar().remove( m_button_open );
    get_buttonbar().remove( m_button_not_close );
    get_buttonbar().remove( get_close_button() );
}
