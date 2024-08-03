// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "nodetree2chcompati.h"
#include "interface.h"

#include "jdlib/jdiconv.h"
#include "jdlib/loaderdata.h"
#include "jdlib/miscmsg.h"

#include "config/globalconf.h"

#include "httpcode.h"
#include "session.h"

#include <cstring>


using namespace DBTREE;


/// @brief 読み込みの状態
enum class NodeTree2chCompati::Mode
{
    normal, ///< 普通の読み込み
    kako,   ///< URLリダイレクトで過去ログのURLを読み込み
};


NodeTree2chCompati::NodeTree2chCompati( const std::string& url, const std::string& date_modified )
    : NodeTreeBase( url, date_modified )
    , m_mode{ Mode::normal }
{
#ifdef _DEBUG
    std::cout << "NodeTree2chCompati::NodeTree2chCompati url = " << url << " modified = " << date_modified << std::endl;
#endif
}




NodeTree2chCompati::~NodeTree2chCompati()
{
#ifdef _DEBUG    
    std::cout << "NodeTree2chCompati::~NodeTree2chCompati : " << get_url() << std::endl;
#endif

    NodeTree2chCompati::clear();
}



//
// バッファなどのクリア
//
void NodeTree2chCompati::clear()
{
#ifdef _DEBUG    
    std::cout << "NodeTree2chCompati::clear : " << get_url() << std::endl;
#endif

    NodeTreeBase::clear();

    // iconv 削除
    m_iconv.reset();
    m_mode = Mode::normal;
}



/** @brief ロード実行前に呼ぶ初期化関数
 *
 * @details URLリダイレクトの処理を続行するため m_mode はリセットしない
 */
void NodeTree2chCompati::init_loading()
{
#ifdef _DEBUG    
    std::cout << "NodeTree2chCompati::init_loading : " << get_url() << std::endl;
#endif

    NodeTreeBase::init_loading();

    // iconv 初期化
    if( ! m_iconv ) m_iconv = std::make_unique<JDLIB::Iconv>( Encoding::utf8, DBTREE::article_encoding( get_url() ) );
}



//
// raw データを dat 形式に変換
//
// 2ch型サーバの場合は文字コードを変換するだけ
//
const char* NodeTree2chCompati::raw2dat( char* rawlines, int& byte )
{
    assert( m_iconv != nullptr );

    // バッファ自体はiconvクラスの中で持っているのでポインタだけもらう
    const std::string& result = m_iconv->convert( rawlines, std::strlen( rawlines ) );
    byte = result.size();
    return result.c_str();
}



//
// ロード用データ作成
//
void NodeTree2chCompati::create_loaderdata( JDLIB::LOADERDATA& data )
{
    data.agent = DBTREE::get_agent( get_url() );
    data.byte_readfrom = 0;

    // サーバーから過去ログのURLを通知された
    if( m_mode == Mode::kako ) {

        // レジューム設定
        // DATを読み込んでいた場合はレジュームを有りにして、DATの未取得部分を追加するように処理する
        set_resume( get_lng_dat() > 0 );

        data.url = location();
    }

    // 普通の読み込み
    else {

        // レジューム設定
        // 1byte前からレジュームして '\n' が返ってこなかったらあぼーんがあったってこと
        if( get_lng_dat() ) {
            data.byte_readfrom = get_lng_dat() -1;
            // 更新チェックのときは未取得の範囲を指定する
            if( is_checking_update() ) data.byte_readfrom += 1;
            set_resume( true );
        }
        else set_resume( false );

        data.url = get_url();
    }

    data.host_proxy = DBTREE::get_proxy_host( get_url() );
    data.port_proxy = DBTREE::get_proxy_port( get_url() );
    data.basicauth_proxy = DBTREE::get_proxy_basicauth( get_url() );
    data.size_buf = CONFIG::get_loader_bufsize();
    data.timeout = CONFIG::get_loader_timeout();
    data.basicauth = DBTREE::board_basicauth( get_url() );
    data.cookie_for_request = DBTREE::board_cookie_for_request( get_url() );
    data.encoding_analysis_method = DBTREE::board_encoding_analysis_method( get_url() );

    if( ! get_date_modified().empty() ) data.modified = get_date_modified();
}


/**
 * @brief ロード完了
 */
void NodeTree2chCompati::receive_finish()
{
#ifdef _DEBUG
    std::cout << "NodeTree2chCompati::receive_finish : " << get_url()
              << " , mode = " << ( m_mode == Mode::normal ? "normal" : "kako" )
              << ", code = " << get_code() << std::endl;
#endif

    // 更新チェックではない、オンラインの場合はURLリダイレクトを処理する
    if( ! is_checking_update()
        && SESSION::is_online()
        && ( get_code() == HTTP_REDIRECT || get_code() == HTTP_MOVED_PERM || get_code() == HTTP_PERMANENT_REDIRECT )
      ){

        // サーバーから過去ログのURLを通知された
        if( m_mode == Mode::normal ) {

            m_mode = Mode::kako;

#ifdef _DEBUG
            std::cout << "NodeTree2chCompati::receive_finish : switch mode to kako, location = "
                      << location() << std::endl;
#endif
            download_dat( false );
            return;
        }

        // 失敗
        m_mode = Mode::normal;
    }

    // 過去ログから読み込んだ場合は DAT 落ちにする
    if( m_mode != Mode::normal ) {
        m_mode = Mode::normal;
        set_code( HTTP_OLD );
    }

    NodeTreeBase::receive_finish();
}
