// SPDX-License-Identifier: GPL-2.0-only
/** @file bbsmenu.cpp
 *
 * @remark DBTREE::Root クラスを元にして実装した関数があるため
 * このファイルのライセンスは GPL-2.0-only になる。
 */

//#define _DEBUG
#include "jddebug.h"

#include "bbsmenu.h"

#include "config/globalconf.h"
#include "jdlib/cookiemanager.h"
#include "jdlib/jdiconv.h"
#include "jdlib/loaderdata.h"
#include "jdlib/miscmsg.h"
#include "jdlib/miscutil.h"
#include "skeleton/msgdiag.h"

#include "cache.h"
#include "command.h"
#include "httpcode.h"


namespace DBTREE::bm {
constexpr const std::size_t kSizeOfRawdata = 2 * 1024 * 1024; ///< bbsmenu.html の最大サイズ
constexpr const char* kRootNodeName = "boardlist"; ///< ルート要素名( boards.xml )
}


using namespace DBTREE;


std::string BBSMenu::path_bbsmenu_boards_xml() const
{
    constexpr const bool protocol = false;
    std::string path = CACHE::path_bbsmenu_root();
    path.append( MISC::get_hostname( m_url, protocol ) );
    path.append( "-boards.xml" );
    return path;
}


/** @brief ローカルキャッシュからBBSMENUのXML読み込み
 *
 * @see Root::load_cache()
 */
void BBSMenu::load_cache()
{
    const std::string path = path_bbsmenu_boards_xml();

#ifdef _DEBUG
    std::cout << "BBSMenu::load_cache xml = " << path << std::endl;
#endif

    std::string xml_bbsmenu;
    if( CACHE::load_rawdata( path, xml_bbsmenu ) ) {

        // Domノードを初期化
        m_xml_document.init( xml_bbsmenu );

        // Domノードの内容からDBに板を登録
        analyze_board_xml();
    }
}


/** @brief サーバから bbsmenu.html を読み込んで XML に変換開始
 *
 * @details 読み終わったらreceive_finish()でXMLに変換して"update_bbslist"コマンド発行
 * @see Root::download_bbsmenu()
 */
void BBSMenu::download_bbsmenu()
{
    if( is_loading() ) return;

    clear();
    m_rawdata.reserve( bm::kSizeOfRawdata );

    JDLIB::LOADERDATA data;
    data.init_for_data();
    data.url = m_url;
    data.modified = get_date_modified();

    bool send_cookie = true;

    constexpr bool protocol = false;
    const std::string host = MISC::get_hostname( data.url, protocol );
    if( MISC::ends_with( host, ".5ch.net ") || MISC::ends_with( host, ".2ch.net" ) ) {
        data.agent = CONFIG::get_agent_for2ch();
        if( CONFIG::get_use_proxy_for2ch() ) {
            data.host_proxy = CONFIG::get_proxy_for2ch();
            data.port_proxy = CONFIG::get_proxy_port_for2ch();
            data.basicauth_proxy = CONFIG::get_proxy_basicauth_for2ch();
            data.basicauth = CONFIG::get_proxy_basicauth_for2ch();
            send_cookie = CONFIG::get_send_cookie_to_proxy_for2ch();
        }
    }
    else {
        data.agent = CONFIG::get_agent_for_data();
        if( CONFIG::get_use_proxy_for_data() ) {
            data.host_proxy = CONFIG::get_proxy_for_data();
            data.port_proxy = CONFIG::get_proxy_port_for_data();
            data.basicauth_proxy = CONFIG::get_proxy_basicauth_for_data();
            data.basicauth = CONFIG::get_proxy_basicauth_for_data();
            send_cookie = CONFIG::get_send_cookie_to_proxy_for_data();
        }
    }

    if( send_cookie ) {
        const JDLIB::CookieManager* cookie_manager = JDLIB::get_cookie_manager();
        data.cookie_for_request = cookie_manager->get_cookie_by_host( data.url );
    }

    start_load( data );
}


/** @brief BBSMENUを受信した後の処理
 *
 * @details 受信に失敗したときはローカルキャッシュから読み込む
 * @see Root::receive_finish()
 */
