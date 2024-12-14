// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "toolbar.h"
#include "messageadmin.h"

#include "skeleton/imgtoolbutton.h"

#include "icons/iconmanager.h"

#include "control/controlutil.h"
#include "control/controlid.h"

#include "session.h"
#include "global.h"

using namespace MESSAGE;



MessageToolBarBase::MessageToolBarBase()
    : SKELETON::ToolBar( MESSAGE::get_admin() )
    , m_enable_slot( true )
{}


Gtk::ToggleToolButton* MessageToolBarBase::get_button_preview()
{
#ifdef _DEBUG
    std::cout << "MessageToolBarBase::get_button_preview\n";
#endif

    if( ! m_button_preview ){

        m_button_preview = Gtk::manage( new SKELETON::ImgToggleToolButton( ICON::PREVIEW ) );
        m_button_preview->signal_clicked().connect( sigc::mem_fun( *this, &MessageToolBarBase::slot_toggle_preview ) );
    }

    return m_button_preview;
}


// プレビュー切り替え
void MessageToolBarBase::slot_toggle_preview()
{
    if( ! m_enable_slot ) return;

#ifdef _DEBUG
    std::cout << "MessageToolBarBase::slot_toggle_preview\n";
#endif

    MESSAGE::get_admin()->set_command( "toggle_preview" );
}


// previewボタンのトグル
void MessageToolBarBase::set_active_previewbutton( const bool active )
{
    m_enable_slot = false;
    m_button_preview->set_active( active );
    m_enable_slot = true;
}


/**
 * @brief ボタンのアイコンを再読み込み
 */
void MessageToolBarBase::reload_ui_icon()
{
    SKELETON::ToolBar::reload_ui_icon();

    set_button_icon( m_button_preview, ICON::PREVIEW );
}



//////////////////////////////////


// 通常のツールバー

MessageToolBar::MessageToolBar()
    : MessageToolBarBase()
{
    MessageToolBar::pack_buttons();
}


MessageToolBar::~MessageToolBar() noexcept = default;


// 新規スレ名entry表示切り替え
void MessageToolBar::show_entry_new_subject( bool show )
{
    if( m_show_entry_new_subject == show ) return;
    m_show_entry_new_subject = show;
    update_button();
}


std::string MessageToolBar::get_new_subject() const
{
    if( m_show_entry_new_subject && m_entry_new_subject ) return m_entry_new_subject->get_text();

    return std::string();
}

void MessageToolBar::clear_new_subject()
{
    if( m_entry_new_subject ) m_entry_new_subject->set_text( "" );
}


/**
 * @brief ボタンのアイコンを再読み込み
 */
void MessageToolBar::reload_ui_icon()
{
    MessageToolBarBase::reload_ui_icon();

    set_button_icon( m_button_undo, ICON::UNDO );
    set_button_icon( m_button_insert_draft, ICON::INSERTTEXT );
}


