// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "board2ch.h"
#include "article2ch.h"

#include "config/globalconf.h"

#include "jdlib/miscutil.h"
#include "jdlib/misctime.h"

#include "login2ch.h"

#include <sstream>

using namespace DBTREE;


Board2ch::Board2ch( const std::string& root, const std::string& path_board, const std::string& name )
    : Board2chCompati( root, path_board, name, std::string() )
{
#ifdef _DEBUG
    std::cout << "Board2ch::Board2ch\n";
#endif
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
    if( ! CONFIG::get_use_proxy_for2ch() ) return std::string();
    return CONFIG::get_proxy_for2ch();
}

const int Board2ch::get_proxy_port()
{
    return CONFIG::get_proxy_port_for2ch();
}


const std::string Board2ch::get_proxy_host_w()
{
    if( ! CONFIG::get_use_proxy_for2ch_w() ) return std::string();
    return CONFIG::get_proxy_for2ch_w();
}

const int Board2ch::get_proxy_port_w()
{
    return CONFIG::get_proxy_port_for2ch_w();
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

    // 2chのhana値
    std::string hana = hana_for_write();
    if( ! hana.empty() ) ss_post << "&hana=" << hana;

    // ログイン中
    if( LOGIN::get_login2ch()->login_now() ){
        std::string sid = LOGIN::get_login2ch()->get_sessionid();
        ss_post << "&sid=" << MISC::url_encode( sid.c_str(), sid.length() );
    }

    ss_post << "&time="    << time_modified()
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
    if( article ) get_list_article().push_back( article );
    else return get_article_null();

    return article;
}


