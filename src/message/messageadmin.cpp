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
#include "controlid.h"
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
    : m_url( url ),
      m_win( NULL ),
      m_view( NULL )
{
    m_disp.connect( sigc::mem_fun( *this, &MessageAdmin::exec_command ) );
}


MessageAdmin::~MessageAdmin()
{
#ifdef _DEBUG
    std::cout << "MessageAdmin::~MessageAdmin\n";
#endif 
   
    close_view();
}


Gtk::Window* MessageAdmin::get_win()
{
    return dynamic_cast< Gtk::Window* >( m_win );
}


void MessageAdmin::clock_in()
{
    if( m_view ) m_view->clock_in();
}


//
// コマンドセット(通常)
//
void MessageAdmin::set_command( const std::string& command, const std::string& url, const std::string& arg1 )
{
    set_command_impl( false, command, url, arg1 );
}

//
// コマンドセット(即実行)
//
void MessageAdmin::set_command_immediately( const std::string& command, const std::string& url, const std::string& arg1 )
{
    set_command_impl( true, command, url, arg1 );
}


//
// コマンドセット
//
void MessageAdmin::set_command_impl( bool immediately, const std::string& command, const std::string& url, const std::string& arg1 )
{
#ifdef _DEBUG
    std::cout << "MessageAdmin::set_command : immediately = " << immediately
              << " command = " << command << " url = " << url << " " << arg1 << " " << std::endl;
#endif

    COMMAND_ARGS command_arg;
    command_arg.command = command;
    command_arg.url = url;
    command_arg.arg1 = arg1;

    if( immediately ){

        m_list_command.push_front( command_arg );
        exec_command();

    }else{

        m_list_command.push_back( command_arg );
        m_disp.emit();
    }
}


//
// コマンド実行
//
void MessageAdmin::exec_command()
{
    if( m_list_command.size() == 0 ) return;
    
    COMMAND_ARGS command = m_list_command.front();
    m_list_command.pop_front();

#ifdef _DEBUG
    std::cout << "MessageAdmin::exec_command command = " << command.command << std::endl;
#endif

    if( command.command == "open_view" ) open_view( command.url, command.arg1, false );

    else if( command.command == "create_new_thread" ) open_view( command.url, command.arg1, true );

    else if( command.command  == "close_currentview" ){

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

        close_view();
    }

    else if( command.command  == "focus_current_view" ) focus_view();

    else if( command.command == "relayout_all" ){
        if( m_view ) m_view->relayout();
    }
    else if( command.command == "redraw" ){
        redraw_view( command.url );
    }
    else if( command.command == "redraw_current_view" ){
        if( m_view ) m_view->redraw_view();
    }

    // view の操作
    else if( command.command == "exec_Write" ){
        if( m_view ) m_view->operate_view( CONTROL::ExecWrite );
    }
    else if( command.command == "tab_left" ){
        if( m_view ) m_view->operate_view( CONTROL::TabLeft );
    }
    else if( command.command == "tab_right" ){
        if( m_view ) m_view->operate_view( CONTROL::TabRight );
    }

    // window 開け閉じ
    else if( command.command == "open_window" ){
        open_window();
        return;
    }
    else if( command.command == "close_window" ){
        close_window();
        return;
    }

    // ステータス表示
    else if( command.command == "set_status" ){
        set_status( command.url, command.arg1 );
    }
}


//
// ビューを再描画
//
void MessageAdmin::redraw_view( const std::string& url )
{
#ifdef _DEBUG
        std::cout << "MessageAdmin::redraw_view url = " << url << std::endl;
#endif

    if( m_view && m_view->get_url() == url ){
        m_view->redraw_view();
    }
}



//
// 閉じる
//
void MessageAdmin::close_view()
{
    close_window();

    if( m_view ){
        m_eventbox.remove();
        delete m_view;
        m_view = NULL;

        CORE::core_set_command( "empty_page", m_url );
    }
}



//
// ステータス表示
//
void MessageAdmin::set_status( const std::string& url, const std::string& stat )
{
    if( m_win ) m_win->set_status( stat );
    else CORE::core_set_command( "set_status", url, stat );
}


//
// ウィンドウ開く
//
void MessageAdmin::open_window()
{
    if( ! m_win && ! empty() ){
        m_win = new MESSAGE::MessageWin();
        m_win->set_title( m_title );
        m_win->pack_remove( false, m_eventbox );
        m_win->show_all();
        focus_view();
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
void MessageAdmin::focus_view()
{
    if( m_view ) m_view->focus_view();
}


void MessageAdmin::switch_admin()
{
    CORE::core_set_command( "switch_message" );
}


//
// 開く
//
// new_thread = true なら新スレを立てる
//
void MessageAdmin::open_view( const std::string& url, const std::string& msg, bool new_thread )
{
#ifdef _DEBUG
    std::cout << "MessageAdmin::open_view " << url << std::endl;
#endif

    if( m_view && m_view->set_command( "loading" ) ){
        SKELETON::MsgDiag mdiag( m_win, "書き込み中です" );
        mdiag.run();
        return;
    }

    // 既存のビューにメッセージを追加
    if( m_view && url == m_view->get_url() )
    {
        m_view->set_command( "add_message", msg );

        switch_admin();
        set_command( "focus_current_view" );
        return;
    }

    // URLが異なればビューを破棄
    if( m_view && ! m_view->set_command( "empty" ) ){
        SKELETON::MsgDiag mdiag( m_win, "編集中のメッセージを破棄しますか？", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL );
        if( mdiag.run() == Gtk::RESPONSE_OK );
        else return;
    }

    close_view();

    std::string url_msg;
    int type;
    CORE::VIEWFACTORY_ARGS args;
    if( ! new_thread ){
        type = CORE::VIEW_MESSAGE;
        args.arg1 = msg;
        url_msg = url;
        m_title = "JD - [ 書き込み ] " + DBTREE::article_subject( url );
    }

    // 新スレ
    // スレッドの id は 0000000000.(各板別の拡張子) とする。
    else{
        type = CORE::VIEW_NEWTHREAD;
        args.arg1 = msg;
        url_msg = DBTREE::url_datbase( url ) + "0000000000" + DBTREE::board_ext( url );
        m_title = "JD - [ 新スレ作成 ] " + DBTREE::board_name( url );
    }

    m_view = CORE::ViewFactory( type, url_msg, args );

    m_eventbox.add( *m_view );
    m_eventbox.show_all();

    // ウィンドウ表示
    if( ! SESSION::get_embedded_mes() ) open_window();

    switch_admin();
}