// ボタンのパッキング
// virtual
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
                if( auto button = get_button_preview() ) {
                    get_buttonbar().append( *button );
                    button->set_label( CONTROL::get_label( CONTROL::Preview ) );
                    set_tooltip( *button, CONTROL::get_label_motions( CONTROL::Preview )
                                          + "\n\nタブ移動のショートカットでも表示の切り替えが可能\n\n"
                                          + CONTROL::get_label_motions( CONTROL::TabRight ) + "\n\n"
                                          + CONTROL::get_label_motions( CONTROL::TabLeft ) );
                }
                break;

            case ITEM_WRITEMSG:
                get_buttonbar().append( *get_button_write() );
                set_tooltip( *get_button_write(), CONTROL::get_label_motions( CONTROL::ExecWrite ) + "\n\nTabキーで書き込みボタンにフォーカスを移すことも可能" );
                break;

            case ITEM_OPENBOARD:
                get_buttonbar().append( *get_button_board() );
                break;

            case ITEM_NAME:
                pack_transparent_separator();

                // スレ名ラベルを表示
                if( ! m_show_entry_new_subject ) get_buttonbar().append( *get_label() );

                // 新規スレ名入力entry表示
                else{

                    if( ! m_tool_new_subject ){

                        m_tool_new_subject = Gtk::manage( new Gtk::ToolItem );
                        m_entry_new_subject = Gtk::manage( new Gtk::Entry );
                        // map(表示)したときと新規スレ名を入力したときに処理するシグナル
                        m_entry_new_subject->signal_map().connect(
                            sigc::mem_fun( MESSAGE::get_admin(), &MessageAdmin::slot_new_subject_changed ) );
                        m_entry_new_subject->signal_changed().connect(
                            sigc::mem_fun( MESSAGE::get_admin(), &MessageAdmin::slot_new_subject_changed ) );

                        m_entry_new_subject->set_size_request( 0 );

                        m_tool_new_subject->add( *m_entry_new_subject );
                        m_tool_new_subject->set_expand( true );
                    }

                    get_buttonbar().append( *m_tool_new_subject );
                }
                pack_transparent_separator();
                break;

            case ITEM_UNDO:

                if( ! m_button_undo ){
                    m_button_undo = Gtk::manage( new SKELETON::ImgToolButton( ICON::UNDO, CONTROL::UndoEdit ) );
                    m_button_undo->signal_clicked().connect( sigc::mem_fun( *this, &MessageToolBar::slot_undo_clicked ) );
                }

                get_buttonbar().append( *m_button_undo );
                set_tooltip( *m_button_undo, CONTROL::get_label_motions( CONTROL::UndoEdit ) );

                break;

            case ITEM_INSERTTEXT:

                if( ! m_button_insert_draft ){
                    m_button_insert_draft = Gtk::manage( new SKELETON::ImgToolButton( ICON::INSERTTEXT,
                                                                                      CONTROL::InsertText ) );
                    m_button_insert_draft->signal_clicked().connect( sigc::mem_fun( *this, &MessageToolBar::slot_insert_draft_clicked ) );
                }

                get_buttonbar().append( *m_button_insert_draft );
                set_tooltip( *m_button_insert_draft, CONTROL::get_label_motions( CONTROL::InsertText ) );

                break;

            case ITEM_LOCK_MESSAGE:
                if( auto button = get_button_lock() ) {
                    get_buttonbar().append( *button );
                    button->set_label( CONTROL::get_label( CONTROL::LockMessage ) );
                    set_tooltip( *button, CONTROL::get_label_motions( CONTROL::LockMessage ) );
                }
                break;

            case ITEM_QUIT:
                get_buttonbar().append( *get_button_close() );
                set_tooltip( *get_button_close(), CONTROL::get_label_motions( CONTROL::CancelWrite ) );
                break;

            case ITEM_SEPARATOR:
                pack_separator();
                break;
        }
        ++num;
    }

    set_relief();
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
    MessageToolBarPreview::pack_buttons();
}


MessageToolBarPreview::~MessageToolBarPreview() noexcept = default;


// ボタンのパッキング
// virtual
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
                if( auto button = get_button_preview() ) {
                    get_buttonbar().append( *button );
                    button->set_label( "プレビューを閉じる" );
                    set_tooltip( *button, "プレビューを閉じる" );
                }
                break;

            case ITEM_OPENBOARD:
                get_buttonbar().append( *get_button_board() );
                break;

            case ITEM_NAME:
                pack_transparent_separator();
                get_buttonbar().append( *get_label() );
                pack_transparent_separator();
                break;

            case ITEM_WRITEMSG:
                get_buttonbar().append( *get_button_write() );
                set_tooltip( *get_button_write(), CONTROL::get_label_motions( CONTROL::ExecWrite ) + "\n\nTabキーで書き込みボタンにフォーカスを移すことも可能" );
                break;

            case ITEM_QUIT:
                get_buttonbar().append( *get_button_close() );
                set_tooltip( *get_button_close(), CONTROL::get_label_motions( CONTROL::CancelWrite ) );
                break;
        }
        ++num;
    }

    set_relief();
    show_all_children();
}    
