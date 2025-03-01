// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "nodetree2ch.h"
#include "interface.h"

#include "jdlib/jdregex.h"
#include "jdlib/loaderdata.h"
#include "jdlib/miscutil.h"

#include "config/globalconf.h"

#include "httpcode.h"
#include "session.h"
#include "login2ch.h"

#include <cstdlib>
#include <sstream>


using namespace DBTREE;


enum
{
    MODE_NORMAL = 0,
    MODE_KAKO,
    MODE_OLDURL,
    MODE_KAKO_PROXY,
    MODE_OLDURL_PROXY,
};


NodeTree2ch::NodeTree2ch( const std::string& url, const std::string& org_url,
                          const std::string& date_modified, time_t since_time )
    : NodeTree2chCompati( url, date_modified )
    , m_org_url( org_url )
    , m_since_time( since_time )
    , m_mode( MODE_NORMAL )
    , m_res_number_max( -1 )
{
#ifdef _DEBUG
    std::cout << "NodeTree2ch::NodeTree2ch url = " << url << std::endl
              << "org_url = " << m_org_url << " modified = " << date_modified
              << " since = " << m_since_time << std::endl;
#endif
}



NodeTree2ch::~NodeTree2ch()
{
#ifdef _DEBUG    
    std::cout << "NodeTree2ch::~NodeTree2ch : " << get_url() << std::endl;
#endif
}


/** @brief 5chスレッドの拡張属性を取り出す
 *
 * @param[in] str スレのレス番号1(`>>1`)の本文テキストデータ
 */
void NodeTree2ch::parse_extattr( std::string_view str )
{
    std::size_t pos = str.rfind( "<hr>VIPQ2_EXTDAT: " );
    if( pos == std::string_view::npos ) return;
    pos += 18;
    if( pos >= str.size() ) return;

    JDLIB::Regex regex;
    constexpr std::size_t offset = 0;
    constexpr bool icase = false; // 大文字小文字区別しない
    constexpr bool newline = true; // . に改行をマッチさせない
    constexpr bool usemigemo = false; // migemo使用
    constexpr bool wchar = false; // 全角半角の区別をしない

    str.remove_prefix( pos );
    const std::string extattr{ str };
    if( regex.exec( "[^:]+:[^:]+:([^:]+):(?:([^:]+):)?[^ ]+ EXT was configured",
                    extattr, offset, icase, newline, usemigemo, wchar ) ) {

        // 最大レス数を取得
        std::string num_str = regex.str( 1 );
        if( num_str == "V" ) m_res_number_max = 0;
        else if( num_str[0] >= '0' && num_str[0] <= '9' ) {
            m_res_number_max = std::atoi( num_str.c_str() );
        }
        // 最大DATサイズ(KB)を取得
        num_str = regex.str( 2 );
        if( num_str[0] >= '0' && num_str[0] <= '9' ) {
            m_dat_volume_max = std::atoi( num_str.c_str() );
        }
    }
}


//
// ロード用データ作成
//
void NodeTree2ch::create_loaderdata( JDLIB::LOADERDATA& data )
{
#ifdef _DEBUG    
    std::cout << "NodeTree2ch::create_loaderdata : mode = " << m_mode << " url = " << get_url() << std::endl;

#endif

    data.url = std::string();
    data.byte_readfrom = 0;

    // ( rokka, 旧offlaw, offlaw2は廃止 )
    // DAT落ちした現役サーバに収容されているスレッド
    // または旧URL(過去ログサーバ)に収容されているスレッド
    if( m_mode == MODE_KAKO || m_mode == MODE_OLDURL ) {

        JDLIB::Regex regex;
        const size_t offset = 0;
        const bool icase = false;
        const bool newline = true;
        const bool usemigemo = false;
        const bool wchar = false;

        const std::string& url = ( m_mode == MODE_OLDURL ) ? m_org_url : get_url();

        if( ! regex.exec( "(https?://[^/]*)(/.*)/dat(/.*)\\.dat$", url, offset, icase, newline, usemigemo, wchar ) ) return;

        std::ostringstream ss;

        // 過去ログURLの構築 (9桁のURLは2024年2月頃から上位4桁に移行しています)
        // スレIDが10桁の場合 -> https://サーバ/板ID/oyster/IDの上位4桁/ID.dat
        // スレIDが 9桁の場合 -> https://サーバ/板ID/oyster/IDの上位4桁/ID.dat
        const std::string id_high4 = regex.str( 3 ).substr( 1, 4 );
        ss << regex.str( 1 ) << regex.str( 2 ) << "/oyster/" << id_high4 << regex.str( 3 ) << ".dat";

        // レジューム設定
        // DATを読み込んでいた場合はレジュームを有りにして、DATの未取得部分を追加するように処理する
        set_resume( get_lng_dat() > 0 );

        data.url = ss.str();
    }

    // 普通の読み込み
    else{

        // レジューム設定
        // 1byte前からレジュームして '\n' が返ってこなかったらあぼーんがあったってこと
        if( get_lng_dat() ) {
            data.byte_readfrom = get_lng_dat() -1;
            // 更新チェックのときは未取得の範囲を指定する
            if( is_checking_update() ) data.byte_readfrom += 1;
            set_resume( true );
        }
        else set_resume( false );

        data.url = ( m_mode == MODE_OLDURL_PROXY ) ? m_org_url : get_url();
    }

#ifdef _DEBUG    
    std::cout << "load from " << data.url << std::endl;
#endif


    data.agent = DBTREE::get_agent( get_url() );

#ifdef _DEBUG
    std::cout << "agent = " << data.agent << std::endl;
#endif

    if( m_mode == MODE_KAKO_PROXY || m_mode == MODE_OLDURL_PROXY ) {
        data.host_proxy = CONFIG::get_proxy_for2ch();
        data.port_proxy = CONFIG::get_proxy_port_for2ch();
        data.basicauth_proxy = CONFIG::get_proxy_basicauth_for2ch();
    }
    else {
        data.host_proxy = DBTREE::get_proxy_host( get_url() );
        data.port_proxy = DBTREE::get_proxy_port( get_url() );
        data.basicauth_proxy = DBTREE::get_proxy_basicauth( get_url() );
    }
    data.cookie_for_request = DBTREE::board_cookie_for_request( get_url() );

    data.size_buf = CONFIG::get_loader_bufsize();
    data.timeout = CONFIG::get_loader_timeout();
    data.encoding_analysis_method = DBTREE::board_encoding_analysis_method( get_url() );

    if( ! get_date_modified().empty() ) data.modified = get_date_modified();
}


