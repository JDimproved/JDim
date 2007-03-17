// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "messageadmin.h"
#include "messagewin.h"

#include "skeleton/view.h"
#include "skeleton/msgdiag.h"

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
    : SKELETON::Admin( url ),
      m_win( NULL ),
      m_view( NULL )
{}


MessageAdmin::~MessageAdmin()
{
#ifdef _DEBUG
    std::cout << "MessageAdmin::~MessageAdmin\n";
#endif 

    if( m_view ) close_view( m_view->get_url() );
}


Gtk::Window* MessageAdmin::get_win()
{
    return dynamic_cast< Gtk::Window* >( m_win );
}


//
// ローカルコマンド実行
//
void MessageAdmin::command_local( const COMMAND_ARGS& command )
{
#ifdef _DEBUG
    std::cout << "MessageAdmin::command_local command = " << command.command << std::endl;
#endif

    // 書き込み実行
    if( command.command == "exec_Write" ){
        if( m_view ) m_view->set_command( command.command );
    }
    // 書き込みボタンにフォーカスを移す
    else if( command.command == "focus_write" ){
        if( m_view ) m_view->set_command( command.command );
    }
}


//
// 閉じる
//
void MessageAdmin::close_view( const std::string& url )
{
    if( m_view && m_view->get_url() == url ){

        close_window();

        m_eventbox.remove();
        delete m_view;
        m_view = NULL;

        CORE::core_set_command( "empty_page", get_url() );
    }
}


//
// 現在のビューを閉じる
//
void MessageAdmin::close_current_view()
{
    if( m_view && m_view->set_command( "loading" ) ){
        SKELETON::MsgDiag mdiag( m_win, "書き込み中です" );
        mdiag.run();
        return;
    }

    if( m_view && ! m_view->set_command( "empty" ) ){
        SKELETON::MsgDiag mdiag( m_win, "編集中のメッセージを破棄しますか？", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL );
        if( mdiag.run() == Gtk::RESPONSE_OK );
        else return;
    }

    if( m_view ) close_view( m_view->get_url() );
}


//
// タイトル表示
//
void MessageAdmin::set_title( const std::string& url, const std::string& title )
{
    if( m_win ) m_win->set_title( "JD - " + title );
    else Admin::set_title( url, title );
}


//
// URLバーにアドレス表示
//
void MessageAdmin::set_url( const std::string& url, const std::string& url_show )
{
    if( m_win ){}
    else Admin::set_url( url, url_show );
}


//
// ステータス表示
//
void MessageAdmin::set_status( const std::string& url, const std::string& stat )
{
    if( m_win ) m_win->set_status( stat );
    else Admin::set_status( url, stat );
}


//
// ウィンドウ開く
//
void MessageAdmin::open_window()
{
    if( ! SESSION::get_embedded_mes() && ! m_win && ! empty() ){
        m_win = new MESSAGE::MessageWin();
        m_win->pack_remove( false, m_eventbox );
        m_win->show_all();
        focus_current_view();
    }
    else if( m_win ){
        m_win->show();
        m_win->focus_in();
    }
}


//
// ウィンドウ閉じる
//
void MessageAdmin::close_window()
{
    if( m_win ){
        m_win->pack_remove( true, m_eventbox );
        delete m_win;
        m_win = NULL;
    }
}


//
// フォーカス
//
void MessageAdmin::focus_view( int )
{
#ifdef _DEBUG
    std::cout << "MessageAdmin::focus_view\n";
#endif

    if( m_win ) m_win->focus_in();
    if( m_view ){
        m_view->focus_view();
        set_title( m_view->get_url(), m_view->get_title() );
        set_url( m_view->get_url(), m_view->url_for_copy() );
        set_status( m_view->get_url(), m_view->get_status() );
    }
}


void MessageAdmin::focus_current_view()
{
    focus_view( 0 );
}


void MessageAdmin::switch_admin()
{
    CORE::core_set_command( "switch_message" );
}


void MessageAdmin::tab_left()
{
    if( m_view ) m_view->set_command( "tab_left" );
}


void MessageAdmin::tab_right()
{
    if( m_view ) m_view->set_command( "tab_right" );
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

    if( m_view && m_view->set_command( "loading" ) ){
        SKELETON::MsgDiag mdiag( m_win, "書き込み中です" );
        mdiag.run();
        return;
    }

    // 既存のビューにメッセージを追加してフォーカスを移す
    if( m_view && url == m_view->get_url() )
    {
        if( ! msg.empty() ) m_view->set_command( "add_message", msg );

        switch_admin();
        return;
    }

    // URLが異なればビューを破棄
    if( m_view && ! m_view->set_command( "empty" ) ){
        SKELETON::MsgDiag mdiag( m_win, "編集中のメッセージを破棄しますか？", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL );
        if( mdiag.run() == Gtk::RESPONSE_OK );
        else return;
    }

    // 古いビューを破棄
    if( m_view ){
        m_eventbox.remove();
        delete m_view;
        m_view = NULL;
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

    m_view = CORE::ViewFactory( type, url_msg, args );

    m_eventbox.add( *m_view );
    m_eventbox.show_all();

    // ウィンドウ表示
    open_window();

    switch_admin();
}


//
// 再レイアウト実行
//
void MessageAdmin::relayout_all()
{
    if( m_view ) m_view->relayout();    
}



SKELETON::View* MessageAdmin::get_view( const std::string& url, bool )
{
    return get_current_view();
}


//
// カレントview 取得
//
SKELETON::View* MessageAdmin::get_current_view()
{
    return dynamic_cast< SKELETON::View* >( m_view );
}