void BBSMenu::receive_finish()
{
#ifdef _DEBUG
    std::cout << "BBSMenu::receive_finish code = " << get_code() << std::endl;
#endif

    if( get_code() == HTTP_NOT_MODIFIED ) {

        auto msg = Glib::ustring::compose(
            "%1\n\nサーバー上のBBSMENUは更新されていません。強制的に再読み込みをしますか？", get_str_code() );
        SKELETON::MsgDiag mdiag( nullptr, msg, false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
        mdiag.set_default_response( Gtk::RESPONSE_YES );
        if( mdiag.run() == Gtk::RESPONSE_YES ){
            set_date_modified( std::string() );
            download_bbsmenu();
        }
        else {
            // 再読み込しない場合はローカルキャッシュからXMLを読み込む
            load_cache();
        }
        return;
    }

    if( ( get_code() == HTTP_MOVED_PERM || get_code() == HTTP_REDIRECT || get_code() == HTTP_PERMANENT_REDIRECT )
            && ! location().empty() ) {

        auto msg = Glib::ustring::compose( "%1\n\nBBSMENUが %2 に移転しました。更新しますか？",
                                           get_str_code(), location() );
        SKELETON::MsgDiag mdiag( nullptr, msg, false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
        mdiag.set_default_response( Gtk::RESPONSE_YES );
        if( mdiag.run() == Gtk::RESPONSE_YES ){
            set_date_modified( std::string() );
            m_url = location();
            download_bbsmenu();
        }
        else {
            // 更新しない場合はローカルキャッシュからXMLを読み込む
            load_cache();
        }
        return;
    }

    if( get_code() != HTTP_OK ) {

        auto msg = Glib::ustring::compose( "%1\n\nBBSMENUの読み込みに失敗したためBBSMENUは更新されませんでした。\n\n"
                                           "プロキシ設定やBBSMENUを取得するサーバのアドレスを確認して、"
                                           "再読み込みをして下さい。", get_str_code() );
        SKELETON::MsgDiag mdiag( nullptr, msg, false, Gtk::MESSAGE_ERROR );
        mdiag.run();
        MISC::ERRMSG( "bbsmenu load failed : " + get_str_code() );

        // 通信に失敗したときはローカルキャッシュからXMLを読み込む
        load_cache();

        CORE::core_set_command( "update_bbslist" );
        return;
    }

    // 文字コードを変換してXML作成
    JDLIB::Iconv libiconv{ Encoding::utf8, Encoding::sjis };
    const std::string& rawdata_utf8 = libiconv.convert( m_rawdata.data(), m_rawdata.size() );

    bbsmenu2xml( rawdata_utf8 );

    if( m_xml_document.hasChildNodes() ) {

        // データベース更新
        analyze_board_xml();

        // bbslistview更新
        CORE::core_set_command( "update_bbslist" );
    }

    clear();
}


/** @brief HTMLの解析を行いカテゴリと板を持つDOMを構築しXMLファイルに保存する
 *
 * @param[in] menu BBSMENUのHTMLデータ
 * @see Root::bbsmenu2xml()
 */
void BBSMenu::bbsmenu2xml( const std::string& menu )
{
    if( menu.empty() ) return;

    // menu のノードツリーを取得( menu がHTMLなので第二引数は true )
    const XML::Document html( menu, true );

    // XML用のノードツリーにルートノードを追加
    m_xml_document.clear();
    XML::Dom* root = m_xml_document.appendChild( XML::NODE_TYPE_ELEMENT, bm::kRootNodeName );

    // カテゴリの要素 <subdir></subdir>
    XML::Dom* subdir = nullptr;

    // カテゴリの有効/無効
    bool enabled = true;

    // 現在の仕様では HTML > BODY > font[size="2"] の子要素が対象
    // 特定のサイト(2ch.sc、next2ch.net)のbbsmenu.htmlにはfontタグがないため別のタグを使う
    std::list<XML::Dom*> targets = html.getElementsByTagName( "font" );
    if( targets.empty() ) targets = html.getElementsByTagName( "small" );
    if( targets.empty() ) targets = html.getElementsByTagName( "body" );
    if( targets.empty() ) {
        MISC::ERRMSG( "parse error for bbsmenu" );
        return;
    }
    for( const XML::Dom* child : *targets.front() ) {

        // 要素b( カテゴリ名 )
        if( child->nodeName() == "b" ) {

            const std::string category = MISC::chref_decode( child->firstChild()->nodeValue() );

            // 追加しないカテゴリ
            if( category == "チャット"
                || category == "ツール類"
                || category == "他のサイト" ) {

                enabled = false;
                continue;
            }
            else enabled = true;

            // <subdir>
            subdir = root->appendChild( XML::NODE_TYPE_ELEMENT, "subdir" );
            subdir->setAttribute( "name", category );
        }
        // 要素bに続く要素a( 板URL )
        else if( subdir && enabled && child->nodeName() == "a" ) {

            const std::string board_name = MISC::chref_decode( child->firstChild()->nodeValue() );
            std::string url = child->getAttribute( "href" );

            // Schema-less URL (net_path) ならBBSMENUのURLから http[s]: をコピーして追加する
            if( url.compare( 0, 2, "//" ) == 0 ) {
                const auto scheme = std::string_view{ m_url }.substr( 0, m_url.find( ':' ) + 1 );
                url.insert( 0, scheme );
            }

            // 外部BBSMENUにある <a> 要素はすべて板として扱う
            std::string element_name = "board";

            XML::Dom* board = subdir->appendChild( XML::NODE_TYPE_ELEMENT, element_name );
            board->setAttribute( "name", board_name );
            board->setAttribute( "url", url );
        }
    }

    root->setAttribute( "date_modified", get_date_modified() );

#ifdef _DEBUG
    std::cout << "modified = " << get_date_modified() << std::endl;
#endif

    // キャッシュディレクトリの bbsmenu/ は Root::save_bbsmenu() で作成するため呼び出す順序に注意すること
    const std::string path = path_bbsmenu_boards_xml();
    CACHE::save_rawdata( path, m_xml_document.get_xml() );

#ifdef _DEBUG
    std::cout << "BBSMenu::bbsmenu2xml : save " << path << std::endl;
#endif
}
