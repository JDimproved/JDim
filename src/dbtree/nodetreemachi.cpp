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

#include <sstream>


using namespace DBTREE;


NodeTreeMachi::NodeTreeMachi( const std::string url, const std::string& date_modified )
    : NodeTreeBase( url, date_modified )
    , m_regex( 0 )
    , m_iconv( 0 )
    , m_decoded_lines( 0 )
    , m_buffer( 0 )
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

    m_tmp_buffer = std::string();
}




//
// ロード用データ作成
//
void NodeTreeMachi::create_loaderdata( JDLIB::LOADERDATA& data )
{
    std::stringstream ss;
    ss << get_url();

    // レジュームはしない代わりにスレを直接指定
    set_resume( false );
    if( id_header() ) ss << "&START=" << id_header() + 1;
    data.url = ss.str();
    data.agent = DBTREE::get_agent( get_url() );
    data.host_proxy = DBTREE::get_proxy_host( get_url() );
    data.port_proxy = DBTREE::get_proxy_port( get_url() );
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
// 余計なタグを取り除いて本文だけ取り出す
//
char* NodeTreeMachi::process_raw_lines( char* rawlines )
{
    std::string buffer;

    // 入力データを行ごとに分割して本文だけ取り出す
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

    int next = id_header() + 1;

    std::string reg( "<dt>([1-9][0-9]*) ?名前：(<a href=\"mailto:([^\"]*)\"><b>|<font[^>]*><b>) ?(<font[^>]*>)?([^<]*)(</font>)? ?</[bB]>.+ ?投稿日： ?([^<]*)( <font[^>]*>\\[ ?(.*) ?\\]</font>)?<br><dd> ?(.*) ?<br><br>$" );

    std::string buffer;

    // 文字コード変換
    int byte_lines;
    const char* str_lines = m_iconv->convert( rawlines, strlen( rawlines ), byte_lines );

    // 入力データを行ごとに分割して本文だけ取り出す
    std::list< std::string > lines = MISC::get_lines( str_lines );
    std::list< std::string >::iterator it;
    for( it = lines.begin(); it != lines.end(); ++it ){

        std::string line = MISC::remove_space( *it );
        if( line.empty() ) continue;

        if( ! m_regex->exec( reg, line ) ){
#ifdef _DEBUG
            std::cout << "失敗\n";
            std::cout << line << std::endl;
#endif
            continue;
        }

#ifdef _DEBUG
/*
        std::cout << "1 " << m_regex->str( 1 ) << std::endl;
        std::cout << "2 " << m_regex->str( 2 ) << std::endl;
        std::cout << "3 " << m_regex->str( 3 ) << std::endl;
        std::cout << "4 " << m_regex->str( 4 ) << std::endl;
        std::cout << "5 " << m_regex->str( 5 ) << std::endl;
        std::cout << "6 " << m_regex->str( 6 ) << std::endl;
        std::cout << "7 " << m_regex->str( 7 ) << std::endl;
        std::cout << "8 " << m_regex->str( 8 ) << std::endl;
        std::cout << "9 " << m_regex->str( 9 ) << std::endl;
        std::cout << "10 " << m_regex->str( 10 ) << std::endl;
*/
#endif
        int num = atoi( m_regex->str( 1 ).c_str() );
        std::string name = m_regex->str( 5 );
        std::string mail = m_regex->str( 3 );
        std::string date = m_regex->str( 7 );
        if( !m_regex->str( 9 ).empty() ) date += " HOST:" + m_regex->str( 9 );
        std::string body = m_regex->str( 10 );

        while( next < num ){
#ifdef _DEBUG
            std::cout << "abone = " << num << std::endl;
#endif
            buffer += "あぼ〜ん<><><> あぼ〜ん <><>\n";
            next++;
        }
        
        buffer += name + "<>" + mail + "<>" + date + "<> " + body + " <><>\n";
        next++;
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
