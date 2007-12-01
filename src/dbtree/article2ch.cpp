// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "article2ch.h"
#include "nodetree2ch.h"
#include "interface.h"

#include "jdlib/miscutil.h"
#include "jdlib/misctime.h"

#include "login2ch.h"

#include <sstream>

using namespace DBTREE;


enum
{
    DEFAULT_NUMBER_MAX_2CH = 1000  // 規定の最大レス数
};


Article2ch::Article2ch( const std::string& datbase, const std::string& id, bool cached )
    : Article2chCompati( datbase, id, cached )
{
    // 規定の最大レス数セット
    set_number_max( DEFAULT_NUMBER_MAX_2CH );
}



Article2ch::~Article2ch()
{}



// 書き込みメッセージ変換
const std::string Article2ch::create_write_message( const std::string& name, const std::string& mail, const std::string& msg )
{
    if( msg.empty() ) return std::string();

    std::string charset = DBTREE::board_charset( get_url() );

    std::stringstream ss_post;
    ss_post.clear();
    ss_post << "bbs="      << DBTREE::board_id( get_url() )
            << "&key="     << get_key();

    // hana
    std::string hana = DBTREE::board_hana_for_write( get_url() );
    if( ! hana.empty() ) ss_post << "&hana=" << hana;

    // ログイン中
    if( LOGIN::get_login2ch()->login_now() ){
                std::string sid = LOGIN::get_login2ch()->get_sessionid();
                ss_post << "&sid=" << MISC::url_encode( sid.c_str(), sid.length() );
    }

    ss_post << "&time="    << get_time_modified()
            << "&submit="  << MISC::charset_url_encode( "書き込む", charset )
            << "&FROM="    << MISC::charset_url_encode( name, charset )
            << "&mail="    << MISC::charset_url_encode( mail, charset )
            << "&MESSAGE=" << MISC::charset_url_encode( msg, charset );

#ifdef _DEBUG
    std::cout << "Article2chCompati::create_write_message " << ss_post.str() << std::endl;
#endif

    return ss_post.str();
}



NodeTreeBase* Article2ch::create_nodetree()
{
    return new NodeTree2ch( get_url(), get_org_url(), get_date_modified() );
}
