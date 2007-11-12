// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "messageadmin.h"
#include "messageview.h"
#include "post.h"

#include "skeleton/msgdiag.h"
#include "skeleton/label_entry.h"

#include "jdlib/miscutil.h"

#include "dbtree/interface.h"

#include "command.h"
#include "session.h"

#include <sstream>

using namespace MESSAGE;

 MessageViewMain::MessageViewMain( const std::string& url, const std::string& msg )
    : MessageViewBase( url )
{
    setup_view();
    set_message( msg );

    // タイトルセット
    set_title( "[ 書き込み ] " + DBTREE::article_subject( get_url() ) );
}


MessageViewMain::~MessageViewMain()
{
    save_name();
}


//
// ポストするメッセージ作成
//
std::string MessageViewMain::create_message()
{
    std::string msg = get_text_message().get_text();
    std::string name = get_entry_name().get_text();
    std::string mail = get_entry_mail().get_text();

    if( msg.empty() ){
        SKELETON::MsgDiag mdiag( MESSAGE::get_admin()->get_win(), "本文が空白です" ); mdiag.run();
        return std::string();
    }

    if( SESSION::get_article_current_url().find( get_url() ) == std::string::npos ){
        SKELETON::MsgDiag mdiag( MESSAGE::get_admin()->get_win(),
                                 "スレビューで開いているスレと異なるスレに書き込もうとしています\n\n誤爆する可能性がありますが書き込みますか？",
                                 false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_NONE );

        mdiag.set_title( "！！！誤爆注意！！！" );
        mdiag.add_button( Gtk::Stock::NO, Gtk::RESPONSE_NO );
        mdiag.add_button( Gtk::Stock::YES, Gtk::RESPONSE_YES );
        mdiag.add_button( "スレを開く", Gtk::RESPONSE_YES + 100 );

        int ret = mdiag.run();
        if( ret == Gtk::RESPONSE_NO ) return std::string();
        else if( ret == Gtk::RESPONSE_YES + 100 ){
            CORE::core_set_command( "open_article", get_url(), "true", "" );
            return std::string();
        }
    }

    return DBTREE::create_write_message( get_url(), name, mail, msg );
}


//
// 書き込み
//
// 書き込みが終わったら MessageViewBase::post_fin()が呼ばれる
//
void MessageViewMain::write()
{
    std::string msg = create_message();
    if( msg.empty() ) return;
    post_msg( msg, false );
}



void MessageViewMain::reload()
{
    CORE::core_set_command( "open_article", get_url(), "true", "" );
}


///////////////////////////


 MessageViewNew::MessageViewNew( const std::string& url, const std::string& msg )
    : MessageViewBase( url )
{
    setup_view();

    set_message( msg );

    if( get_entry_subject() ){
        get_entry_subject()->set_editable( true );
        get_entry_subject()->set_text( std::string() );
        get_entry_subject()->grab_focus();
    }

    // タイトルセット
    set_title( "[ 新スレ作成 ] " + DBTREE::article_subject( get_url() ) );
}




//
// ポストするメッセージ作成
//
std::string MessageViewNew::create_message()
{
    std::string subject;
    if( get_entry_subject() ) subject = get_entry_subject()->get_text();
    std::string msg = get_text_message().get_text();
    std::string name = get_entry_name().get_text();
    std::string mail = get_entry_mail().get_text();

    if( subject.empty() ){
        SKELETON::MsgDiag mdiag( MESSAGE::get_admin()->get_win(), "スレタイトルが空白です" ); mdiag.run();
        return std::string();
    }

    if( msg.empty() ){
        SKELETON::MsgDiag mdiag( MESSAGE::get_admin()->get_win(), "本文が空白です" ); mdiag.run();
        return std::string();
    }
    
    SKELETON::MsgDiag mdiag( MESSAGE::get_admin()->get_win(),
                             "新スレを作成しますか？", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
    if( mdiag.run() == Gtk::RESPONSE_YES ) return DBTREE::create_newarticle_message( get_url(), subject, name, mail, msg );

    return std::string();
}


//
// 書き込み
//
// 書き込みが終わったら MessageViewBase::post_fin()が呼ばれる
//
void MessageViewNew::write()
{
    std::string msg = create_message();
    if( msg.empty() ) return;
    post_msg( msg, true );
}



void MessageViewNew::reload()
{
    CORE::core_set_command( "open_board", DBTREE::url_subject( get_url() ), "true" );
}
