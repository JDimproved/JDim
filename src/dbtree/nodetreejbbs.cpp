// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "nodetreejbbs.h"
#include "interface.h"

#include "jdlib/jdiconv.h"
#include "jdlib/loaderdata.h"

#include "config/globalconf.h"

#include "global.h"

#include <sstream>


#define APPEND_SECTION( num ) do {\
if( lng_sec[ num ] ){ \
assert( m_decoded_lines.size() + lng_sec[ num ] < BUF_SIZE_ICONV_OUT ); \
m_decoded_lines.append( lines + pos_sec[ num ], lng_sec[ num ] ); \
} } while( 0 )


using namespace DBTREE;


NodeTreeJBBS::NodeTreeJBBS( const std::string& url, const std::string& date_modified )
    : NodeTreeBase( url, date_modified )
    , m_iconv( nullptr )
{
#ifdef _DEBUG
    std::cout << "NodeTreeJBBS::NodeTreeJBBS url = " << get_url() << " modified = " << date_modified << std::endl;
#endif
}


NodeTreeJBBS::~NodeTreeJBBS()
{
#ifdef _DEBUG    
    std::cout << "NodeTreeJBBS::~NodeTreeJBBS : " << get_url() << std::endl;
#endif

    NodeTreeJBBS::clear();
}


//
// バッファなどのクリア
//
void NodeTreeJBBS::clear()
{
#ifdef _DEBUG    
    std::cout << "NodeTreeJBBS::clear : " << get_url() << std::endl;
#endif
    NodeTreeBase::clear();

    // iconv 削除
    if( m_iconv ) delete m_iconv;
    m_iconv = nullptr;

    m_decoded_lines.clear();
    m_decoded_lines.shrink_to_fit();
}



//
// ロード実行前に呼ぶ初期化関数
//
void NodeTreeJBBS::init_loading()
{
#ifdef _DEBUG    
    std::cout << "NodeTreeJBBS::init_loading : " << get_url() << std::endl;
#endif

    NodeTreeBase::init_loading();

    // iconv 初期化
    std::string charset = DBTREE::board_charset( get_url() );
    if( ! m_iconv ) m_iconv = new JDLIB::Iconv( charset, "UTF-8" );

    if( m_decoded_lines.capacity() < BUF_SIZE_ICONV_OUT ) {
        m_decoded_lines.reserve( BUF_SIZE_ICONV_OUT );
    }
}




//
// ロード用データ作成
//
void NodeTreeJBBS::create_loaderdata( JDLIB::LOADERDATA& data )
{
    std::stringstream ss;
    ss << get_url() << "/";

    // レジュームはしない代わりにスレを直接指定
    set_resume( false );
    if( id_header() ) ss << id_header() + 1 << "-";

    data.url = ss.str();
    data.agent = DBTREE::get_agent( get_url() );
    data.host_proxy = DBTREE::get_proxy_host( get_url() );
    data.port_proxy = DBTREE::get_proxy_port( get_url() );
    data.basicauth_proxy = DBTREE::get_proxy_basicauth( get_url() );
    data.size_buf = CONFIG::get_loader_bufsize();
    data.timeout = CONFIG::get_loader_timeout();
    data.cookie_for_write = DBTREE::board_cookie_for_write( get_url() );

    if( ! get_date_modified().empty() ) data.modified = get_date_modified();

#ifdef _DEBUG    
    std::cout << "NodeTreeJBBS::create_loader : " << data.url << std::endl;
#endif
}


//
// raw データを dat 形式に変換
//

enum
{
    MIN_SECTION = 5,
    MAX_SECTION = 8
};

const char* NodeTreeJBBS::raw2dat( char* rawlines, int& byte )
{
    assert( m_iconv != nullptr );

    int byte_lines;
    const char* lines = m_iconv->convert( rawlines, strlen( rawlines ), byte_lines );

    int number = id_header() + 1;

#ifdef _DEBUG
    std::cout << "NodeTreeJBBS::raw2dat : byte_lines = " << byte_lines << std::endl;
#endif

    // セクション分けして再合成する
    m_decoded_lines.clear();
    byte = 0;
    int pos = 0;
    int section = 0;
    int pos_sec[ MAX_SECTION ];
    int lng_sec[ MAX_SECTION ];
    memset( lng_sec, 0, sizeof( int ) * MAX_SECTION );

    while( pos < byte_lines ){

        // セクション分け
        pos_sec[ section ] = pos;
        while( !( lines[ pos ] == '<' && lines[ pos +1 ] == '>' ) && lines[ pos ] != '\n' && pos < byte_lines ) ++pos;
        lng_sec[ section ] = pos - pos_sec[ section ];

        // 最後の行で、かつ壊れている場合
        if( pos >= byte_lines ){
            set_broken( true );
            break;
        }

        // スレを2ch型に再構築して改行
        if( lines[ pos ] == '\n' ){

            // セクション数が MIN_SECTION より小さい時はスレが壊れている
            if( section >= MIN_SECTION ){

                // 透明あぼーんの判定
                char number_str[ 64 ];
                memset( number_str, 0, 64 );
                memcpy( number_str, lines + pos_sec[ 0 ], MIN( lng_sec[ 0 ], 64 -1 ) );
                int number_in = atoi( number_str );

                while( number_in > number ){
#ifdef _DEBUG
                    std::cout << "abone : number = "<< number << " : " << number_in << std::endl;
#endif
                    constexpr char broken_str[] = "あぼ〜ん<><>あぼ〜ん<> あぼ〜ん <>\n";
                    m_decoded_lines.append( broken_str );
                    ++number;
                }

                // 名前
                APPEND_SECTION( 1 );
                m_decoded_lines.append( "<>" );

                // メアド
                APPEND_SECTION( 2 );
                m_decoded_lines.append( "<>" );

                // 日付
                APPEND_SECTION( 3 );

                // ID
                constexpr int i = 6;
                if( lng_sec[ i ] ){

                    m_decoded_lines.append( " ID:" );

                    m_decoded_lines.append( lines + pos_sec[ i ], lng_sec[ i ] );
                }
                m_decoded_lines.append( "<>" );

                // 本文
                APPEND_SECTION( 4 );
                m_decoded_lines.append( "<>" );

                // タイトル
                APPEND_SECTION( 5 );

                m_decoded_lines.push_back( '\n' );
                ++number;
            }

            // 新しい行へ移動
            ++pos;
            section = 0;
            memset( lng_sec, 0, sizeof( int ) * MAX_SECTION );
        }

        // 次のセクションへ移動
        else{

            pos += 2;
            ++section;

            // 壊れている
            if( section >= MAX_SECTION ){

#ifdef _DEBUG
                std::cout << "NodeTreeJBBS::raw2dat : broken section = " << section-1 << std::endl;
#endif
                set_broken( true );

                // その行は飛ばす
                while( lines[ pos ] != '\n' && pos < byte_lines ) ++pos;
                ++pos;
                section = 0;
                memset( lng_sec, 0, sizeof( int ) * MAX_SECTION );
            }
        }
    }

    byte = m_decoded_lines.size();
#ifdef _DEBUG
    std::cout << "byte = " << byte << std::endl;
#endif

    return m_decoded_lines.c_str();
}
