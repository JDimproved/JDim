// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "nodetreemachi.h"
#include "interface.h"

#include "jdlib/jdiconv.h"
#include "jdlib/jdregex.h"
#include "jdlib/loaderdata.h"
#include "jdlib/miscutil.h"
#include "jdlib/miscmsg.h"

#include "config/globalconf.h"

#include "global.h"
#include "httpcode.h"

#include <sstream>


using namespace DBTREE;


constexpr size_t BUF_SIZE_200 = 256;


NodeTreeMachi::NodeTreeMachi( const std::string& url, const std::string& date_modified )
    : NodeTreeBase( url, date_modified )
    , m_regex( NULL )
    , m_iconv( NULL )
    , m_decoded_lines( NULL )
    , m_buffer( NULL )
    , m_buffer_for_200( NULL )
{
#ifdef _DEBUG
    std::cout << "NodeTreeMachi::NodeTreeMachi url = " << get_url() << " modified = " << date_modified << std::endl;
#endif
}


NodeTreeMachi::~NodeTreeMachi()
{
#ifdef _DEBUG    
    std::cout << "NodeTreeMachi::~NodeTreeMachi : " << get_url() << std::endl;
#endif

    clear();
}


//
// バッファなどのクリア
//
void NodeTreeMachi::clear()
{
#ifdef _DEBUG    
    std::cout << "NodeTreeMachi::clear : " << get_url() << std::endl;
#endif
    NodeTreeBase::clear();

    // regex 削除
    if( m_regex ) delete m_regex;
    m_regex = NULL;

    // iconv 削除
    if( m_iconv ) delete m_iconv;
    m_iconv = NULL;

    if( m_decoded_lines ) free( m_decoded_lines );
    m_decoded_lines = NULL;

    if( m_buffer ) free( m_buffer );
    m_buffer = NULL;

    if( m_buffer_for_200 ) free( m_buffer_for_200 );
    m_buffer_for_200 = NULL;
}



//
// ロード実行前に呼ぶ初期化関数
//
void NodeTreeMachi::init_loading()
{
#ifdef _DEBUG    
    std::cout << "NodeTreeMachi::init_loading : " << get_url() << std::endl;
#endif

    NodeTreeBase::init_loading();

    // regex 初期化
    if( ! m_regex ) m_regex = new JDLIB::Regex();

    // iconv 初期化
    std::string charset = DBTREE::board_charset( get_url() );
    if( ! m_iconv ) m_iconv = new JDLIB::Iconv( charset, "UTF-8" );

    if( ! m_decoded_lines ) m_decoded_lines = ( char* )malloc( BUF_SIZE_ICONV_OUT );
    if( ! m_buffer ) m_buffer = ( char* )malloc( BUF_SIZE_ICONV_OUT + 64 );

    if( m_buffer_for_200 ) m_buffer_for_200[ 0 ] = '\0';

    m_tmp_buffer = std::string();
}




//
// ロード用データ作成
//
void NodeTreeMachi::create_loaderdata( JDLIB::LOADERDATA& data )
{
    // レジュームはしない代わりにスレを直接指定
    set_resume( false );

    // offlaw 形式
    if( CONFIG::get_use_machi_offlaw() ){

        JDLIB::Regex regex;
        const size_t offset = 0;
        const bool icase = false;
        const bool newline = true;
        const bool usemigemo = false;
        const bool wchar = false;

        if( regex.exec( "(https?://[^/]*)/bbs/read.cgi\\?BBS=([^&]*)&KEY=([0-9]*)", get_url(), offset, icase, newline, usemigemo, wchar ) ){

            data.url = regex.str( 1 ) + std::string( "/bbs/offlaw.cgi/" ) + regex.str( 2 ) + std::string( "/" ) +  regex.str( 3 );
            if( id_header() >= 1 ) data.url += "/" + MISC::itostr( id_header() +1 ) + "-";
        }
    }

    // read.cgi 形式
    else{

        data.url = get_url();
        if( id_header() ) data.url += "&START=" + MISC::itostr( id_header() + 1 );
    }

    data.agent = DBTREE::get_agent( get_url() );
    data.host_proxy = DBTREE::get_proxy_host( get_url() );
    data.port_proxy = DBTREE::get_proxy_port( get_url() );
    data.basicauth_proxy = DBTREE::get_proxy_basicauth( get_url() );
    data.size_buf = CONFIG::get_loader_bufsize();
    data.timeout = CONFIG::get_loader_timeout();

    if( ! get_date_modified().empty() ) data.modified = get_date_modified();

#ifdef _DEBUG    
    std::cout << "NodeTreeMachi::create_loader : " << data.url << std::endl;
#endif
}



