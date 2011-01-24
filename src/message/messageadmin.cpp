// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "messageadmin.h"
#include "messagewin.h"
#include "toolbar.h"

#include "skeleton/view.h"
#include "skeleton/msgdiag.h"
#include "skeleton/dragnote.h"
#include "skeleton/editview.h"

#include "dbtree/interface.h"

#include "config/globalconf.h"

#include "viewfactory.h"
#include "command.h"
#include "global.h"
#include "session.h"

MESSAGE::MessageAdmin* instance_messageadmin = NULL;

MESSAGE::MessageAdmin* MESSAGE::get_admin()
{
    if( ! instance_messageadmin ) instance_messageadmin = new MESSAGE::MessageAdmin( URL_MESSAGEADMIN );
    return instance_messageadmin;

}

void MESSAGE::delete_admin()
{
    if( instance_messageadmin ) delete instance_messageadmin;
    instance_messageadmin = NULL;
}

using namespace MESSAGE;


MessageAdmin::MessageAdmin( const std::string& url )
    : SKELETON::Admin( url ), m_toolbar( NULL ), m_toolbar_preview( NULL ), m_text_message( NULL )
{
    get_notebook()->set_show_tabs( false );
}


MessageAdmin::~MessageAdmin()
{
#ifdef _DEBUG
    std::cout << "MessageAdmin::~MessageAdmin\n";
#endif 

    if( m_toolbar ) delete m_toolbar;
    if( m_toolbar_preview ) delete m_toolbar_preview;
    if( m_text_message ) delete m_text_message;
}



void MessageAdmin::show_entry_new_subject( bool show )
{
    if( m_toolbar ) m_toolbar->show_entry_new_subject( show );
}


std::string MessageAdmin::get_new_subject()
{
    if( m_toolbar ) return m_toolbar->get_new_subject();

    return std::string();
}


SKELETON::EditView* MessageAdmin::get_text_message()
{
    if( ! m_text_message ) m_text_message = new SKELETON::EditView();

    return m_text_message;
}


//
// ローカルコマンド実行
//
// virtual
void MessageAdmin::command_local( const COMMAND_ARGS& command )
{
#ifdef _DEBUG
    std::cout << "MessageAdmin::command_local command = " << command.command << std::endl;
#endif

    SKELETON::View *view = get_current_view();

    // 書き込みボタンにフォーカスを移す
    if( command.command == "focus_button_write" ){
        if( get_notebook()->get_current_toolbar() == TOOLBAR_MESSAGE ) m_toolbar->focus_button_write();
        else m_toolbar_preview->focus_button_write();
    }
    // プレビュー切り替え
    else if( command.command == "toggle_preview" ){
        if( view ) view->set_command( command.command );
    }
    // undo
    else if( command.command == "undo_text" ){
        if( view ) view->set_command( command.command );
    }
    // 下書き挿入
    else if( command.command == "insert_draft" ){
        if( view ) view->set_command( command.command );
    }
    // 通常のツールバーに表示切り替え
    else if( command.command == "switch_toolbar_message" ){
        if( view ){
            get_notebook()->set_current_toolbar( TOOLBAR_MESSAGE, view );
            m_toolbar->set_active_previewbutton( false );
        }
    }
    // プレビューツールバーに表示切り替え
    else if( command.command == "switch_toolbar_preview" ){
        if( view ){
            get_notebook()->set_current_toolbar( TOOLBAR_PREVIEW, view );
            m_toolbar_preview->set_active_previewbutton( true );
        }
    }
    // 指定したスレに対応する書き込みビューを開いていて
    // かつ書き込みビューが空なら閉じる
    else if( command.command == "close_message" ){

        SKELETON::View *view = get_current_view();
        if( view && view->set_command( "empty" ) && view->get_url().find( command.url ) != std::string::npos ){
            close_current_view();
        }
    }

    // ビューの wrap 切り替え
    else if( "toggle_wrap" ){

        if( view ) view->set_command( command.command );
    }
}


//
// 閉じる
//
// virtual
void MessageAdmin::close_view( const std::string& url )
{
#ifdef _DEBUG
    std::cout << "MessageAdmin::close_view url = " << url << std::endl;
#endif

    SKELETON::View *view = get_current_view();
    if( ! view ) return;

    if( view->is_loading() ){
        SKELETON::MsgDiag mdiag( get_win(), "書き込み中です" );
        mdiag.run();
        return;
    }

    if( ! view->set_command( "empty" ) ){
        if( ! delete_message( view ) ) return;
    }

    if( m_toolbar ) m_toolbar->clear_new_subject();

    if( view->is_locked() ) view->set_command( "clear_message" );
    else{
        Admin::close_view( url );
        if( empty() ) close_window();
    }
}


//
// ウィンドウ開く
//
// virtual
void MessageAdmin::open_window()
{
    SKELETON::JDWindow* win = get_jdwin();

    if( ! SESSION::get_embedded_mes() && ! win && ! empty() ){

#ifdef _DEBUG
    std::cout << "MessageAdmin::open_window\n";
#endif

        win = new MESSAGE::MessageWin();
        set_jdwin( win );
        win->pack_remove_end( false, *get_widget() );
        win->show_all();
    }
    else if( win && win->is_hide() ){
        win->show();
        win->focus_in();
    }
}


