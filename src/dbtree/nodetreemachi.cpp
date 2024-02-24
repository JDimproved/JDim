// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "nodetreemachi.h"
#include "interface.h"

#include "jdlib/jdiconv.h"
#include "jdlib/jdregex.h"
#include "jdlib/loaderdata.h"
#include "jdlib/misccharcode.h"
#include "jdlib/miscutil.h"

#include "config/globalconf.h"

#include "global.h"
#include "httpcode.h"

#include <sstream>


using namespace DBTREE;


constexpr size_t BUF_SIZE_200 = 256;


NodeTreeMachi::NodeTreeMachi( const std::string& url, const std::string& date_modified )
    : NodeTreeBase( url, date_modified )
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

    NodeTreeMachi::clear();
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

    m_regex.reset(); // regex 削除
    m_iconv.reset(); // iconv 削除

    m_decoded_lines.clear();
    m_decoded_lines.shrink_to_fit();

    m_buffer_for_200.clear();
    m_buffer_for_200.shrink_to_fit();
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
    if( ! m_regex ) m_regex = std::make_unique<JDLIB::Regex>();

    // iconv 初期化
    if( ! m_iconv ) m_iconv = std::make_unique<JDLIB::Iconv>( Encoding::utf8, DBTREE::article_encoding( get_url() ) );

    m_buffer_for_200.clear();
}




/** @brief ロード用データ作成
 *
 * @details read.cgiを使ってダウンロードする方法はサポートを終了した。(since v0.11.0-20240224) @n
 * また、read.cgiからダウンロードしたHTMLをrawデータに変換する処理 NodeTreeMachi::process_raw_lines() も削除した。
 *
 * @param[out] data ローダーで使うデータを格納する
 */
void NodeTreeMachi::create_loaderdata( JDLIB::LOADERDATA& data )
{
    // レジュームはしない代わりにスレを直接指定
    set_resume( false );

    JDLIB::Regex regex;
    constexpr std::size_t offset = 0;
    constexpr bool icase = false;
    constexpr bool newline = true;
    constexpr bool usemigemo = false;
    constexpr bool wchar = false;

    // read.cgi のdat URLを offlaw.cgi v2 のdat URLに変換する
    if( regex.exec( "(https?://[^/]*)/bbs/read.cgi\\?BBS=([^&]*)&KEY=([0-9]*)",
                    get_url(), offset, icase, newline, usemigemo, wchar ) ) {

        data.url = regex.replace( "\\1/bbs/offlaw.cgi/2/\\2/\\3" );
    }
    else {
        // offlaw.cgi v2 のdat URL
        data.url = get_url();
    }

    if( id_header() >= 1 ) {
        data.url += "/" + std::to_string( id_header() +1 ) + "-";
    }

    data.agent = DBTREE::get_agent( get_url() );
    data.host_proxy = DBTREE::get_proxy_host( get_url() );
    data.port_proxy = DBTREE::get_proxy_port( get_url() );
    data.basicauth_proxy = DBTREE::get_proxy_basicauth( get_url() );
    data.size_buf = CONFIG::get_loader_bufsize();
    data.timeout = CONFIG::get_loader_timeout();
    data.cookie_for_request = DBTREE::board_cookie_for_request( get_url() );
    data.encoding_analysis_method = DBTREE::board_encoding_analysis_method( get_url() );

    if( ! get_date_modified().empty() ) data.modified = get_date_modified();

#ifdef _DEBUG    
    std::cout << "NodeTreeMachi::create_loader : " << data.url << std::endl;
#endif
}


