// ライセンス: 最新のGPL

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

MESSAGE::MessageAdmin* instance_messageadmin = NULL;

MESSAGE::MessageAdmin* MESSAGE::get_admin()
{
    if( ! instance_messageadmin ) instance_messageadmin = new MESSAGE::MessageAdmin();
    return instance_messageadmin;

}

void MESSAGE::delete_admin()
{
    if( instance_messageadmin ) delete instance_messageadmin;
    instance_messageadmin = NULL;
}

using namespace MESSAGE;


MessageAdmin::MessageAdmin()
    : m_win( 0 ),
      m_view( 0 )
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

    else if( command.command  == "focus_view" ) focus_view();

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
// 閉じる
//
void MessageAdmin::close_view()
{
    if( m_win ) delete m_win;
    if( m_view ) delete m_view;

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
    m_win = new MESSAGE::MessageWin();
    m_win->add( *m_view );
    m_win->show_all();
}
