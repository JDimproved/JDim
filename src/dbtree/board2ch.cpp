// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "board2ch.h"
#include "article2ch.h"

#include "config/globalconf.h"

#include "jdlib/miscutil.h"
#include "jdlib/misctime.h"
#include "jdlib/jdregex.h"

#include "login2ch.h"
#include "loginbe.h"

#include <sstream>

using namespace DBTREE;


enum
{
    DEFAULT_NUMBER_MAX_2CH = 1000  // 規定の最大レス数
};


Board2ch::Board2ch( const std::string& root, const std::string& path_board, const std::string& name )
    : Board2chCompati( root, path_board, name, std::string() )
{
#ifdef _DEBUG
    std::cout << "Board2ch::Board2ch\n";
#endif

    set_number_max_res( DEFAULT_NUMBER_MAX_2CH );
}


Board2ch::~Board2ch()
{}


const std::string& Board2ch::get_agent()
{
    return CONFIG::get_agent_for2ch();
}


//
// ホスト別プロクシ
//
const std::string Board2ch::get_proxy_host()
{
    int mode = get_mode_local_proxy();

    if( mode == DBTREE::PROXY_GLOBAL ){

        if( CONFIG::get_use_proxy_for2ch() ) return CONFIG::get_proxy_for2ch();
    }
    else if( mode == DBTREE::PROXY_LOCAL ) return get_local_proxy();

    return std::string();
}

const int Board2ch::get_proxy_port()
{
    int mode = get_mode_local_proxy();

    if( mode == DBTREE::PROXY_GLOBAL ) return CONFIG::get_proxy_port_for2ch();
    else if( mode == DBTREE::PROXY_LOCAL ) return get_local_proxy_port();

    return 0;
}

const std::string Board2ch::get_proxy_host_w()
{
    int mode = get_mode_local_proxy_w();

    if( mode == DBTREE::PROXY_GLOBAL ){
        if( CONFIG::get_use_proxy_for2ch_w() ) return CONFIG::get_proxy_for2ch_w();
    }
    else if( mode == DBTREE::PROXY_LOCAL ) return get_local_proxy_w();

    return std::string();
}

const int Board2ch::get_proxy_port_w()
{
    int mode = get_mode_local_proxy_w();

    if( mode == DBTREE::PROXY_GLOBAL ) return CONFIG::get_proxy_port_for2ch_w();
    else if( mode == DBTREE::PROXY_LOCAL ) return get_local_proxy_port_w();

    return 0;
}


//書き込み用クッキー作成
const std::string Board2ch::cookie_for_write()
{
#ifdef _DEBUG
    std::cout << "Board2ch::cookie_for_write\n";
#endif

    std::string cookie = Board2chCompati::cookie_for_write();

    // BE ログイン中
    if( CORE::get_loginbe()->login_now() ){
        if( ! cookie.empty() ) cookie += "; ";
        cookie += "DMDM=" + CORE::get_loginbe()->get_username() + "; MDMD=" + CORE::get_loginbe()->get_passwd();
    }

#ifdef _DEBUG
    std::cout << "cookie = " << cookie << std::endl;
#endif 

    return cookie;
}


// 新スレ作成時の書き込みメッセージ作成
const std::string Board2ch::create_newarticle_message( const std::string& subject,
                                                       const std::string& name, const std::string& mail, const std::string& msg )
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
// 新しくArticleBaseクラスを追加してそのポインタを返す
//
// cached : HDD にキャッシュがあるならtrue
//
ArticleBase* Board2ch::append_article( const std::string& id, bool cached )
{
    if( empty() ) return get_article_null();

    ArticleBase* article = new DBTREE::Article2ch( url_datbase(), id, cached );
    if( article ){
        get_list_article().push_back( article );

        // 最大レス数セット
        article->set_number_max( get_number_max_res() );
    }
    else return get_article_null();

    return article;
}


