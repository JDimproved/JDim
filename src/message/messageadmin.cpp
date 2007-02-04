// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "messageadmin.h"
#include "messagewin.h"

#include "skeleton/view.h"

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


void MessageAdmin::clock_in()
{
    if( m_view ) m_view->clock_in();
}


//
// コマンドセット
//
void MessageAdmin::set_command( const std::string& command, const std::string& url, const std::string& arg1 )
{
#ifdef _DEBUG
    std::cout << "MessageAdmin::set_command : " << command << " "
              << url << " " << arg1 << " " << std::endl;
#endif

    COMMAND_ARGS command_arg;
    command_arg.command = command;
    command_arg.url = url;
    command_arg.arg1 = arg1;
    m_list_command.push_back( command_arg );
    m_disp.emit();
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
            Gtk::MessageDialog mdiag( "書き込み中です" );
            mdiag.run();
            return;
        }

        if( m_view && ! m_view->set_command( "empty" ) ){
            Gtk::MessageDialog mdiag( "編集中のメッセージを破棄しますか？", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL );
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
    if( m_win ){
        m_win->remove();
        delete m_win;
    }

    if( m_view ){
        m_vbox.remove( *m_view );
        delete m_view;

        CORE::core_set_command( "empty_page", m_url );
    }

    m_view = NULL;
    m_win = NULL;
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
        Gtk::MessageDialog mdiag( "書き込み中です" );
        mdiag.run();
        return;
    }

    if( m_view && ! m_view->set_command( "empty" ) ){
        Gtk::MessageDialog mdiag( "編集中のメッセージを破棄しますか？", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL );
        if( mdiag.run() == Gtk::RESPONSE_OK );
        else return;
    }

    close_view();

    std::string title;
    std::string url_msg;
    int type;
    CORE::VIEWFACTORY_ARGS args;
    if( ! new_thread ){
        type = CORE::VIEW_MESSAGE;
        args.arg1 = msg;
        url_msg = url;
        title = "JD - [ 書き込み ] " + DBTREE::article_subject( url );
    }

    // 新スレ
    // スレッドの id は 0000000000.(各板別の拡張子) とする。
    else{
        type = CORE::VIEW_NEWTHREAD;
        args.arg1 = msg;
        url_msg = DBTREE::url_datbase( url ) + "0000000000" + DBTREE::board_ext( url );
        title = "JD - [ 新スレ作成 ] " + DBTREE::board_name( url );
    }

    m_view = CORE::ViewFactory( type, url_msg, args );

    m_vbox.pack_start( *m_view );
    m_vbox.show_all();

    // ウィンドウ表示
    if( ! SESSION::get_embedded_mes() ){

        m_win = new MESSAGE::MessageWin();
        m_win->set_title( title );
        m_win->add( m_vbox );
        m_win->show_all();
    }

    switch_admin();
}
