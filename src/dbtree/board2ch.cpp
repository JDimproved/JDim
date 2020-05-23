// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "board2ch.h"
#include "article2ch.h"
#include "articlehash.h"

#include "config/globalconf.h"

#include "jdlib/miscutil.h"
#include "jdlib/misctime.h"

#include "login2ch.h"
#include "loginbe.h"

#include <sstream>

using namespace DBTREE;


Board2ch::Board2ch( const std::string& root, const std::string& path_board, const std::string& name )
    : Board2chCompati( root, path_board, name, std::string() )
{
#ifdef _DEBUG
    std::cout << "Board2ch::Board2ch\n";
#endif
}


Board2ch::~Board2ch() noexcept = default;


// ユーザエージェント
// ダウンロード用
const std::string& Board2ch::get_agent()
{
    return CONFIG::get_agent_for2ch();
}

// 書き込み用
const std::string& Board2ch::get_agent_w()
{
    return CONFIG::get_agent_for2ch();
}


// 読み込み用プロキシ
std::string Board2ch::get_proxy_host()
{
    const int mode = get_mode_local_proxy();

    if( mode == DBTREE::PROXY_GLOBAL ){

        if( CONFIG::get_use_proxy_for2ch() ) return CONFIG::get_proxy_for2ch();
    }
    else if( mode == DBTREE::PROXY_LOCAL ) return get_local_proxy();

    return std::string();
}

int Board2ch::get_proxy_port()
{
    const int mode = get_mode_local_proxy();

    if( mode == DBTREE::PROXY_GLOBAL ) return CONFIG::get_proxy_port_for2ch();
    else if( mode == DBTREE::PROXY_LOCAL ) return get_local_proxy_port();

    return 0;
}

std::string Board2ch::get_proxy_basicauth()
{
    const int mode = get_mode_local_proxy();

    if( mode == DBTREE::PROXY_GLOBAL ) return CONFIG::get_proxy_basicauth_for2ch();
    else if( mode == DBTREE::PROXY_LOCAL ) return get_local_proxy_basicauth();

    return std::string();
}


// 書き込み用プロキシ
std::string Board2ch::get_proxy_host_w()
{
    const int mode = get_mode_local_proxy_w();

    if( mode == DBTREE::PROXY_GLOBAL ){
        if( CONFIG::get_use_proxy_for2ch_w() ) return CONFIG::get_proxy_for2ch_w();
    }
    else if( mode == DBTREE::PROXY_LOCAL ) return get_local_proxy_w();

    return std::string();
}

int Board2ch::get_proxy_port_w()
{
    const int mode = get_mode_local_proxy_w();

    if( mode == DBTREE::PROXY_GLOBAL ) return CONFIG::get_proxy_port_for2ch_w();
    else if( mode == DBTREE::PROXY_LOCAL ) return get_local_proxy_port_w();

    return 0;
}


std::string Board2ch::get_proxy_basicauth_w()
{
    const int mode = get_mode_local_proxy_w();

    if( mode == DBTREE::PROXY_GLOBAL ) return CONFIG::get_proxy_basicauth_for2ch_w();
    else if( mode == DBTREE::PROXY_LOCAL ) return get_local_proxy_basicauth_w();

    return std::string();
}


// 読み書き用クッキー作成
std::string Board2ch::cookie_for_request() const
{
#ifdef _DEBUG
    std::cout << "Board2ch::cookie_for_request\n";
#endif

    std::string cookie = Board2chCompati::cookie_for_request();
    if( cookie.empty() ) cookie = get_hap();

    // BE ログイン中
    if( CORE::get_loginbe()->login_now() ){
        if( ! cookie.empty() ) cookie += "; ";
        cookie += "DMDM=" + CORE::get_loginbe()->get_sessionid() + "; MDMD=" + CORE::get_loginbe()->get_sessiondata();
    }

#ifdef _DEBUG
    std::cout << "cookie = " << cookie << std::endl;
#endif 

    return cookie;
}


