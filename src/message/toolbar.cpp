// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "toolbar.h"
#include "messageadmin.h"

#include "icons/iconmanager.h"

#include "controlutil.h"
#include "controlid.h"
#include "session.h"
#include "global.h"

using namespace MESSAGE;



MessageToolBarBase::MessageToolBarBase() :
    SKELETON::ToolBar( MESSAGE::get_admin() ),
    m_button_preview( NULL )
{}


SKELETON::ImgButton* MessageToolBarBase::get_button_preview()
{
#ifdef _DEBUG
    std::cout << "MessageToolBarBase::get_button_preview\n";
#endif

    if( ! m_button_preview ){

        m_button_preview = Gtk::manage( new SKELETON::ImgButton( ICON::THREAD ) );
        m_button_preview->signal_clicked().connect( sigc::mem_fun( *this, &MessageToolBarBase::slot_toggle_preview ) );
    }

    return m_button_preview;
}


// プレビュー切り替え
void MessageToolBarBase::slot_toggle_preview()
{
#ifdef _DEBUG
    std::cout << "MessageToolBarBase::slot_toggle_preview\n";
#endif

    MESSAGE::get_admin()->set_command( "toggle_preview" );
}


//
// 書き込みボタンをフォーカス
//
void MessageToolBarBase::focus_writebutton()
{
    get_button_write()->grab_focus();
}


//////////////////////////////////


// 通常のツールバー

MessageToolBar::MessageToolBar() :
    MessageToolBarBase(),
    m_button_insert_draft( Gtk::Stock::OPEN ),
    m_button_undo( Gtk::Stock::UNDO ),
    m_entry_subject( false, "", "" )
{
    m_button_undo.signal_clicked().connect( sigc::mem_fun( *this, &MessageToolBar::slot_undo_clicked ) );
    m_button_insert_draft.signal_clicked().connect( sigc::mem_fun( *this, &MessageToolBar::slot_insert_draft_clicked ) );

    pack_buttons();
}


// ボタンのパッキング
void MessageToolBar::pack_buttons()
{
#ifdef _DEBUG
    std::cout << "MessageToolBar::pack_buttons\n";
#endif

    int num = 0;
    for(;;){
        int item = SESSION::get_item_msg_toolbar( num );
        if( item == ITEM_END ) break;
        switch( item ){

            case ITEM_PREVIEW:
                get_buttonbar().pack_start( *get_button_preview(), Gtk::PACK_SHRINK );
                set_tooltip( *get_button_preview(), CONTROL::get_label_motion( CONTROL::Preview )
                             + "\n\nタブ移動のショートカットでも表示の切り替えが可能\n\n"
                             + CONTROL::get_label_motion( CONTROL::TabRight ) + "\n\n"+ CONTROL::get_label_motion( CONTROL::TabLeft ) );
                break;

            case ITEM_WRITEMSG:
                get_buttonbar().pack_start( *get_button_write(), Gtk::PACK_SHRINK );
                set_tooltip( *get_button_write(), CONTROL::get_label_motion( CONTROL::ExecWrite ) + "\n\nTabキーで書き込みボタンにフォーカスを移すことも可能" );
                break;

            case ITEM_NAME:
                get_buttonbar().pack_start( m_entry_subject, Gtk::PACK_EXPAND_WIDGET, 2 );
                break;

            case ITEM_UNDO:
                get_buttonbar().pack_start( m_button_undo, Gtk::PACK_SHRINK );
                set_tooltip( m_button_undo, CONTROL::get_label_motion( CONTROL::UndoEdit ) );
                break;

            case ITEM_INSERTTEXT:
                get_buttonbar().pack_start( m_button_insert_draft, Gtk::PACK_SHRINK );
                set_tooltip( m_button_insert_draft, CONTROL::get_label_motion( CONTROL::InsertText ) );
                break;

            case ITEM_LOCK_MESSAGE:
                get_buttonbar().pack_start( *get_button_lock(), Gtk::PACK_SHRINK );
                set_tooltip( *get_button_lock(), CONTROL::get_label_motion( CONTROL::LockMessage ) );
                break;

            case ITEM_QUIT:
                get_buttonbar().pack_start( *get_button_close(), Gtk::PACK_SHRINK );
                set_tooltip( *get_button_close(), CONTROL::get_label_motion( CONTROL::CancelWrite ) );
                break;

            case ITEM_SEPARATOR:
                pack_separator();
                break;
        }
        ++num;
    }

    show_all_children();
}    


// undo ボタン
void MessageToolBar::slot_undo_clicked()
{
    MESSAGE::get_admin()->set_command( "undo_text" );
}


// 下書き挿入ボタン
void MessageToolBar::slot_insert_draft_clicked()
{
    MESSAGE::get_admin()->set_command( "insert_draft" );
}


///////////////////////


// プレビュー用のツールバー

MessageToolBarPreview::MessageToolBarPreview() :
    MessageToolBarBase()
{
    pack_buttons();
}



// ボタンのパッキング
void MessageToolBarPreview::pack_buttons()
{
#ifdef _DEBUG
    std::cout << "MessageToolBarPreview::pack_buttons\n";
#endif

    int num = 0;
    for(;;){
        int item = SESSION::get_item_msg_toolbar( num );
        if( item == ITEM_END ) break;
        switch( item ){

            case ITEM_PREVIEW:
                get_buttonbar().pack_start( *get_button_preview(), Gtk::PACK_SHRINK );
                set_tooltip( *get_button_preview(), "プレビューを閉じる" );
                break;

            case ITEM_NAME:
                get_buttonbar().pack_start( *get_label(), Gtk::PACK_EXPAND_WIDGET, 2 );
                break;

            case ITEM_WRITEMSG:
                get_buttonbar().pack_start( *get_button_write(), Gtk::PACK_SHRINK );
                set_tooltip( *get_button_write(), CONTROL::get_label_motion( CONTROL::ExecWrite ) + "\n\nTabキーで書き込みボタンにフォーカスを移すことも可能" );
                break;

            case ITEM_QUIT:
                get_buttonbar().pack_start( *get_button_close(), Gtk::PACK_SHRINK );
                set_tooltip( *get_button_close(), CONTROL::get_label_motion( CONTROL::CancelWrite ) );
                break;
        }
        ++num;
    }

    show_all_children();
}    
