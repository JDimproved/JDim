// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "articlemachi.h"
#include "nodetreemachi.h"
#include "interface.h"

#include "jdlib/miscutil.h"
#include "jdlib/misctime.h"

#include "config/globalconf.h"

#include <sstream>

using namespace DBTREE;


ArticleMachi::ArticleMachi( const std::string& datbase, const std::string& _id, bool cached, const Encoding enc )
    : ArticleBase( datbase, _id, cached, enc )
{
    assert( !get_id().empty() );

    // Machi の場合は拡張子が無いので key = id
    set_key( get_id() );

    // key から since 計算
    set_since_time( atol( get_key().c_str() ) );
}


ArticleMachi::~ArticleMachi() noexcept = default;


std::string ArticleMachi::create_write_message( const std::string& name, const std::string& mail,
                                                const std::string& msg, const bool utf8_post )
{
    if( msg.empty() ) return std::string();

    const Encoding enc{ utf8_post ? Encoding::utf8 : get_encoding() };

    std::stringstream ss_post;
    ss_post.clear();
    ss_post << "BBS="      << DBTREE::board_id( get_url() )
            << "&KEY="     << get_key()
            << "&TIME="    << get_time_modified()
            << "&submit="  << MISC::url_encode_plus( "書き込む", enc )
            << "&NAME="    << MISC::url_encode_plus( name, enc )
            << "&MAIL="    << MISC::url_encode_plus( mail, enc )
            << "&MESSAGE=" << MISC::url_encode_plus( msg, enc );

#ifdef _DEBUG
    std::cout << "ArticleMachi::create_write_message " << ss_post.str() << std::endl;
#endif

    return ss_post.str();
}




//
// bbscgi のURL
//
// (例) "http://www.machi.to/bbs/write.cgi"
//
//
std::string ArticleMachi::url_bbscgi() const
{
    std::string cgibase = DBTREE::url_bbscgibase( get_url() );
    if( ! cgibase.empty() ) cgibase.pop_back(); // 最後の '/' を除く
    return cgibase;
}


//
// subbbscgi のURL
//
// (例) "http://www.machi.to/bbs/write.cgi"
//
std::string ArticleMachi::url_subbbscgi() const
{
    std::string cgibase = DBTREE::url_subbbscgibase( get_url() );
    if( ! cgibase.empty() ) cgibase.pop_back(); // 最後の '/' を除く
    return cgibase;
}



// offlawモードなら更新チェック可能
bool ArticleMachi::enable_check_update() const
{
    return CONFIG::get_use_machi_offlaw();
}


NodeTreeBase* ArticleMachi::create_nodetree()
{
#ifdef _DEBUG
    std::cout << "ArticleMachi::create_nodetree " << get_url() << std::endl;
#endif

    return new NodeTreeMachi( get_url(), get_date_modified() );
}
