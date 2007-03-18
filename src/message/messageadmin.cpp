// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "messageadmin.h"
#include "messagewin.h"

#include "skeleton/view.h"
#include "skeleton/msgdiag.h"
#include "skeleton/dragnote.h"

#include "dbtree/interface.h"

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
    : SKELETON::Admin( url )
{
    get_notebook()->set_show_tabs( false );
}


MessageAdmin::~MessageAdmin()
{
#ifdef _DEBUG
    std::cout << "MessageAdmin::~MessageAdmin\n";
#endif 
}


//
// ローカルコマンド実行
//
void MessageAdmin::command_local( const COMMAND_ARGS& command )
{
#ifdef _DEBUG
    std::cout << "MessageAdmin::command_local command = " << command.command << std::endl;
#endif

    SKELETON::View *view = get_current_view();

    // 書き込み実行
    if( command.command == "exec_Write" ){
        if( view ) view->set_command( command.command );
    }
    // 書き込みボタンにフォーカスを移す
    else if( command.command == "focus_write" ){
        if( view ) view->set_command( command.command );
    }
}


//
// 閉じる
//
void MessageAdmin::close_view( const std::string& url )
{
    SKELETON::View *view = get_current_view();
    if( ! view ) return;

    if( view->set_command( "loading" ) ){
        SKELETON::MsgDiag mdiag( get_win(), "書き込み中です" );
        mdiag.run();
        return;
    }

    if( ! view->set_command( "empty" ) ){
        SKELETON::MsgDiag mdiag( get_win(), "編集中のメッセージを破棄しますか？", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL );
        if( mdiag.run() == Gtk::RESPONSE_OK );
        else return;
    }

    Admin::close_view( url );
    if( empty() ) close_window();
}


//
// ウィンドウ開く
//
void MessageAdmin::open_window()
{
    if( ! SESSION::get_embedded_mes() && ! get_jdwin() && ! empty() ){
        MESSAGE::MessageWin *win = new MESSAGE::MessageWin();
        set_jdwin( win );
        win->pack_remove_end( false, *get_widget() );
        win->show_all();
    }
    else if( get_jdwin() ){
        get_jdwin()->show();
        get_jdwin()->focus_in();
    }
}


//
// ウィンドウ閉じる
//
void MessageAdmin::close_window()
{
    if( get_jdwin() ){
        get_jdwin()->pack_remove_end( true, *get_widget() );
        delete_jdwin();
    }
}


void MessageAdmin::switch_admin()
{
    if( ! has_focus() ) CORE::core_set_command( "switch_message" );
}


void MessageAdmin::tab_left()
{
    SKELETON::View *view = get_current_view();
    if( view ) view->set_command( "tab_left" );
}


void MessageAdmin::tab_right()
{
    SKELETON::View *view = get_current_view();
    if( view ) view->set_command( "tab_right" );
}



//
// 開く
//
// command.arg2 == "new" なら新スレ
//
void MessageAdmin::open_view( const COMMAND_ARGS& command )
{
    std::string url = command.url;
    std::string msg = command.arg1;
    bool new_thread = ( command.arg2 == "new" );

#ifdef _DEBUG
    std::cout << "MessageAdmin::open_view " << url << std::endl;
#endif

    SKELETON::View *current_view = get_current_view();
    if( current_view ){

        if( current_view->set_command( "loading" ) ){
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
            SKELETON::MsgDiag mdiag( get_win(), "編集中のメッセージを破棄しますか？", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL );
            if( mdiag.run() == Gtk::RESPONSE_OK );
            else return;
        }

        // 古いビューを破棄
        int page = get_notebook()->get_current_page();
        get_notebook()->remove_page( page );
        delete current_view;
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
    // スレッドの id は 0000000000.(各板別の拡張子) とする。
    else{
        type = CORE::VIEW_NEWTHREAD;
        args.arg1 = msg;
        url_msg = DBTREE::url_datbase( url ) + "0000000000" + DBTREE::board_ext( url );
    }

    SKELETON::View *view = CORE::ViewFactory( type, url_msg, args );

    get_notebook()->append_page( url_msg, *view );

    // ウィンドウ表示
    open_window();

    get_notebook()->show_all();
    switch_admin();
    view->show();
    view->show_view();
    set_current_page( get_notebook()->page_num( *view ) );
    focus_current_view();
}