/** @brief raw データを dat 形式に変換 (read.cgi と offlaw.cgi v1, v2 に対応)
 *
 * @details 後方互換性のためofflaw.cgi v1と
 * read.cgiからダウンロードして作成したrawデータをサポートする。
 * @param[in]  rawlines rawデータ
 * @param[out] byte     変換したdatのサイズ
 * @return dat形式のデータ
 */
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
    const std::string& str_lines = m_iconv->convert( rawlines, std::strlen( rawlines ) );

    std::list< std::string > lines = MISC::get_lines( str_lines );
    for( std::string& line : lines ) {

        line = MISC::utf8_trim( line );
        if( line.empty() ) continue;

        int num = 0;
        std::string name;
        std::string mail;
        std::string date;
        std::string body;

        // offlaw 形式
        if( line[0] != '<' ) {

            // offlaw.cgi v1 and v2, 2021-08-04 のバージョンに対応する正規表現
            // http://www.machi.to/bbs/read.cgi/tawara/1416672649/87
            std::string reg( "(.*?)<>(.*?)<>(.*?)<>(.*?)<>(.*?)<>(.*?)(?:<>(.*?))?$" );

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
            // ID: が見つからなければあぼ〜ん
            const auto i = date.rfind( "ID:" );
            if( i == std::string::npos ) continue;
            body = m_regex->str( 5 );
            if( num == 1 ) m_subject_machi = m_regex->str( 6 );

            // ホスト情報を表示するモードのときはID部分を削除して置き換える
            if( ! CONFIG::get_show_machi_id() ) {
                // v1 (num == 1) or v2
                if( m_regex->length( 7 ) > 0 ) {
                    date.resize( i );
                    date += "HOST:" + m_regex->str( 7 );
                }
                // v1 (num >= 2)
                else if( m_regex->length( 6 ) > 0 ) {
                    date.resize( i );
                    date += "HOST:" + m_regex->str( 6 );
                }
            }
        }

        // read.cgi 形式
        else{

            constexpr const char* reg =
                "<dt>([1-9][0-9]*) ?名前：(?:<a href=\"mailto:([^\"]*)\"><b>|<font[^>]*><b>) ?"
                "(?:<font[^>]*>)?([^<]*)(?:</font>)? ?</[bB]>.+ ?投稿日： ?([^>]*)"
                "(?: <font[^>]*>\\[ ?([!-~]*) ?\\]</font>)?<br><dd> ?(.*) ?<br><br>(<script[^>]*>)?$";

            if( ! m_regex->exec( reg, line, offset, icase, newline, usemigemo, wchar ) ){
#ifdef _DEBUG
                std::cout << "NodeTreeMachi::raw2dat read.cgi regex 失敗\n";
                std::cout << line << std::endl;
#endif
                continue;
            }

            num = std::atoi( m_regex->str( 1 ).c_str() );
            name = m_regex->str( 3 );
            mail = m_regex->str( 2 );
            date = m_regex->str( 4 );
            body = m_regex->str( 6 );

            // ホスト情報を表示するモードのときはID部分を削除して置き換える
            if( ! CONFIG::get_show_machi_id() && m_regex->length( 5 ) > 0 ) {
                if( const auto i = date.rfind( "ID:" ); i != std::string::npos ) {
                    date.resize( i );
                }
                else {
                    // IDが見つからないときは日付と区切るため半角空白を追加する
                    date.push_back( ' ' );
                }
                date += "HOST:" + m_regex->str( 5 );
            }
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

    m_decoded_lines = std::move( buffer );
    byte = m_decoded_lines.size();

    return m_decoded_lines.c_str();
}



//
// ローダからデータ受け取り
//
void NodeTreeMachi::receive_data( std::string_view buf )
{
    // dat落ち判定用処理。 receive_finish() も参照
    if( ! is_checking_update() && get_code() == HTTP_OK && m_buffer_for_200.empty() ) {
#ifdef _DEBUG
        std::cout << "NodeTreeMachi::receive_data : save some bytes\n";
#endif

        m_buffer_for_200.append( buf.substr( 0, BUF_SIZE_200 ) );
    }

    NodeTreeBase::receive_data( buf );
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
    if( m_buffer_for_200.size() >= 2 && m_buffer_for_200[ 0 ] == '<'
            && ( m_buffer_for_200[ 1 ] == 'E' || m_buffer_for_200[ 1 ] == 'h' ) ) {

        const std::string& str_lines = m_iconv->convert( m_buffer_for_200.data(), m_buffer_for_200.size() );

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
