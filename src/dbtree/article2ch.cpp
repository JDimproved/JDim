// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "article2ch.h"
#include "nodetree2ch.h"
#include "interface.h"

#include "jdlib/misccharcode.h"
#include "jdlib/misctime.h"
#include "jdlib/miscutil.h"

#include "config/globalconf.h"

#include "login2ch.h"

#include <sstream>
#include <ctime>

using namespace DBTREE;


Article2ch::Article2ch( const std::string& datbase, const std::string& id, bool cached, const Encoding enc )
    : Article2chCompati( datbase, id, cached, enc )
{}


Article2ch::~Article2ch() noexcept = default;


/** @brief 書き込みメッセージ作成
 *
 * @param[in] name      名前、トリップ
 * @param[in] mail      メールアドレス、sage
 * @param[in] msg       書き込むメッセージ
 * @param[in] utf8_post trueならUTF-8のままURLエンコードする
 * @return URLエンコードしたフォームデータ (application/x-www-form-urlencoded)
 */
std::string Article2ch::create_write_message( const std::string& name, const std::string& mail,
                                              const std::string& msg, const bool utf8_post )
{
    if( msg.empty() ) return std::string();

    const Encoding enc{ utf8_post ? Encoding::utf8 : get_encoding() };

    std::stringstream ss_post;
    ss_post << "FROM=" << MISC::url_encode_plus( name, enc )
            << "&mail=" << MISC::url_encode_plus( mail, enc )
            << "&MESSAGE=" << MISC::url_encode_plus( msg, enc )
            << "&bbs=" << DBTREE::board_id( get_url() )
            << "&key=" << get_key()
            << "&time=" << time(0)
            << "&submit=" << MISC::url_encode_plus( "書き込む", enc )
            // XXX: ブラウザの種類に関係なく含めて問題ないか？
            << "&oekaki_thread1=";

    // キーワード( hana=mogera や suka=pontan など )
    const std::string& keyword = DBTREE::board_keyword_for_write( get_url() );
    if( ! keyword.empty() ) ss_post << "&" << keyword;

    // ログイン中
    if( CORE::get_login2ch()->login_now() ){
        const std::string sid = CORE::get_login2ch()->get_sessionid();
        ss_post << "&sid=" << MISC::url_encode_plus( sid );
    }

#ifdef _DEBUG
    std::cout << "Article2ch::create_write_message " << ss_post.str() << std::endl;
#endif

    // 書き込みメッセージを作成したらキーワードはリセットする
    DBTREE::board_set_keyword_for_write( get_url(), std::string{} );

    return ss_post.str();
}


NodeTreeBase* Article2ch::create_nodetree()
{
    return new NodeTree2ch( get_url(), get_org_url(), get_date_modified(), get_since_time() );
}



//
// dat落ちしたスレをロードするか
//
bool Article2ch::is_load_olddat() const
{
    // 2chにログインしている場合
    return CORE::get_login2ch()->login_now();
}