std::string Board2ch::get_write_referer()
{
    return Board2chCompati::get_write_referer();
}


// 新スレ作成時の書き込みメッセージ作成
std::string Board2ch::create_newarticle_message( const std::string& subject, const std::string& name,
                                                 const std::string& mail, const std::string& msg )
{
    if( subject.empty() ) return std::string();
    if( msg.empty() ) return std::string();

    std::stringstream ss_post;
    ss_post.clear();
    ss_post << "bbs="      << get_id()
            << "&subject=" << MISC::charset_url_encode( subject, get_charset() );

    // キーワード( hana=mogera や suka=pontan など )
    const std::string keyword = get_keyword_for_write();
    if( ! keyword.empty() ) ss_post << "&" << keyword;

    // 2chログイン中
    // sidを送る
    if( CORE::get_login2ch()->login_now() ){
        std::string sid = CORE::get_login2ch()->get_sessionid();
        ss_post << "&sid=" << MISC::url_encode( sid.c_str(), sid.length() );
    }

    ss_post << "&time="    << get_time_modified()
            << "&submit="  << MISC::charset_url_encode( "新規スレッド作成", get_charset() )
            << "&FROM="    << MISC::charset_url_encode( name, get_charset() )
            << "&mail="    << MISC::charset_url_encode( mail, get_charset() )
            << "&MESSAGE=" << MISC::charset_url_encode( msg, get_charset() );

#ifdef _DEBUG
    std::cout << "Board2ch::create_newarticle_message " << ss_post.str() << std::endl;
#endif

    return ss_post.str();
}


//
// 新スレ作成時のbbscgi のURL
//
// (例) "http://www.hoge2ch.net/test/bbs.cgi"
//
//
std::string Board2ch::url_bbscgi_new()
{
    return Board2chCompati::url_bbscgi_new();
}


//
// 新スレ作成時のsubbbscgi のURL
//
// (例) "http://www.hoge2ch.net/test/subbbs.cgi"
//
std::string Board2ch::url_subbbscgi_new()
{
    return Board2chCompati::url_subbbscgi_new();
}



//
// 新しくArticleBaseクラスを追加してそのポインタを返す
//
// cached : HDD にキャッシュがあるならtrue
//
ArticleBase* Board2ch::append_article( const std::string& datbase, const std::string& id, const bool cached )
{
    if( empty() ) return get_article_null();

    ArticleBase* article = new DBTREE::Article2ch( datbase, id, cached );
    if( article ){
        get_hash_article()->push( article );

        // 最大レス数セット
        article->set_number_max( get_number_max_res() );
    }
    else return get_article_null();

    return article;
}



// 2chのクッキー
std::string Board2ch::get_hap() const
{
    if( ! CONFIG::get_use_cookie_hap() ) return std::string();

    if( get_root().find( ".bbspink.com" ) != std::string::npos ) return CONFIG::get_cookie_hap_bbspink();
    return CONFIG::get_cookie_hap();
}

void Board2ch::set_hap( const std::string& hap )
{
    if( ! CONFIG::get_use_cookie_hap() )  return;

    if( get_root().find( ".bbspink.com" ) != std::string::npos ) CONFIG::set_cookie_hap_bbspink( hap );
    else CONFIG::set_cookie_hap( hap );
}


//
// 2chのクッキーの更新
//
void Board2ch::update_hap()
{
    if( ! CONFIG::get_use_cookie_hap() ) return;

    const std::string new_cookie = Board2chCompati::cookie_for_request();

    if( ! new_cookie.empty() ) {
        const std::string old_cookie = get_hap();
        set_hap( new_cookie );
#ifdef _DEBUG
        std::cout << "Board2ch::update_hap old = " << old_cookie << std::endl;
        std::cout << "Board2ch::update_hap new = " << new_cookie << std::endl;
#endif
    }
}