//
// ロード完了
//
void NodeTree2ch::receive_finish()
{
#ifdef _DEBUG    
    std::cout << "NodeTree2ch::receive_finish : " << get_url() << std::endl
              << "mode = " << m_mode << " code = " << get_code() << std::endl;
#endif

    // 更新チェックではない、オンラインの場合は過去ログ倉庫から取得出来るか試みる
    if( ! is_checking_update()
        && SESSION::is_online()
        && ( get_code() == HTTP_REDIRECT || get_code() == HTTP_MOVED_PERM || get_code() == HTTP_PERMANENT_REDIRECT
             || get_code() == HTTP_NOT_FOUND )
        ){

/*
        DATファイルへのアクセス方法 ( 2023-07-11 からの仕様 )
        参考文献: https://agree.5ch.net/test/read.cgi/operate/9240230711/3

        ・スレIDが10桁の場合 -> https://サーバ/板ID/oyster/IDの上位4桁/ID.dat

        (例) https://HOGE.5ch.net/test/read.cgi/hoge/1234567890/ を取得
        (1) https://HOGE.5ch.net/hoge/dat/1234567890.dat から DAT を取得。
        (2) https://HOGE.5ch.net/hoge/oyster/1234/1234567890.dat から取得。
        (3) 旧URL(過去ログサーバ)がある場合、そのURLから取得
        取得できなかったとき -> 最初にプロキシを使わない接続で読み込めるか試す設定なら(4)へ

        (4) プロキシに接続して現役サーバから取得
        (5) 旧URL(過去ログサーバ)がある場合、プロキシに接続してそのURLから取得

        ・スレIDが9桁の場合 -> https://サーバ/板ID/oyster/IDの上位3桁/ID.dat

        (例) https://HOGE.5ch.net/test/read.cgi/hoge/123456789/ を取得
        (1) https://HOGE.5ch.net/hoge/dat/123456789.dat から DAT を取得。
        (2) https://HOGE.5ch.net/hoge/oyster/123/123456789.dat から取得。
        (3) 旧URL(過去ログサーバ)がある場合、そのURLから取得
        取得できなかったとき -> 最初にプロキシを使わない接続で読み込めるか試す設定なら(4)へ

        (4) プロキシに接続して現役サーバから取得
        (5) 旧URL(過去ログサーバ)がある場合、プロキシに接続してそのURLから取得
*/

        // DAT落ちした現役サーバに収容されているスレッド
        if( m_mode == MODE_NORMAL ) m_mode = MODE_KAKO;

        // 旧URL(過去ログサーバ)に収容されているスレッド
        else if( m_mode == MODE_KAKO && get_url() != m_org_url ) m_mode = MODE_OLDURL;

        // プロキシを使わない接続で過去ログが見つからなかったときは2ch読み込み用プロキシを使うモード
        else if( ( m_mode == MODE_KAKO || m_mode == MODE_OLDURL )
                 && CONFIG::get_use_fallback_proxy_for2ch()
                 && ! CONFIG::get_proxy_for2ch().empty() ) m_mode = MODE_KAKO_PROXY;

        // 現役サーバで見つからなかったときは旧URL(過去ログサーバ)に切り替えて試す
        else if( m_mode == MODE_KAKO_PROXY && get_url() != m_org_url ) m_mode = MODE_OLDURL_PROXY;

        // 失敗
        else m_mode = MODE_NORMAL;

#ifdef _DEBUG    
        std::cout << "switch mode to " << m_mode << std::endl;
#endif
        if( m_mode != MODE_NORMAL ){
            download_dat( is_checking_update() );
            return;
        }
    }

    // 過去ログから読み込んだ場合は DAT 落ちにする
    if( m_mode != MODE_NORMAL ){
        m_mode = MODE_NORMAL;
        set_code( HTTP_OLD );
    }

    NodeTreeBase::receive_finish();
}