//
// キャッシュに保存する前の前処理
//
char* NodeTreeMachi::process_raw_lines( char* rawlines )
{
    // オフラインか offlaw 形式を使用する場合はそのまま返す
    if( ! is_loading() || CONFIG::get_use_machi_offlaw() ) return rawlines;

    const size_t offset = 0;
    const bool icase = false;
    const bool newline = true;
    const bool usemigemo = false;
    const bool wchar = false;

    std::string buffer;

    // オンラインでかつ read.cgi 形式の場合は
    // 入力データを行ごとに分割して余計なタグを取り除いて本文だけ取り出す
    std::list< std::string > lines = MISC::get_lines( rawlines );
    std::list< std::string >::iterator it;
    for( it = lines.begin(); it != lines.end(); ++it ){

        std::string line = MISC::remove_space( *it );

        if( m_tmp_buffer.empty() ){

            if( line.find( "<dt>" ) == 0 ){

                // 既に読み込んでいる場合は飛ばす
                char num_tmp[ 8 ];
                memcpy( num_tmp, line.c_str() + strlen( "<dt>" ), 5 );
                num_tmp[ 5 ] = '0';
                if( atoi( num_tmp ) <= id_header() ) continue;

                // 行の途中で改行が入ったときは一時バッファに貯めておく
                if( line.find( "<dd>" ) != std::string::npos ){
                    buffer += line;
                    buffer += "\n";
                }
                else m_tmp_buffer = line;
            }

            // タイトル取得
            else if( ! id_header() && m_subject_machi.empty() ){

                std::string reg_subject( "<title>([^<]*)</title>" );
                if( m_regex->exec( reg_subject, line, offset, icase, newline, usemigemo, wchar ) ){

                    const std::string charset = DBTREE::board_charset( get_url() );
                    m_subject_machi = MISC::Iconv( m_regex->str( 1 ), charset, "UTF-8" );
#ifdef _DEBUG
                    std::cout << "NodeTreeMachi::process_raw_lines\n";
                    std::cout << "subject = " << m_subject_machi << std::endl;
#endif
                }
            }

        }
        else{

            if( line.find( "<dd>" ) != std::string::npos ){

                buffer += m_tmp_buffer;
                buffer += line;
                buffer += "\n";

                m_tmp_buffer.clear();
            }
        }
    }

    if( buffer.length() > BUF_SIZE_ICONV_OUT ){
        MISC::ERRMSG( "buffer over flow in NodeTreeMachi::process_raw_lines" );
        buffer = std::string();
    }

    int byte = buffer.length();
    memcpy( m_buffer, buffer.c_str(), byte );
    m_buffer[ byte ] = '\0';

    return m_buffer;
}


