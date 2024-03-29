// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "messageadmin.h"
#include "messageview.h"
#include "post.h"

#include "skeleton/msgdiag.h"
#include "skeleton/label_entry.h"
#include "skeleton/editview.h"

#include "jdlib/miscutil.h"

#include "dbtree/interface.h"

#include "command.h"
#include "session.h"

#include <glib/gi18n.h>

#include <sstream>

using namespace MESSAGE;

MessageViewMain::MessageViewMain( const std::string& url, const std::string& msg )
    : MessageViewBase( url )
{
    setup_view();
    set_message( msg );

    // ツールバーのスレタイトルを編集不可にする
    MESSAGE::get_admin()->show_entry_new_subject( false );

    const std::string& subject = DBTREE::article_modified_subject( get_url() );

    // メインウィンドウのタイトルに表示する文字
    set_title( "[ 書き込み ] " + MISC::to_plain( subject ) );

    // ツールバーにスレ名を表示
    set_label( MISC::to_markup( subject ), true );
}


MessageViewMain::~MessageViewMain()
{
    save_name();
}


/** @brief ポストするメッセージ作成
 *
 * @param[in] utf8_post trueならUTF-8のままURLエンコードする
 * @return URLエンコードしたフォームデータ (application/x-www-form-urlencoded)
 */
std::string MessageViewMain::create_message( const bool utf8_post )
{
    if( ! get_text_message() ) return std::string();

    const Glib::ustring msg = get_text_message()->get_text();
    const std::string name = get_entry_name().get_text();
    const std::string mail = get_entry_mail().get_text();

    if( msg.empty() ){
        SKELETON::MsgDiag mdiag( get_parent_win(), "書き込みがキャンセルされました。" );
        mdiag.property_message_type() = Gtk::MESSAGE_WARNING;
        mdiag.set_secondary_text( "本文が空欄です。" );
        mdiag.run();
        return std::string();
    }
/*    else
    {
        // 終端スペース/改行チェック
        const size_t end_pos = msg.find_last_not_of( "　 \n" );
        const size_t msg_length = msg.length();

        // 最後がスペース/改行である
        if( end_pos != msg_length - 1 )
        {
            SKELETON::MsgDiag mdiag( get_parent_win(),
                                     "メッセージがスペースまたは改行で終わっています。\n\n"
                                     "このまま書き込みますか？ (または、改行等を削除)",
                                     false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_NONE );

            mdiag.set_title( "確認" );
            mdiag.add_button( g_dgettext( GTK_DOMAIN, "_Cancel" ), Gtk::RESPONSE_CANCEL );
            mdiag.add_button( g_dgettext( GTK_DOMAIN, "_Remove" ), Gtk::RESPONSE_DELETE_EVENT );
            mdiag.add_button( g_dgettext( GTK_DOMAIN, "_Yes" ), Gtk::RESPONSE_YES );

            switch( mdiag.run() )
            {
                // スペース/改行を取り除いた物に置き換える(書き込まない)
                case Gtk::RESPONSE_DELETE_EVENT:

                    get_text_message().set_text( msg.substr( 0, end_pos + 1 ) );
                    return std::string();

                // キャンセル
                case Gtk::RESPONSE_CANCEL:

                    return std::string();
            }
        }
    }
*/
    // 誤爆を警告
    if( SESSION::get_article_current_url().find( get_url() ) == std::string::npos ){

        SKELETON::MsgDiag mdiag( get_parent_win(),
                                 "スレビューで開いているスレと異なるスレに書き込もうとしています\n\n誤爆する可能性がありますが書き込みますか？",
                                 false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_NONE );

        mdiag.set_title( "！！！誤爆注意！！！" );
        mdiag.add_button( g_dgettext( GTK_DOMAIN, "_No" ), Gtk::RESPONSE_NO );
        mdiag.add_button( g_dgettext( GTK_DOMAIN, "_Yes" ), Gtk::RESPONSE_YES );
        mdiag.add_button( "スレを開く", Gtk::RESPONSE_YES + 100 );

        int ret = mdiag.run();
        if( ret != Gtk::RESPONSE_YES ){

            if( ret == Gtk::RESPONSE_YES + 100 ) CORE::core_set_command( "open_article", get_url(), "true", "" );
            return std::string();
        }
    }

    return DBTREE::create_write_message( get_url(), name, mail, msg, utf8_post );
}


//
// 書き込み
//
// 書き込みが終わったら MessageViewBase::post_fin()が呼ばれる
//
void MessageViewMain::write_impl( const std::string& msg )
{
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
    m_max_subject = DBTREE::subject_count( get_url() );

    setup_view();

    set_message( msg );

    // MessageAdmin のシグナルに接続して MessageToolBar のスレタイトル入力欄に橋渡ししてもらう
    MESSAGE::get_admin()->sig_new_subject_changed().connect(
        sigc::mem_fun( *this, &MessageViewBase::slot_new_subject_changed ) );

    // ツールバーのスレタイトルを編集可能にする
    MESSAGE::get_admin()->show_entry_new_subject( true );

    // メインウィンドウのタイトルに表示する文字
    set_title( "[ 新スレ作成 ] " + MISC::to_plain( DBTREE::article_modified_subject( get_url() ) ) );

    // 板のフロントページをダウンロードしてスレ立てに使うキーワードを更新する
    DBTREE::board_download_front( get_url() );
}


/** @brief ポストするメッセージ作成
 *
 * @param[in] utf8_post trueならUTF-8のままURLエンコードする
 * @return URLエンコードしたフォームデータ (application/x-www-form-urlencoded)
 */
std::string MessageViewNew::create_message( const bool utf8_post )
{
    if( ! get_text_message() ) return std::string();

    std::string subject = MESSAGE::get_admin()->get_new_subject();
    std::string msg = get_text_message()->get_text();
    std::string name = get_entry_name().get_text();
    std::string mail = get_entry_mail().get_text();

    const char* reason = nullptr;
    if( subject.empty() ) reason = "スレタイトルが空欄です。";
    else if( m_max_subject < get_lng_encoded_subject() ) reason = "スレタイトルの文字数が多すぎます。";
    else if( msg.empty() ) reason = "本文が空欄です。";

    if( reason ) {
        SKELETON::MsgDiag mdiag( get_parent_win(), "スレ立てがキャンセルされました。" );
        mdiag.property_message_type() = Gtk::MESSAGE_WARNING;
        mdiag.set_secondary_text( reason );
        mdiag.run();
        return std::string{};
    }

    SKELETON::MsgDiag mdiag( get_parent_win(),
                             "新スレを作成しますか？", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
    if( mdiag.run() == Gtk::RESPONSE_YES ) {
        return DBTREE::create_newarticle_message( get_url(), subject, name, mail, msg, utf8_post );
    }

    return std::string();
}


//
// 書き込み
//
// 書き込みが終わったら MessageViewBase::post_fin()が呼ばれる
//
void MessageViewNew::write_impl( const std::string& msg )
{
    post_msg( msg, true );
}



void MessageViewNew::reload()
{
    CORE::core_set_command( "open_board", DBTREE::url_boardbase( get_url() ), "true" );
}
