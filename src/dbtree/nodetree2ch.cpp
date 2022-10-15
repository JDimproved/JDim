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

#include <sstream>

using namespace DBTREE;


enum
{
    MODE_NORMAL = 0,
    MODE_OFFLAW,
    MODE_KAKO_GZ,
    MODE_KAKO,
    MODE_OLDURL
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


//
// キャッシュに保存する前の前処理
//
// 先頭にrawモードのステータスが入っていたら取り除く
//
char* NodeTree2ch::process_raw_lines( char* rawlines )
{
    char* pos = rawlines;

    if( m_mode == MODE_OFFLAW ){
        // rokka独自のステータスが入っている

        int status = 0;
        if( strncmp( pos, "Success", 7 ) == 0 ) status = 1;
        if( strncmp( pos, "Error", 5 ) == 0 ) status = 2;

#ifdef _DEBUG
        std::cout << "NodeTree2ch::process_raw_lines : raw mode status = " << status << std::endl;
#endif

        if( status != 0 ){
            pos = skip_status_line( pos, status );
        }
    }

    else {
        pos = NodeTree2chCompati::process_raw_lines( rawlines );
    }

    return pos;
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

    //rokka使用 (旧offlaw, offlaw2は廃止)
    if( m_mode == MODE_OFFLAW ){

        JDLIB::Regex regex;
        const size_t offset = 0;
        const bool icase = false;
        const bool newline = true;
        const bool usemigemo = false;
        const bool wchar = false;

        if( ! regex.exec( "(https?://)([^/\\.]+)(\\.[^/]+)(/.*)/dat(/.*)\\.dat$", m_org_url, offset, icase, newline, usemigemo, wchar ) ) return;

        // http://rokka(.2ch.net|.bbspink.com)/<SERVER NAME>/<BOARD NAME>/<DAT NUMBER>/<OPTIONS>?sid=<SID>
        std::ostringstream ss;
        ss << regex.str( 1 ) << "rokka" << regex.str( 3 ) << "/" << regex.str( 2 )
           << regex.str( 4 ) << regex.str( 5 );

        std::string sid = CORE::get_login2ch()->get_sessionid();
        ss << "/?sid=" << MISC::url_encode( sid.c_str(), sid.length() );

        // レジューム設定
        // レジュームを有りにして、サーバが range を無視して送ってきた場合と同じ処理をする
        if( get_lng_dat() ) {
            set_resume( true );
        }
        else set_resume( false );

        data.url = ss.str();
    }

    // 過去ログ倉庫使用
    else if( m_mode == MODE_KAKO_GZ || m_mode == MODE_KAKO ){

        JDLIB::Regex regex;
        const size_t offset = 0;
        const bool icase = false;
        const bool newline = true;
        const bool usemigemo = false;
        const bool wchar = false;

        if( ! regex.exec( "(https?://[^/]*)(/.*)/dat(/.*)\\.dat$", m_org_url, offset, icase, newline, usemigemo, wchar ) ) return;
        const int id = atoi( regex.str( 3 ).c_str() + 1 );

        std::ostringstream ss;

        // スレIDが10桁の場合 → http://サーバ/板ID/kako/IDの上位4桁/IDの上位5桁/ID.dat.gz
        if( id / 1000000000 ) ss << regex.str( 1 ) << regex.str( 2 ) << "/kako/" << ( id / 1000000 ) << "/" << ( id / 100000 ) << regex.str( 3 );

        // スレIDが9桁の場合 → http://サーバ/板ID/kako/IDの上位3桁/ID.dat.gz
        else ss << regex.str( 1 ) << regex.str( 2 ) << "/kako/" << ( id / 1000000 ) << regex.str( 3 );

        if( m_mode == MODE_KAKO_GZ ) ss << ".dat.gz";
        else ss << ".dat";

        // レジュームは無し
        set_resume( false );

        data.url = ss.str();
    }

    // 普通もしくは旧URLからの読み込み
    else{

        // レジューム設定
        // 1byte前からレジュームして '\n' が返ってこなかったらあぼーんがあったってこと
        if( get_lng_dat() ) {
            data.byte_readfrom = get_lng_dat() -1;
            set_resume( true );
        }
        else set_resume( false );

        data.url = ( m_mode == MODE_OLDURL ) ? m_org_url : get_url();
    }

#ifdef _DEBUG    
    std::cout << "load from " << data.url << std::endl;
#endif


    data.agent = DBTREE::get_agent( get_url() );

#ifdef _DEBUG
    std::cout << "agent = " << data.agent << std::endl;
#endif

    data.host_proxy = DBTREE::get_proxy_host( get_url() );
    data.port_proxy = DBTREE::get_proxy_port( get_url() );
    data.basicauth_proxy = DBTREE::get_proxy_basicauth( get_url() );
    data.cookie_for_request = DBTREE::board_cookie_for_request( get_url() );

    data.size_buf = CONFIG::get_loader_bufsize();
    data.timeout = CONFIG::get_loader_timeout();

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

    // 更新チェックではない、オンラインの場合は offlaw や 過去ログ倉庫から取得出来るか試みる
    if( ! is_checking_update()
        && SESSION::is_online()
        && ( get_code() == HTTP_REDIRECT || get_code() == HTTP_MOVED_PERM || get_code() == HTTP_NOT_FOUND
             || ( m_mode == MODE_OFFLAW && ! get_ext_err().empty() ) // rokka 読み込み失敗
            )
        ){

/*

  ・スレIDが10桁の場合 → http://サーバ/板ID/kako/IDの上位4桁/IDの上位5桁/ID.dat.gz

  (例) http://HOGE.2ch.net/test/read.cgi/hoge/1234567890/ を取得

  (1) http://HOGE.2ch.net/hoge/dat/1234567890.dat から dat を取得。302で●がある場合(2-1)、旧URLがある場合(2-2)、無い場合は(2-3)へ(※)

  (2-1) offlaw.cgiを使って取得

  (2-2) 旧URLから取得

  (2-3) http://HOGE.2ch.net/hoge/kako/1234/12345/1234567890.dat.gz から取得。302なら(3)へ

  (3) http://HOGE.2ch.net/hoge/kako/1234/12345/1234567890.dat から取得 


  ・スレIDが9桁の場合 → http://サーバ/板ID/kako/IDの上位3桁/ID.dat.gz

  (例) http://HOGE.2ch.net/test/read.cgi/hoge/123456789/ を取得

  (1) http://HOGE.2ch.net/hoge/dat/1234567890.dat から dat を取得。302で●がある場合(2-1)、旧URLがある場合(2-2)、無い場合は(2-3)へ(※)

  (2-1) offlaw.cgiを使って取得

  (2-2) 旧URLから取得

  (2-3) http://HOGE.2ch.net/hoge/kako/123/123456789.dat.gz から取得。302なら(3)へ

  (3) http://HOGE.2ch.net/hoge/kako/123/123456789.dat から取得 


  (※)ただし 2008年1月1日以降に立てられたスレは除く

  (注) 古すぎる(2000年頃)のdatは形式が違う(<>ではなくて,で区切られている)ので読み込みに失敗する

*/

        // ログインしている場合は rokka 経由で旧URLで再取得
        if( m_mode == MODE_NORMAL && CORE::get_login2ch()->login_now() ) m_mode = MODE_OFFLAW;

        // 旧URLがある場合、そのURLで再取得
        else if( ( m_mode == MODE_NORMAL || m_mode == MODE_OFFLAW ) && get_url() != m_org_url ) m_mode = MODE_OLDURL;

        // 過去ログ倉庫(gz圧縮)
        // ただし 2008年1月1日以降に立てられたスレは除く
        else if( ( m_mode == MODE_NORMAL || m_mode == MODE_OFFLAW || m_mode == MODE_OLDURL ) && m_since_time < 1199113200 ) m_mode = MODE_KAKO_GZ;

        // 過去ログ倉庫
        else if( m_mode == MODE_KAKO_GZ ) m_mode = MODE_KAKO;

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

    // offlaw や 過去ログから読み込んだ場合は DAT 落ちにする
    if( m_mode != MODE_NORMAL ){
        m_mode = MODE_NORMAL;
        set_code( HTTP_OLD );
    }

    NodeTreeBase::receive_finish();
}
