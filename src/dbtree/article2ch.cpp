// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "article2ch.h"
#include "nodetree2ch.h"
#include "interface.h"

#include "jdlib/miscutil.h"
#include "jdlib/misctime.h"

#include "config/globalconf.h"

#include "login2ch.h"
#include "loginp2.h"

#include <sstream>

using namespace DBTREE;


Article2ch::Article2ch( const std::string& datbase, const std::string& id, bool cached )
    : Article2chCompati( datbase, id, cached )
{}


Article2ch::~Article2ch() noexcept
{}



// 書き込みメッセージ変換
const std::string Article2ch::create_write_message( const std::string& name, const std::string& mail, const std::string& msg )
{
    if( msg.empty() ) return std::string();

    const std::string charset = DBTREE::board_charset( get_url() );

    std::stringstream ss_post;
    ss_post.clear();
    ss_post << "bbs="      << DBTREE::board_id( get_url() )
            << "&key="     << get_key();

    // キーワード( hana=mogera や suka=pontan など )
    const std::string keyword = DBTREE::board_keyword_for_write( get_url() );
    if( ! keyword.empty() ) ss_post << "&" << keyword;

    // ログイン中
    if( CORE::get_login2ch()->login_now() ){
                std::string sid = CORE::get_login2ch()->get_sessionid();
                ss_post << "&sid=" << MISC::url_encode( sid.c_str(), sid.length() );
    }

    ss_post << "&time="    << get_time_modified()
            << "&submit="  << MISC::charset_url_encode( "書き込む", charset )
            << "&FROM="    << MISC::charset_url_encode( name, charset )
            << "&mail="    << MISC::charset_url_encode( mail, charset )
            << "&MESSAGE=" << MISC::charset_url_encode( msg, charset );

    if( CORE::get_loginp2()->login_now() ){

        ss_post << "&detect_hint=" << MISC::charset_url_encode( "◎◇", charset )
                << "&host=" << MISC::url_encode( MISC::get_hostname( get_url(), false ) )
                << "&popup=1"
                << "&rescount=" << get_number_load()
                << "&ttitle_en=" << MISC::url_encode( MISC::base64( MISC::Iconv( MISC::remove_space( get_subject() ), "UTF-8", charset ) ) )
                << "&csrfid=" << MISC::url_encode( CORE::get_loginp2()->get_sessiondata() );
    }

#ifdef _DEBUG
    std::cout << "Article2chCompati::create_write_message " << ss_post.str() << std::endl;
#endif

    return ss_post.str();
}


//
// bbscgi のURL
//
// (例) "http://www.hoge2ch.net/test/bbs.cgi"
//
//
const std::string Article2ch::url_bbscgi()
{
    if( CORE::get_loginp2()->login_now() ) return CONFIG::get_url_loginp2() + "post.php";

    return Article2chCompati::url_bbscgi();
}



//
// subbbscgi のURL
//
// (例) "http://www.hoge2ch.net/test/subbbs.cgi"
//
const std::string Article2ch::url_subbbscgi()
{
    if( CORE::get_loginp2()->login_now() ) return CONFIG::get_url_loginp2() + "post.php";

    return Article2chCompati::url_subbbscgi();
}



NodeTreeBase* Article2ch::create_nodetree()
{
    return new NodeTree2ch( get_url(), get_org_url(), get_date_modified(), get_since_time() );
}



//
// dat落ちしたスレをロードするか
//
const bool Article2ch::is_load_olddat()
{
    // 2chにログインしている場合
    // または、offlaw2を使う設定の場合 ( bbspinkを除く )
    return CORE::get_login2ch()->login_now()
            || ( CONFIG::get_use_offlaw2_2ch() && get_url().find( ".bbspink.com" ) == std::string::npos );
}
