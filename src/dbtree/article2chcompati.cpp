// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "article2chcompati.h"
#include "nodetree2chcompati.h"
#include "interface.h"

#include "jdlib/miscutil.h"
#include "jdlib/misctime.h"

#include "config/globalconf.h"

#include <sstream>

using namespace DBTREE;

Article2chCompati::Article2chCompati( const std::string& datbase, const std::string& _id, bool cached )
    : ArticleBase( datbase, _id, cached )
{
    assert( ! get_id().empty() );

    // key (idから拡張子を除いたもの)を取得
    size_t i = get_id().rfind( "." ); // 拡張子は取り除く
    if( i != std::string::npos ) set_key( get_id().substr( 0, i ) );

    // key から since 計算
    const char* ckey = get_key().c_str();
    if( i != std::string::npos ) set_since_time( atol( ckey ) );

    // スレッド924か
    if( ckey[ 0 ] == '9' && ckey[ 1 ] == '2' && ckey[ 2 ] == '4' ) set_is_924( true );
}



Article2chCompati::~Article2chCompati() noexcept
{}



// 書き込みメッセージ変換
std::string Article2chCompati::create_write_message( const std::string& name, const std::string& mail, const std::string& msg )
{
    if( msg.empty() ) return std::string();

    std::string charset = DBTREE::board_charset( get_url() );

    std::stringstream ss_post;
    ss_post.clear();
    ss_post << "bbs="      << DBTREE::board_id( get_url() )
            << "&key="     << get_key()
            << "&time="    << get_time_modified()
            << "&submit="  << MISC::charset_url_encode( "書き込む", charset )
            << "&FROM="    << MISC::charset_url_encode( name, charset )
            << "&mail="    << MISC::charset_url_encode( mail, charset )
            << "&MESSAGE=" << MISC::charset_url_encode( msg, charset );

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
std::string Article2chCompati::url_bbscgi()
{
    std::string cgibase = DBTREE::url_bbscgibase( get_url() );
    return cgibase.substr( 0, cgibase.length() -1 ); // 最後の '/' を除く
}



//
// subbbscgi のURL
//
// (例) "http://www.hoge2ch.net/test/subbbs.cgi"
//
std::string Article2chCompati::url_subbbscgi()
{
    std::string cgibase = DBTREE::url_subbbscgibase( get_url() );
    return cgibase.substr( 0, cgibase.length() -1 ); // 最後の '/' を除く
}




NodeTreeBase* Article2chCompati::create_nodetree()
{
    return new NodeTree2chCompati( get_url(), get_date_modified() );
}