//
// ウィンドウ閉じる
//
// virtual
void MessageAdmin::close_window()
{
    if( get_jdwin() ){

#ifdef _DEBUG
    std::cout << "MessageAdmin::close_window\n";
#endif
        get_jdwin()->pack_remove_end( true, *get_widget() );
        delete_jdwin();
    }
}


// virtual
void MessageAdmin::switch_admin()
{
    if( ! has_focus() ) CORE::core_set_command( "switch_message" );
}


// virtual
void MessageAdmin::tab_left( const bool updated )
{
    SKELETON::View *view = get_current_view();
    if( view ) view->set_command( "tab_left" );
}


// virtual
void MessageAdmin::tab_right( const bool updated )
{
    SKELETON::View *view = get_current_view();
    if( view ) view->set_command( "tab_right" );
}



//
// 開く
//
// command.arg2 == "new" なら新スレ
//
// virtual
void MessageAdmin::open_view( const COMMAND_ARGS& command )
{
    const std::string url = command.url;
    const std::string msg = command.arg1;
    const bool new_thread = ( command.arg2 == "new" );

#ifdef _DEBUG
    std::cout << "MessageAdmin::open_view " << url << std::endl;
#endif

    SKELETON::View *current_view = get_current_view();
    if( current_view ){

        if( current_view->is_loading() ){
            SKELETON::MsgDiag mdiag( get_win(), "書き込み中です" );
            mdiag.run();
            return;
        }

        // 既存のビューにメッセージを追加してフォーカスを移す
        if( url == current_view->get_url() )
        {
            if( ! msg.empty() ) current_view->set_command( "add_message", msg );

            switch_admin();
            return;
        }

        // URLが異なればビューを破棄
        if( ! current_view->set_command( "empty" ) ){
            if( ! delete_message( current_view ) ) return;
        }

        // 古いビューを破棄
        int page = get_notebook()->get_current_page();
        get_notebook()->remove_page( page, true );
        delete current_view;

        if( m_toolbar ) m_toolbar->clear_new_subject();
    }

    std::string url_msg;
    int type;
    CORE::VIEWFACTORY_ARGS args;
    if( ! new_thread ){
        type = CORE::VIEW_MESSAGE;
        args.arg1 = msg;
        url_msg = url;
    }

    // 新スレ
    // スレッドの id は ID_OF_NEWTHREAD.(各板別の拡張子) とする。
    else{
        type = CORE::VIEW_NEWTHREAD;
        args.arg1 = msg;
        url_msg = DBTREE::url_datbase( url ) + ID_OF_NEWTHREAD + DBTREE::board_ext( url );
    }

    // ツールバー表示
    show_toolbar();

    SKELETON::View *view = CORE::ViewFactory( type, url_msg, args );
    get_notebook()->append_page( url_msg, *view );

    // ウィンドウ表示
    open_window();

    get_notebook()->show_all();
    switch_admin();
    view->show();
    view->show_view();

    get_notebook()->set_current_toolbar( view->get_id_toolbar(), view );

    set_current_page( get_notebook()->page_num( *view ) );
    focus_current_view();
}



//
// ステータス表示
//
// virtual
void MessageAdmin::set_status( const std::string& url, const std::string& stat, const bool force )
{
    // 埋め込みビューで、実況中の場合はステータス表示しない
    if( SESSION::get_embedded_mes() && SESSION::is_live( url ) ) return;

    SKELETON::Admin::set_status( url, stat, force );
}


//
// ツールバー表示
//
// virtual
void MessageAdmin::show_toolbar()
{
    if( ! m_toolbar ){

        // 通常のツールバー
        m_toolbar = new MessageToolBar();
        get_notebook()->append_toolbar( *m_toolbar );
        m_toolbar->open_buttonbar();

        // プレビュー用のツールバー
        m_toolbar_preview = new MessageToolBarPreview();
        get_notebook()->append_toolbar( *m_toolbar_preview );
        m_toolbar_preview->open_buttonbar();
    }

    get_notebook()->show_toolbar();
}



//
// メッセージを破棄するか尋ねる
//
// 破棄する場合はtrueが戻る
//
const bool MessageAdmin::delete_message( SKELETON::View * view )
{
    if( ! CONFIG::get_show_savemsgdiag() ) return true;

    SKELETON::MsgCheckDiag mdiag( get_win(),
                                  "編集中のメッセージを閉じる前に内容を保存しますか？\n\n保存ボタンを押すとメッセージを保存できます。",

                                  "今後表示しない(常に保存せずに閉じる) (_D)",
                                  Gtk::MESSAGE_WARNING, Gtk::BUTTONS_NONE );

    mdiag.add_button( "保存せずに閉じる(_Q)", Gtk::RESPONSE_NO );
    mdiag.add_button( Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL );
    Gtk::Button button( Gtk::Stock::SAVE );
    mdiag.add_default_button( &button, Gtk::RESPONSE_YES );

    int ret = mdiag.run();
    mdiag.hide();
    bool result = false;

    switch( ret )
    {
        case Gtk::RESPONSE_NO:
            result = true;
            if( mdiag.get_chkbutton().get_active() ) CONFIG::set_show_savemsgdiag( false );
            break;

        case Gtk::RESPONSE_YES:
            if( ! view->set_command( "save_message" ) ) return delete_message( view );
            result = true;
            break;
    }

    return result;
}