//
// raw データを dat 形式に変換
//
const char* NodeTreeMachi::raw2dat( char* rawlines, int& byte )
{
#ifdef _DEBUG
    std::cout << "NodeTreeMachi::raw2dat\n";
#endif

    const size_t offset = 0;
    const bool icase = false;
    const bool newline = true;
    const bool usemigemo = false;
    const bool wchar = false;

    int next = id_header() + 1;

    std::string buffer;

    // 文字コード変換
    int byte_lines;
    const char* str_lines = m_iconv->convert( rawlines, strlen( rawlines ), byte_lines );

    std::list< std::string > lines = MISC::get_lines( str_lines );
    std::list< std::string >::iterator it;
    for( it = lines.begin(); it != lines.end(); ++it ){

        std::string line = MISC::remove_space( *it );
        if( line.empty() ) continue;

        int num = 0;
        std::string name;
        std::string mail;
        std::string date;
        std::string body;

        // offlaw 形式
        if( line.c_str()[ 0 ] != '<' ){

            std::string reg( "(.*?)<>(.*?)<>(.*?)<>(.*?)<>(.*?)<>(.*?)$");

            if( ! m_regex->exec( reg, line, offset, icase, newline, usemigemo, wchar ) ){
#ifdef _DEBUG
                std::cout << "失敗\n";
                std::cout << line << std::endl;
#endif
                continue;
            }

            num = atoi( m_regex->str( 1 ).c_str() );
            name = m_regex->str( 2 );
            mail = m_regex->str( 3 );
            date = m_regex->str( 4 );
            body = m_regex->str( 5 );
            if( num == 1 ) m_subject_machi = m_regex->str( 6 );
        }

        // read.cgi 形式
        else{

            std::string reg( "<dt>([1-9][0-9]*) ?名前：(<a href=\"mailto:([^\"]*)\"><b>|<font[^>]*><b>) ?(<font[^>]*>)?([^<]*)(</font>)? ?</[bB]>.+ ?投稿日： ?([^<]*)( <font[^>]*>\\[ ?(.*) ?\\]</font>)?<br><dd> ?(.*) ?<br><br>$" );

            if( ! m_regex->exec( reg, line, offset, icase, newline, usemigemo, wchar ) ){
#ifdef _DEBUG
                std::cout << "失敗\n";
                std::cout << line << std::endl;
#endif
                continue;
            }

            num = atoi( m_regex->str( 1 ).c_str() );
            name = m_regex->str( 5 );
            mail = m_regex->str( 3 );
            date = m_regex->str( 7 );
            if( !m_regex->str( 9 ).empty() ) date += " HOST:" + m_regex->str( 9 );
            body = m_regex->str( 10 );
        }

        while( next < num ){
#ifdef _DEBUG
            std::cout << "abone = " << num << std::endl;
#endif
            buffer += "あぼ〜ん<><>あぼ〜ん<> あぼ〜ん <><>\n";
            next++;
        }

        if( num == 1 ){
#ifdef _DEBUG
            std::cout << "subject = " << m_subject_machi << std::endl;
#endif
            buffer = name + "<>" + mail + "<>" + date + "<> " + body + " <>" + m_subject_machi + "<>\n";
        }
        else buffer += name + "<>" + mail + "<>" + date + "<> " + body + " <><>\n";

        ++next;
    }

    if( buffer.length() > BUF_SIZE_ICONV_OUT ){
        MISC::ERRMSG( "buffer over flow in NodeTreeMachi::process_raw_lines" );
        buffer = std::string();
    }

    byte = buffer.length();
    memcpy( m_decoded_lines, buffer.c_str(), byte );
    m_decoded_lines[ byte ] = '\0';

    return m_decoded_lines;
}



//
// ローダからデータ受け取り
//
void NodeTreeMachi::receive_data( const char* data, size_t size )
{
    // dat落ち判定用処理。 receive_finish() も参照
    if( ! is_checking_update() && get_code() == HTTP_OK
        && ( ! m_buffer_for_200 || m_buffer_for_200[ 0 ] == '\0' ) ){
#ifdef _DEBUG
        std::cout << "NodeTreeMachi::receive_data : save some bytes\n";
#endif

        if( ! m_buffer_for_200 ) m_buffer_for_200 = ( char* )malloc( BUF_SIZE_200 + 64 );
        const int lng = MIN( size, BUF_SIZE_200 );
        memcpy( m_buffer_for_200, data, lng );
        m_buffer_for_200[ lng ] = '\0';
    }

    NodeTreeBase::receive_data( data, size );
}


//
// ロード完了
//
void NodeTreeMachi::receive_finish()
{
#ifdef _DEBUG    
    std::cout << "NodeTreeMachi::receive_finish : " << get_url() << std::endl
              << " code = " << get_code() << std::endl;
#endif

    // dat落ち判定
    if( m_buffer_for_200 && m_buffer_for_200[ 0 ] == '<' && ( m_buffer_for_200[ 1 ] == 'E' || m_buffer_for_200[ 1 ] == 'h' )
        ){

        int byte_lines;
        std::string str_lines( m_iconv->convert( m_buffer_for_200, strlen( m_buffer_for_200 ), byte_lines ) );

#ifdef _DEBUG    
        std::cout << str_lines << std::endl;
#endif

        if( str_lines.find( "<ERROR>" ) != std::string::npos
            || str_lines.find( "<html><head></head><body>\nなんらかの原因により、まちＢＢＳサーバ内にログを見つけることができませんでした。" ) != std::string::npos
            ){

#ifdef _DEBUG    
            std::cout << "not found\n";
#endif
            set_code( HTTP_NOT_FOUND );
            set_str_code( "Not Found" );
        }
    }

    NodeTreeBase::receive_finish();
}
