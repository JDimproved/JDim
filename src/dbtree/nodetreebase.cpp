// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "nodetreebase.h"
#include "spchar_decoder.h"
#include "interface.h"

#include "jdlib/loaderdata.h"
#include "jdlib/misccharcode.h"
#include "jdlib/miscgtk.h"
#include "jdlib/miscmsg.h"
#include "jdlib/miscutil.h"

#include "dbimg/imginterface.h"

#include "message/logmanager.h"

#include "config/globalconf.h"

#include "cache.h"
#include "colorid.h"
#include "command.h"
#include "cssmanager.h"
#include "fontid.h"
#include "global.h"
#include "httpcode.h"
#include "replacestrmanager.h"
#include "session.h"
#include "urlreplacemanager.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <limits>
#include <sstream>


#ifndef MAX
#define MAX( a, b ) ( a > b ? a : b )
#endif


#ifndef MIN
#define MIN( a, b ) ( a < b ? a : b )
#endif


constexpr size_t SECTION_NUM = 5;
constexpr int LNG_ID = 256;
constexpr size_t LNG_LINK = 256;
constexpr size_t MAX_ANCINFO = 64;
constexpr int RANGE_REF = 20;
constexpr size_t MAX_RES_DIGIT = std::numeric_limits< int >::digits10 + 1;  // レスアンカーや発言数で使われるレスの桁数

constexpr std::size_t MAXSISE_OF_LINES = 256 * 1024;  // ロード時に１回の呼び出しで読み込まれる最大データサイズ
constexpr std::size_t SIZE_OF_HEAP = 512 * 1024;

constexpr size_t INITIAL_RES_BUFSIZE = 128;  // レスの文字列を返すときの初期バッファサイズ

constexpr std::size_t kMaxBytesOfUTF8Char = 8; // UTF-8文字の最大バイト数4 + ヌル文字


// レジュームのモード
enum
{
    RESUME_NO = 0,  // レジューム無し
    RESUME_MODE1,   // 通常のレジューム
    RESUME_MODE2,   // サーバが range を無視してデータを送ってきた
    RESUME_MODE3,   // サーバが range を無視してデータを送ってきた時にデータをスキップ中
    RESUME_FAILED   // レジューム失敗
};


#define IS_URL(node) \
 ( node->type == NODE_LINK && node->linkinfo->link \
  && ( memcmp( node->linkinfo->link, "http", 4 ) == 0 \
      || memcmp( node->linkinfo->link, "ftp", 3 ) == 0 ) )


using namespace DBTREE;


NodeTreeBase::NodeTreeBase( const std::string& url, const std::string& modified )
    : SKELETON::Loadable()
    , m_url( url )
    , m_resume( RESUME_NO )
    , m_heap( SIZE_OF_HEAP )
{
    set_date_modified( modified );

    // ルートヘッダ作成。中は空。
    m_id_header = -1; // ルートヘッダIDが 0 になるように -1
    NODE* tmpnode = create_node_header();
    assert( tmpnode );
    assert( m_vec_header.size() == static_cast< decltype( m_vec_header.size() ) >( m_id_header ) );
    m_vec_header.push_back( tmpnode );

    m_default_noname = DBTREE::default_noname( m_url );
    m_url_readcgi = url_readcgi( m_url, 0, 0 );

    // 参照で色を変える回数
    m_num_reference[ LINK_HIGH ] = CONFIG::get_num_reference_high();
    m_num_reference[ LINK_LOW ] = CONFIG::get_num_reference_low();

    // 発言数で色を変える回数
    m_num_id[ LINK_HIGH ] = CONFIG::get_num_id_high();
    m_num_id[ LINK_LOW ] = CONFIG::get_num_id_low();

    // レスにアスキーアートがあると判定する正規表現
    if( CONFIG::get_aafont_enabled() ){
        constexpr bool icase = false;
        constexpr bool newline = true;
        m_aa_regex.set( CONFIG::get_regex_res_aa(), icase, newline );
    }

#ifdef _DEBUG
    std::cout << "NodeTreeBase::NodeTreeBase url = " << m_url << " modified = " << get_date_modified()
              << " noname = " << m_default_noname << std::endl;
#endif
}


NodeTreeBase::~NodeTreeBase()
{
#ifdef _DEBUG    
    std::cout << "NodeTreeBase::~NodeTreeBase : " << m_url << std::endl;
#endif

    NodeTreeBase::clear();
}


//
// url の更新
//
// 移転があったときなどにarticlebaseから呼ばれる
//
void NodeTreeBase::update_url( const std::string& url )
{
    if( empty() ) return;

#ifdef _DEBUG
    std::string old_url = m_url;
#endif

    m_url = url;
    m_url_readcgi = url_readcgi( m_url, 0, 0 );

#ifdef _DEBUG
    if( ! old_url.empty() ) std::cout << "NodeTreeBase::update_url from "  << old_url
                                     << " to " << m_url << std::endl;
#endif
}


//
// バッファなどのクリア
//
void NodeTreeBase::clear()
{
#ifdef _DEBUG    
    std::cout << "NodeTreeBase::clear : " << m_url << std::endl;
#endif

    m_buffer_lines.clear();
    m_buffer_lines.shrink_to_fit();

    m_parsed_text.clear();
    m_parsed_text.shrink_to_fit();

    m_buffer_write.clear();
    m_buffer_write.shrink_to_fit();

    m_buf_text.clear();
    m_buf_link.clear();

    if( m_fout ) fclose( m_fout );
    m_fout = nullptr;

    m_ext_err = std::string();
}


//
// 総レス数
//
// ロード中は m_id_header 番のレスはまだ処理中なので m_id_header -1 を返す
//
int NodeTreeBase::get_res_number() const
{
    if( is_loading() ) return m_id_header -1;
    
    return m_id_header;
}


//
// number 番のレスのヘッダのポインタを返す
//
const NODE* NodeTreeBase::res_header( int number ) const
{
    if( number > m_id_header || number <= 0 ) return nullptr;
    
    return m_vec_header[ number ];
}


//
// 指定したID の重複数( = 発言数 )
//
// 下の get_num_id_name( int number ) と違って検索するので遅い
//
int NodeTreeBase::get_num_id_name( const std::string& id ) const
{
    if( id.empty() ) return 0;

    if( ! CONFIG::get_check_id() ){ // IDの数を数えていない場合は全て数える

        int count = 0;
        for( int i = 1; i <= m_id_header; ++i ){

            if( get_id_name( i ) == id ) count++;
        }
        return count;
    }

    // IDの数を数えている場合

    for( int i = 1; i <= m_id_header; ++i ){

        if( get_id_name( i ) == id ) return get_num_id_name( i );
    }

    return 0;
}


//
// number番の ID の重複数
//
int NodeTreeBase::get_num_id_name( const int number ) const
{
    const NODE* head = res_header( number );
    if( ! head ) return 0;

    // IDの数を数えていない場合
    if( ! CONFIG::get_check_id() ) return get_num_id_name( get_id_name( number ) );

    return head->headinfo->num_id_name;
}


//
// 指定した発言者IDを持つレス番号をリストにして取得
//
std::list< int > NodeTreeBase::get_res_id_name( const std::string& id_name ) const
{
    std::list< int > list_resnum;          
    for( int i = 1; i <= m_id_header ; ++i ){
        if( id_name == get_id_name( i )
            && ( ! m_abone_transparent || ! get_abone( i ) ) //  透明あぼーんしていない　or あぼーんしていない
            ) list_resnum.push_back( i );
    }

    return list_resnum;
}



//
// str_num で指定したレス番号をリストにして取得
// str_num は "from-to"　の形式 (例) 3から10をセットしたいなら "3-10"
// list_jointは出力で true のスレは前のスレに連結される (例) "3+4" なら 4が3に連結
//
std::list< int > NodeTreeBase::get_res_str_num( const std::string& str_num, std::list< bool >& list_joint ) const
{
#ifdef _DEBUG
    std::cout << "NodeTreeBase::get_res_str_num " << str_num << std::endl;
#endif

    std::list< int > list_resnum;

    // "," ごとにブロック分けする (例) "1-2,3-4+5" -> "1-2","3-4+5"
    std::list< std::string > list_str_num = MISC::StringTokenizer( str_num, ',' );
    for( const std::string& comma_block : list_str_num ) {

        // "=" ごとにブロック分けする (例) "1-2=3-4+5" -> "1-2","3-4+5"
        std::list<std::string> list_str_num_eq = MISC::StringTokenizer( comma_block, '=' );
        for( const std::string& eq_block : list_str_num_eq ) {

            // true なら前のスレと結合
            bool joint = false;

            // "+"ごとにブロックを分ける (例) "1+2-3+4" -> "1","2-3","4"
            std::list<std::string> list_str_num_pl = MISC::StringTokenizer( eq_block, '+' );
            for( const std::string& plus_block : list_str_num_pl ) {

                // num_from から num_to まで表示
                int num_from = MAX( 1, atol( plus_block.c_str() ) );

                if( num_from <= m_id_header  ){

                    int num_to = 0;
                    size_t i;
                    if( ( i = plus_block.find( '-' ) ) != std::string::npos ) num_to = atol( plus_block.substr( i +1 ).c_str() );
                    num_to = MIN( MAX( num_to, num_from ), m_id_header );

                    for( int i2 = num_from; i2 <= num_to ; ++i2 ) {

                        //  透明あぼーんしていない　or あぼーんしていないなら追加
                        if( ! m_abone_transparent || ! get_abone( i2 ) ){ 
#ifdef _DEBUG
                            std::cout << plus_block << " " << num_from << " - " << num_to
                                      << " i2 = " << i2 << " joint = " << joint << std::endl;
#endif
                            list_resnum.push_back( i2 );
                            list_joint.push_back( joint );

                            // "+"が付いていたら2つ目のブロックから連結指定
                            if( list_str_num_pl.size() >= 2 ) joint = true;
                        }
                    }
                }
            }
        }
    }

    return list_resnum;
}



//
// URL を含むレス番号をリストにして取得
//
std::list< int > NodeTreeBase::get_res_with_url() const
{
    std::list< int > list_resnum;
    for( int i = 1; i <= m_id_header; ++i ){

        const NODE* head = res_header( i );
        if( head ){

            for( int block = 0; block < BLOCK_NUM; ++block ){

                const NODE* node = head->headinfo->block[ block ];

                while( node ){

                    if( IS_URL( node )
                        && ( ! m_abone_transparent || ! get_abone( i ) ) //  透明あぼーんしていない　or あぼーんしていない
                        ){

                        list_resnum.push_back( i );
                        block = BLOCK_NUM;
                        break;
                    }

                    node = node->next_node;
                }
            }
        }
    }
    
    return list_resnum;
}


//
// ツリーに含まれてる 画像URL をリストにして取得
//
std::list<std::string> NodeTreeBase::get_imglinks() const
{
    std::list<std::string> list_urls;
    for( int i = 1; i <= m_id_header; ++i ){

        const NODE* head = res_header( i );
        if( head ){

            for( int block = 0; block < BLOCK_NUM; ++block ){

                const NODE* node = head->headinfo->block[ block ];

                while( node ){
                    if( IS_URL( node ) && node->linkinfo->imglink ) {
                        list_urls.emplace_back( node->linkinfo->imglink );
                    }
                    node = node->next_node;
                }
            }
        }
    }
    
    return list_urls;
}



//
// number番のレスを参照しているレス番号をリストにして取得
//
std::list< int > NodeTreeBase::get_res_reference( const int number ) const
{
    std::list< int > res_num;
    res_num.push_back( number );
    return get_res_reference( res_num );
}


//
// res_num に含まれるレスを参照しているレス番号をリストにして取得
//
std::list< int > NodeTreeBase::get_res_reference( const std::list< int >& res_num ) const
{
    std::list< int > list_resnum;

    if( ! res_num.size() ) return list_resnum;

    for( int i = 1; i <= m_id_header; ++i ){

        //  透明あぼーんは除外
        if( m_abone_transparent && get_abone( i ) ) continue;

        const NODE* head = res_header( i );
        if( head ){

            for( int block = 0; block < BLOCK_NUM; ++block ){

                const NODE* node = head->headinfo->block[ block ];

                while( node ){

                    if( node->type == NODE_LINK ){

                        // アンカーノードの時は node->linkinfo->ancinfo != nullptr;
                        if( node->linkinfo->ancinfo ){

                            int anc = 0;
                            int anc_from;
                            int anc_to;
                            for(;;){

                                anc_from = node->linkinfo->ancinfo[ anc ].anc_from;
                                anc_to = node->linkinfo->ancinfo[ anc ].anc_to;
                                if( anc_from == 0 ) break;
                                ++anc;

                                for( const int number : res_num ) {

                                    if( i != number
                                        && anc_to - anc_from < RANGE_REF // >>1-1000 みたいなアンカーは弾く
                                        && anc_from <= number && number <= anc_to
                                        ) {
                                        list_resnum.push_back( i );
                                        goto EXIT_LOOP;
                                    }
                                }
                            }
                        }
                    }

                    node = node->next_node;

                } // while( node )

            } // for( block )

        } // if( head )

EXIT_LOOP:;

    } // for( int i )

#ifdef _DEBUG
    std::cout << "NodeTreeBase::get_reference\n";
    for( const int resnum : list_resnum ) std::cout << resnum << std::endl;
#endif
    
    return list_resnum;
}


//
// 高参照レスの番号をリストにして取得
//
std::list< int > NodeTreeBase::get_highly_referened_res() const
{
    std::list< int > list_resnum;
    for( int i = 1; i <= m_id_header; ++i ){

        const NODE* head = res_header( i );
        if ( ! head ) continue;
        if ( get_abone( i ) ) continue; // あぼーんしているものは飛ばす
        if ( head->headinfo->num_reference < m_num_reference[ LINK_HIGH ] ) continue;

        // リストに追加
        list_resnum.push_back( i );
    }

    return list_resnum;
}



//
// number番のレスに含まれるレスアンカーをリストにして取得
//
std::list< ANCINFO* > NodeTreeBase::get_res_anchors( const int number )
{
    std::list< ANCINFO* > list_resnum;

    NODE* head = res_header( number );
    if( head && head->headinfo
            && ! head->headinfo->abone ){

        for( int block = 0; block < BLOCK_NUM; ++block ){

            NODE* node = head->headinfo->block[ block ];
            while( node ){

                // アンカーノードの時は node->linkinfo->ancinfo != nullptr;
                if( node->type == NODE_LINK 
                        && node->linkinfo->ancinfo ){

                    for(int anc = 0; ; ++anc){

                        ANCINFO* anchor = &( node->linkinfo->ancinfo[ anc ] );
                        if( anchor->anc_from == 0 ) break;

                        // >>1-1000 みたいなアンカーは弾く
                        if( anchor->anc_to - anchor->anc_from < RANGE_REF ){
                            list_resnum.push_back( anchor );
                        }
                    }
                }
                node = node->next_node;

            } // while( node )

        } // for( block )

    } // if( head )
    return list_resnum;
}


//
// query を含むレス番号をリストにして取得
//
// mode_or == true なら OR抽出
//
std::list< int > NodeTreeBase::get_res_query( const std::string& query, const bool mode_or ) const
{
    std::list< int > list_resnum;
    if( query.empty() ) return list_resnum;

    std::list<JDLIB::RegexPattern> list_regex;
    JDLIB::Regex regex;
    const auto make_pattern = []( const std::string& qry ) {
        constexpr bool icase = true; // 大文字小文字区別しない
        constexpr bool newline = true; // . に改行をマッチさせない
        constexpr bool usemigemo = true; // migemo使用
        constexpr bool wchar = true; // 全角半角の区別をしない

        return JDLIB::RegexPattern( qry, icase, newline, usemigemo, wchar );
    };

    const std::list<std::string> list_query = MISC::split_line( query );
    std::transform( list_query.cbegin(), list_query.cend(), std::back_inserter( list_regex ), make_pattern );

    for( int i = 1; i <= m_id_header ; ++i ){

        const std::string res_str = get_res_str( i );

        bool apnd = true;
        if( mode_or ) apnd = false;

        for( const JDLIB::RegexPattern& pattern : list_regex ) {

            constexpr std::size_t offset = 0;
            const bool ret = regex.match( pattern, res_str, offset );

            // OR
            if( mode_or ){
                
                if( ret ){
                    apnd = true;
                    break;
                }
            }

            // AND
            else{

                if( ! ret ){
                    apnd = false;
                    break;
                }
            }
        }

        if( apnd
            && ( ! m_abone_transparent || ! get_abone( i ) ) //  透明あぼーんしていない　or あぼーんしていない
            ) list_resnum.push_back( i );
    }

    return list_resnum;
}



//
// number　番のレスの文字列を返す
//
// ref == true なら先頭に参照文字( "> "など)を付ける
//

#define GETNODESTR( id )  do{ \
node = head->headinfo->block[ id ]; \
while( node ){ \
if( node->type == DBTREE::NODE_BR ) str_res += "\n" + ref_prefix; \
else if( node->type == DBTREE::NODE_SP ) str_res.push_back( ' ' ); \
else if( node->type == DBTREE::NODE_HTAB ) str_res += "\t"; \
else if( node->text ) str_res += node->text; \
node = node->next_node; \
} }while(0) \

std::string NodeTreeBase::get_res_str( int number, bool ref ) const
{
    std::string str_res;

#ifdef _DEBUG
    std::cout << "NodeTreeBase::get_res_str : num = " << number << std::endl;
#endif

    const NODE* head = res_header( number );
    if( ! head ) return std::string();

    std::string ref_prefix;
    if( ref ) ref_prefix = CONFIG::get_ref_prefix();

    str_res.reserve( INITIAL_RES_BUFSIZE );
    str_res += ref_prefix;

    NODE* node;
    GETNODESTR( BLOCK_NUMBER ); str_res += " ";
    GETNODESTR( BLOCK_NAMELINK ); str_res += "：";
    GETNODESTR( BLOCK_NAME ); str_res += " ";
    GETNODESTR( BLOCK_MAIL ); str_res += "： ";
    GETNODESTR( BLOCK_DATE ); str_res += " ";
    GETNODESTR( BLOCK_ID_NAME ); str_res += " ";

    str_res += "\n" + ref_prefix;
    GETNODESTR( BLOCK_MES );
    str_res += "\n";

#ifdef _DEBUG
    std::cout << str_res << std::endl;
#endif
    
    return str_res;
}



//
// number番を書いた人の名前を取得
//
std::string NodeTreeBase::get_name( int number ) const
{
    const NODE* head = res_header( number );
    if( ! head ) return std::string();
    if( ! head->headinfo->name ) return std::string();

    return head->headinfo->name;
}


//
// number番の名前の重複数( = 発言数 )
//
int NodeTreeBase::get_num_name( int number ) const
{
    int num = 0;
    std::string name = get_name( number );

    for( int i = 1; i <= m_id_header; ++i ){

        if( get_name( i ) == name ) ++num;
    }

    return num;
}



//
// 指定した発言者の名前のレス番号をリストにして取得
//
std::list< int > NodeTreeBase::get_res_name( const std::string& name ) const
{
    std::list< int > list_resnum;          
    for( int i = 1; i <= m_id_header ; ++i ){
        if( name == get_name( i )
            && ( ! m_abone_transparent || ! get_abone( i ) ) //  透明あぼーんしていない　or あぼーんしていない
            ) list_resnum.push_back( i );
    }

    return list_resnum;
}


//
// number番のレスの時刻を文字列で取得
// 内部で regex　を使っているので遅い
//
std::string NodeTreeBase::get_time_str( int number ) const
{
    std::string res_str = get_res_str( number );
    if( res_str.empty() ) return std::string();

    std::string time_str;
    JDLIB::Regex regex;
    const size_t offset = 0;
    const bool icase = false;
    const bool newline = true;
    const bool usemigemo = false;
    const bool wchar = false;

    if( regex.exec( " 名前：.+]： +([0-9]*/[0-9]*/[0-9]*[^ ]* [0-9]*:[0-9]*[^ ]*).*$", res_str, offset, icase, newline, usemigemo, wchar ) ){
        time_str = regex.str( 1 );
    }

    return time_str;
}


//
// number番の ID 取得
//
std::string NodeTreeBase::get_id_name( int number ) const
{
    const NODE* head = res_header( number );
    if( ! head || ! head->headinfo->block[ BLOCK_ID_NAME ] ) return std::string();

    const NODE* idnode = head->headinfo->block[ BLOCK_ID_NAME ]->next_node;
    while( idnode && ( ! idnode->linkinfo || ! idnode->linkinfo->link
                        || idnode->linkinfo->link[ 0 ] != 'I' ) ){
        idnode = idnode->next_node;
    }

    if( idnode ) return idnode->linkinfo->link;

    return std::string();
}



//
// 基本ノード作成
//
NODE* NodeTreeBase::create_node()
{
    NODE* tmpnode = m_heap.heap_alloc<NODE>();

    tmpnode->id_header = m_id_header;
    tmpnode->fontid = FONT_EMPTY; // フォントID未設定
    if( m_node_previous ) m_node_previous->next_node = tmpnode;
    m_node_previous = tmpnode;
    
    return tmpnode;
}


//
// ヘッダノード作成
//
// 要素数が(CONFIG::get_max_resnumber())より多くなる場合nullptrを返す
NODE* NodeTreeBase::create_node_header()
{
    if( m_id_header >= CONFIG::get_max_resnumber() ) {
        return nullptr;
    }

    ++m_id_header;
    m_node_previous = nullptr;

    NODE* tmpnode = create_node();
    tmpnode->type =  NODE_HEADER;

    // ヘッダ情報
    tmpnode->headinfo = m_heap.heap_alloc<HEADERINFO>();
    if( m_id_header >= 2 ) m_vec_header[ m_id_header -1 ]->headinfo->next_header = tmpnode;
    
    return tmpnode;
}


//
// block ノード作成
//
// 名前や本文などのブロックの先頭に置く
//
NODE* NodeTreeBase::create_node_block()
{
    m_node_previous = nullptr;

    NODE* tmpnode = create_node();
    tmpnode->type =  NODE_BLOCK;
    
    return tmpnode;
}


/**
 * @brief 発言回数(IDの出現数と何番目の投稿)ノード
 */
NODE* NodeTreeBase::create_node_idnum()
{
    // (何番目の投稿/発言数) の形式で表示するためメモリを確保する
    NODE* tmpnode = create_node_text( DBTREE::kPlaceholderForNodeIdNum, COLOR_CHAR );
    tmpnode->type = NODE_IDNUM;
    tmpnode->text[ 0 ] = '\0'; // メモリだけ確保して文字を消す
    return tmpnode;
}


//
// 改行ノード作成
//
NODE* NodeTreeBase::create_node_br()
{
    // 改行前の空白を取り除く
    NODE* tmpnode = ( m_node_previous->type == NODE_SP ) ? m_node_previous : create_node();
    tmpnode->type = NODE_BR;
    return tmpnode;
}


//
// 水平線ノード作成
//
NODE* NodeTreeBase::create_node_hr()
{
    NODE* tmpnode = create_node();
    tmpnode->type = NODE_HR;
    return tmpnode;
}


//
// スペースノード
//
NODE* NodeTreeBase::create_node_space( const int type, const int bg )
{
    NODE* tmpnode;

    if( type != NODE_SP || ( m_node_previous->type != type
                             && m_node_previous->type != NODE_MULTISP
                             && m_node_previous->type != NODE_HTAB ) ) {
        tmpnode = create_node();
        tmpnode->type = type;
        tmpnode->color_text = COLOR_CHAR;
        tmpnode->color_back = bg;
        tmpnode->bold = false;
    }
    else {
        tmpnode = m_node_previous;
    }
    return tmpnode;
}


//
// 連続半角スペース
//
NODE* NodeTreeBase::create_node_multispace( std::string_view text, const int bg, const char fontid )
{
    NODE* tmpnode = create_node_ntext( text.data(), text.size(), COLOR_CHAR, bg, false, fontid );
    tmpnode->type = NODE_MULTISP;
    return tmpnode;
}


//
// リンクノード作成
//
// bold : 太字か
//
NODE* NodeTreeBase::create_node_link( std::string_view text, std::string_view link,
                                      const int color_text, const int color_back, const bool bold, const char fontid )
{
    NODE* tmpnode = create_node_ntext( text.data(), text.size(), color_text, color_back, bold, fontid );

    if( tmpnode ){
        tmpnode->type = NODE_LINK;

        // リンク情報作成
        char *tmplink = m_heap.heap_alloc<char>( link.size() + 1 );
        link.copy( tmplink, link.size() );
        tmplink[ link.size() ] = '\0';

        // リンク情報セット
        tmpnode->linkinfo = m_heap.heap_alloc<LINKINFO>();
        tmpnode->linkinfo->link = tmplink;
    }
    
    return tmpnode;
}


//
// アンカーノード作成
//
NODE* NodeTreeBase::create_node_anc( std::string_view text, std::string_view link,
                                     const int color_text,  const bool bold,
                                     const ANCINFO* ancinfo, const int lng_ancinfo, const char fontid )
{
    NODE* tmpnode = create_node_link( text, link, color_text, COLOR_NONE, bold, fontid );
    if( tmpnode ){

        tmpnode->linkinfo->ancinfo = m_heap.heap_alloc<ANCINFO>( lng_ancinfo + 1 );
        memcpy( tmpnode->linkinfo->ancinfo, ancinfo, sizeof( ANCINFO ) * lng_ancinfo );
    }
    
    return tmpnode;
}


//
// SSSPノード
//
NODE* NodeTreeBase::create_node_sssp( std::string_view link )
{
    NODE* tmpnode = create_node();
    tmpnode->type = NODE_SSSP;

    // リンク情報作成
    char *tmplink = m_heap.heap_alloc<char>( link.size() + 1 );
    link.copy( tmplink, link.size() );
    tmplink[ link.size() ] = '\0';

    // リンク情報セット
    tmpnode->linkinfo = m_heap.heap_alloc<LINKINFO>();
    tmpnode->linkinfo->link = tmplink;
    tmpnode->linkinfo->image = true;
    tmpnode->linkinfo->imglink = tmpnode->linkinfo->link;

    return tmpnode;
}


//
// 画像ノード作成
//
NODE* NodeTreeBase::create_node_img( std::string_view text, std::string_view link, const int color_text,
                                     const bool bold, const char fontid )
{
    NODE* tmpnode = create_node_link( text, link, color_text, COLOR_NONE, bold, fontid );
    if( tmpnode ){
        tmpnode->linkinfo->image = true;
        tmpnode->linkinfo->imglink = tmpnode->linkinfo->link;
    }

    return tmpnode;
}


//
// サムネイル画像ノード ( youtubeなどのサムネイル表示用 )
//
NODE* NodeTreeBase::create_node_thumbnail( std::string_view text, std::string_view link,
                                           std::string_view thumb, const int color_text, const bool bold,
                                           const char fontid )
{
    NODE* tmpnode = create_node_link( text, link, color_text, COLOR_NONE, bold, fontid );

    if( tmpnode ){
        // サムネイル画像のURLをセット
        char *tmpthumb = m_heap.heap_alloc<char>( thumb.size() + 1 );
        thumb.copy( tmpthumb, thumb.size() );
        tmpthumb[ thumb.size() ] = '\0';

        tmpnode->linkinfo->imglink = tmpthumb;
    }

    return tmpnode;
}



//
// テキストノード作成
//
NODE* NodeTreeBase::create_node_text( std::string_view text, const int color_text, const bool bold, const char fontid )
{
    return create_node_ntext( text.data(), text.size(), color_text, COLOR_NONE, bold, fontid );
}



//
// テキストノード作成( サイズ指定 )
//
NODE* NodeTreeBase::create_node_ntext( const char* text, const int n, const int color_text, const int color_back,
                                       const bool bold, const char fontid )
{
    if( n <= 0 ) return nullptr;
    
    NODE* tmpnode = create_node();

    if( tmpnode ){
        tmpnode->type = NODE_TEXT;

        tmpnode->text = m_heap.heap_alloc<char>( n + MAX_RES_DIGIT + 4 );
        memcpy( tmpnode->text, text, n ); tmpnode->text[ n ] = '\0';
        tmpnode->color_text = color_text;
        tmpnode->color_back = color_back;
        tmpnode->bold = bold;
        if ( fontid != FONT_MAIN ) tmpnode->fontid = fontid;
    }

    return tmpnode;
}



//
// html をコメントとして追加
//
// パースして追加したノードのポインタを返す
//
NODE* NodeTreeBase::append_html( const std::string& html )
{
    if( is_loading() ) return nullptr;
    if( html.empty() ) return nullptr;

#ifdef _DEBUG
    std::cout << "NodeTreeBase::append_html url = " << m_url << " html = " << html << std::endl;
#endif

    NODE* header = create_node_header();
    if( !header ) {
        return nullptr;
    }
    assert( m_vec_header.size() == static_cast< decltype( m_vec_header.size() ) >( m_id_header ) );
    m_vec_header.push_back( header );

    init_loading();
    header->headinfo->block[ BLOCK_MES ] = create_node_block();

    const bool digitlink = false;
    const bool bold = false;
    parse_html( html.data(), html.size(), COLOR_CHAR, digitlink, bold );

    clear();

    return header;
}



//
// dat を追加
//
// パースして追加したノードのポインタを返す
//
NODE* NodeTreeBase::append_dat( const std::string& dat )
{
    if( is_loading() ) return nullptr;
    if( dat.empty() ) return nullptr;

    init_loading();
    receive_data( dat );
    receive_finish();

    return res_header( m_id_header );
}


//
// キャッシュからスレッドをロード
//
void NodeTreeBase::load_cache()
{
    std::string path_cache = CACHE::path_dat( m_url );
    if( CACHE::file_exists( path_cache ) == CACHE::EXIST_FILE ){

#ifdef _DEBUG
        std::cout << "NodeTreeBase::load_cache from " << path_cache << std::endl;
#endif
        std::string str;
        if( CACHE::load_rawdata( path_cache, str ) ){

            m_check_update = false;
            m_check_write = false;
            m_loading_newthread = false;
            set_resume( false );
            init_loading();

            receive_data( str );
            receive_finish();

            // レジューム時のチェックデータをキャッシュ
            set_resume_data( str.c_str(), str.size() );
        }
    }
}


//
// レジューム時のチェックデータをキャッシュ
//
void NodeTreeBase::set_resume_data( const char* data, size_t length )
{
    // キャッシュされていない場合だけキャッシュする
    if( ! m_resume_cached ){
        m_resume_cached = true;

        // レジューム時のチェック用に生データの先頭から RESUME_CHKSIZE バイト分をコピーしておく
        // 詳しくは NodeTreeBase::receive_data() を参照せよ
        const size_t length_chk = MIN( (RESUME_CHKSIZE - 1), length );
        memcpy( m_resume_head, data, length_chk );
        m_resume_head[ length_chk ] = '\0';
    }
}


//
// ロード実行前に呼ぶ初期化関数
//
void NodeTreeBase::init_loading()
{
    m_buffer_lines.clear();

    // 一時バッファ作成
    if( m_buffer_lines.capacity() < MAXSISE_OF_LINES ) {
        m_buffer_lines.reserve( MAXSISE_OF_LINES );
    }
    if( m_parsed_text.capacity() < MAXSISE_OF_LINES ) {
        m_parsed_text.reserve( MAXSISE_OF_LINES );
    }
    if( m_buf_text.capacity() < LNG_LINK ) {
        m_buf_text.reserve( LNG_LINK +16 );
    }
    if( m_buf_link.capacity() < LNG_LINK ) {
        m_buf_link.reserve( LNG_LINK +16 );
    }
}


//
// レジュームのモードをセットする
//
void NodeTreeBase::set_resume( const bool resume )
{
#ifdef _DEBUG
    std::cout << "NodeTreeBase::set_resume resume = " << resume << std::endl;
#endif

    if( resume ) m_resume = RESUME_MODE1;
    else m_resume = RESUME_NO;
}


//
// ロード開始
//
// check_update : HEADによる更新チェックのみ
//
void NodeTreeBase::download_dat( const bool check_update )
{
    if( is_loading() ) return;

    m_check_update = check_update;
    m_check_write = ! m_check_update;
    m_loading_newthread = ( ! get_res_number() );

#ifdef _DEBUG    
    std::cout << "NodeTreeBase::download_dat : " << m_url << " lng = " << m_lng_dat << std::endl
              << "modified = " << get_date_modified() << " check_update = " << check_update
              << " newthread = " << m_loading_newthread << std::endl;
#endif


    // オフライン
    if( ! SESSION::is_online() ){

        set_str_code( "" );

        // ディスパッチャ経由でreceive_finish()を呼ぶ
        finish();

        return;
    }

    //
    // オンライン
    //

    init_loading();

    if( ! m_check_update ){

        // 保存ディレクトリ作成(無ければ)
        if( CACHE::mkdir_boardroot( m_url ) ){
    
            // 保存ファイルオープン
            std::string path_cache = CACHE::path_dat( m_url );

#ifdef _DEBUG
            std::cout << "open " << path_cache.c_str() << std::endl;
#endif

            m_fout = fopen( to_locale_cstr( path_cache ), "ab" );
            if( m_fout == nullptr ){
                MISC::ERRMSG( "fopen failed : " + path_cache );
            }
        }
        else{
            MISC::ERRMSG( "could not create " + DBTREE::url_boardbase( m_url ) );
        }
    }

    // ロード開始
    // ロード完了したら receive_finish() が呼ばれる
    JDLIB::LOADERDATA data;
    create_loaderdata( data );

    // 更新チェックの時はHEADを使う
    if( m_check_update ){
        data.head = true;
        data.timeout = CONFIG::get_loader_timeout_checkupdate();
    }

    if( data.url.empty() || ! start_load( data ) ){
        m_sig_finished.emit();
        clear();
    }
}


//
// ローダからデータ受け取り
//
void NodeTreeBase::receive_data( std::string_view buf )
{
    if( is_loading()
        && ( get_code() != HTTP_OK && get_code() != HTTP_PARTIAL_CONTENT ) ){

#ifdef _DEBUG
        std::cout << "NodeTreeBase::receive_data : code = " << get_code() << std::endl;
        std::cout << buf << std::endl;
#endif
        return;
    }

    if( m_check_update ) return;

    // 通常のレジューム処理
    if( m_resume == RESUME_MODE1 ){

#ifdef _DEBUG
        std::cout << "resume mode = " << m_resume << " -> ";
#endif

        // レジュームした時に先頭が '\n' ならレジューム成功
        if( ! buf.empty() && buf[0] == '\n' ){

            buf.remove_prefix( 1 );
            m_resume = RESUME_NO;
        }

        // サーバが range を無視してデータを送ってきた
        // 続きは add_raw_lines() を参照
        else if( get_code() == HTTP_OK ) m_resume = RESUME_MODE2;

        // あぼーんが起きた?
        else{

            m_broken = true;
            MISC::ERRMSG( "failed to resume" );
            m_resume = RESUME_NO;
        }

#ifdef _DEBUG
        std::cout << m_resume << std::endl;
#endif
    }

    if( m_resume == RESUME_FAILED ) return;

    if( buf.empty() ) return;

    while( ! buf.empty() ) {
        // BOF防止
        std::size_t size_in = (std::min)( MAXSISE_OF_LINES - m_buffer_lines.size() - 1, buf.size() );

        // バッファが '\n' で終わるように調整
        const auto pos = buf.rfind( '\n', size_in );
        if( pos != std::string_view::npos ) size_in = pos + 1; // '\n' を加える

        // '\n' が無かった場合バッファサイズの半分まではDATの処理を中断して改行が来るのを待ってみる
        else if( ( m_buffer_lines.size() + buf.size() ) < ( MAXSISE_OF_LINES / 2 ) ) size_in = 0;

        if( size_in == 0 ) break;

        // 前回の残りのデータに新しいデータを付け足して add_raw_lines() にデータを送る
        m_buffer_lines.append( buf.substr( 0, size_in ) );
        add_raw_lines( m_buffer_lines );
        m_buffer_lines.clear();

        buf.remove_prefix( size_in );

        // add_raw_lines() でレジュームに失敗したと判断したら、バッファをクリアする
        if( m_resume == RESUME_FAILED ) {
            m_buffer_lines.clear();
            return;
        }
    }

    // 残りのデータをバッファにコピーしておく
    if( ! buf.empty() ) m_buffer_lines.append( buf );
}


/**
 * @brief 保留された受信データをDATに変換する
 */
void NodeTreeBase::sweep_buffer()
{
    // 特殊スレのdatには、最後の行に'\n'がない場合がある
    if( ! m_buffer_lines.empty() ) {
        // 正常に読込完了した場合で、バッファが残っていれば add_raw_lines()にデータを送る
        add_raw_lines( m_buffer_lines );
        // バッファをクリア
        m_buffer_lines.clear();
    }
}


//
// ロード完了
//
void NodeTreeBase::receive_finish()
{
    bool is_error = false;
    if( get_code() != HTTP_INIT
        && get_code() != HTTP_OK
        && get_code() != HTTP_PARTIAL_CONTENT
        && get_code() != HTTP_NOT_MODIFIED
        && get_code() != HTTP_RANGE_ERR
        && get_code() != HTTP_OLD
        ){
        is_error = true;

        std::ostringstream err;
        err << m_url << std::endl
            << "load failed. : " << get_str_code();
        if( get_code() == HTTP_MOVED_PERM || get_code() == HTTP_REDIRECT ) err << " location = " << location();
        MISC::ERRMSG( err.str() );
    }

    // HTTP HEAD は本文を含まないが、あった場合は無視しないといけない
    if( ! m_check_update ){

        if( ! is_error ) sweep_buffer();
        else m_buffer_lines.clear();

        // Requested Range Not Satisfiable
        if( get_code() == HTTP_RANGE_ERR ){
            // ネットワーク設定の変更などで読み込みが再開できる可能性があるためスレッドが壊れたとマークしない
            MISC::ERRMSG( "Requested Range Not Satisfiable" );
        }

        // データがロードされなかったらキャッシュを消す
        if( get_res_number() == 0 ){

            std::string path = CACHE::path_dat( m_url );
            if( CACHE::file_exists( path ) == CACHE::EXIST_FILE ) unlink( to_locale_cstr( path ) );
            set_date_modified( std::string() );
        }

        // その他、何かエラーがあったらmodifiedをクリアしておく
        if( !get_ext_err().empty() ) set_date_modified( std::string() );

        // 書き込みチェック終了
        if( m_check_write && MESSAGE::get_log_manager()->size()
            && ( get_code() == HTTP_OK
                 || get_code() == HTTP_PARTIAL_CONTENT
                 || get_code() == HTTP_NOT_MODIFIED
                )
            ) MESSAGE::get_log_manager()->remove_items( m_url );
    }

#ifdef _DEBUG
    std::cout << "NodeTreeBase::receive_finish lng = " << m_lng_dat
              << " raw lng = " << current_length()
              << " code = " << get_code() << " " << get_str_code()
              << " modified = " << get_date_modified() << std::endl;
#endif    
    
    // 親 article クラスにシグナルを打ってツリー構造が変わったことを教える
    m_sig_finished.emit();

    clear();

    if( ! m_check_update
        && ( get_code() == HTTP_OK || get_code() == HTTP_PARTIAL_CONTENT )
        && ! get_date_modified().empty()
    ) {
        CACHE::set_filemtime( CACHE::path_dat( m_url ), get_time_modified() );
        // クッキーのセット
        DBTREE::board_set_list_cookies( m_url, SKELETON::Loadable::cookies() );
    }


    m_check_update = false;
    m_check_write = false;
    m_loading_newthread = false;
}


//
// 鯖から生の(複数)行のデータを受け取ってdat形式に変換して add_one_dat_line() に出力
//
void NodeTreeBase::add_raw_lines( std::string& buffer_lines )
{
    // 時々サーバ側のdatファイルが壊れていてデータ中に \0 が
    // 入っている時があるので取り除く
    if( const auto i = buffer_lines.find( '\0' ); i != std::string::npos ) {
        MISC::ERRMSG( std::to_string( i ) + " EOF was inserted in the middle of the raw data" );
        std::replace( buffer_lines.begin() + i, buffer_lines.end(), '\0', ' ' );
    }

    // 保存前にrawデータを加工
    char* rawlines = process_raw_lines( buffer_lines );

    size_t lng = strlen( rawlines );
    if( ! lng ) return;

    // サーバが range を無視してデータを送ってきたときのレジューム処理
    if( m_resume == RESUME_MODE2 ){

#ifdef _DEBUG
        std::cout << "NodeTreeBase::add_raw_lines : resume\n";
#endif

        // 先頭からdatを送ってきたかチェック
        const size_t length_chk = MIN( lng, MIN( (RESUME_CHKSIZE - 1), strlen( m_resume_head ) ) );
        if( strncmp( rawlines, m_resume_head, length_chk ) == 0 ){
            m_resume = RESUME_MODE3;
            m_resume_lng = 0;
        }

        // 全く違うデータを送ってきた
        else{

            m_broken = true;
            MISC::ERRMSG( "failed to resume" );
            m_resume = RESUME_FAILED;
            return;
        }
    }

    // レジューム処理でデータをスキップ中
    if( m_resume == RESUME_MODE3 ){

#ifdef _DEBUG
        std::cout << "NodeTreeBase::add_raw_lines : resume skip resume_lng = " << m_resume_lng
                  << " lng = " << lng << " / lng_dat = " << m_lng_dat << std::endl;
#endif

        m_resume_lng += lng;
        if( m_resume_lng <= m_lng_dat ) return;

        // 越えた分をカットしてレジューム処理終了
        rawlines += ( lng  - ( m_resume_lng - m_lng_dat ) );
        lng = ( m_resume_lng - m_lng_dat );
        m_resume = RESUME_NO;

#ifdef _DEBUG
        std::cout << "resume finished : lng = " << lng << std::endl;
#endif
    }

    if( ! lng ) return;

    m_lng_dat += lng;

    // キャッシュに保存
    if( m_fout ){
#ifdef _DEBUG
        std::cout << "NodeTreeBase::add_raw_lines save " << lng << " bytes\n";
#endif       
        if( fwrite( rawlines, 1, lng, m_fout ) < lng ){
            MISC::ERRMSG( "write failed in NodeTreeBase::add_raw_lines\n" );
        }
        // レジューム時のチェックデータをキャッシュ
        set_resume_data( rawlines, lng );
    }

    // dat形式に変換
    int byte;
    const char* datlines = raw2dat( rawlines, byte );
    if( !byte ) return;

    // '\n' 単位で区切って add_one_dat_line() に渡す
    int num_before = m_id_header;
    const char* pos = datlines;

    while( ( pos = add_one_dat_line( pos ) ) && *pos != '\0' ) ++pos;

    if( num_before != m_id_header ){

        // あぼーん判定
        update_abone( num_before +1,  m_id_header );

        // 発言数更新
        update_id_name( num_before +1,  m_id_header );

        // 参照数更新
        update_reference( num_before +1,  m_id_header );

        // フォント判定
        update_fontid( num_before +1,  m_id_header );

        // articlebase クラスに状態が変わったことを知らせる
        m_sig_updated.emit();
    }
}




//
// dat 解析関数
//
// datの1行を解析してノード木を作成する
//
// 戻り値: バッファの最後の位置
//
const char* NodeTreeBase::add_one_dat_line( const char* datline )
{
    const char* pos = datline;
    if( *pos == '\0' || *pos == '\n' ) return datline;

    size_t i;
    NODE* header = create_node_header();
    NODE *node;
    if( !header ) {
        return nullptr;
    }
    assert( m_vec_header.size() == static_cast< decltype( m_vec_header.size() ) >( m_id_header ) );
    m_vec_header.push_back( header );

    // レス番号
    const std::string tmpstr = std::to_string( header->id_header );
    const std::string tmplink = PROTO_RES + tmpstr;

    node = header->headinfo->block[ BLOCK_NUMBER ] = create_node_block();
    node->fontid = FONT_MAIL;
    create_node_link( tmpstr, tmplink, COLOR_CHAR_LINK_RES, COLOR_NONE, true, FONT_MAIL );

    std::string_view section[SECTION_NUM]{};

    // セクション分けしながら壊れてないかチェック
    for( i = 0; i < SECTION_NUM; ++i ) {

        const char* start = pos;
        while( *pos != '\0' && *pos != '\n' && ! ( pos[0] == '<' && pos[1] == '>' ) ) ++pos;
        section[i] = std::string_view( start, pos - start );

        if( *pos == '\0' || *pos == '\n' ) {
            ++i;
            break;
        }
        pos += 2; // "<>"の分
    }

    if( i < ( SECTION_NUM - 1 ) ) {
        // 本文途中まで解析出来てなければ旧dat形式でチェックしてみる
        pos = datline;

        for( i = 0; i < SECTION_NUM; ++i ) {

            const char* start = pos;
            while( *pos != ',' && *pos != '\0' && *pos != '\n' ) ++pos;
            section[i] = std::string_view( start, pos - start );

            if( *pos == '\0' || *pos == '\n' ) {
                ++i;
                break;
            }
            pos += 1; // ","の分
        }
    }

    // 行末まで読み飛ばす
    while( *pos != '\0' && *pos != '\n' ) ++pos;

    // 名前
    const int color_name = section[1].size() ? COLOR_CHAR_NAME : COLOR_CHAR_NAME_NOMAIL;
    parse_name( header, section[0], color_name );

    // メール
    if( i > 1 ) parse_mail( header, section[1] );

    // 日付とID
    if( i > 2 ) parse_date_id( header, section[2] );

    // 本文
    if( i > 3 ) {
        // EXTDAT情報を取得
        if( header->id_header == 1 ) parse_extattr( section[3] );

        header->headinfo->block[ BLOCK_MES ] = create_node_block();

        std::string_view str = section[3];
        std::string str_msg;

        // 文字列置換
        const CORE::ReplaceStr_Manager* const mgr = CORE::get_replacestr_manager();
        if( mgr->list_get_active( CORE::REPLACETARGET_MESSAGE ) ) {
            str_msg = mgr->replace( str, CORE::REPLACETARGET_MESSAGE );
            str = str_msg;
        }

        constexpr bool digitlink = false;
        constexpr bool bold = false;
        parse_html( str.data(), str.size(), COLOR_CHAR, digitlink, bold );
    }

    // 壊れている
    if( i != SECTION_NUM ){

#ifdef _DEBUG
        std::cout << header->id_header << " is broken section = " << i << std::endl;
        std::cout << datline << std::endl;
#endif

        m_broken = true;
        if( i <= 2 ) {
            node = header->headinfo->block[ BLOCK_MES ] = create_node_block();
            node->fontid = FONT_MAIL;
        }
        constexpr bool digitlink = false;
        constexpr bool bold = true;
        constexpr std::string_view message = "<br> <br> 壊れています<br>";
        parse_html( message.data(), message.size(), COLOR_CHAR, digitlink, bold, FONT_MAIL );

        constexpr const char str_broken[] = "ここ";
        create_node_link( str_broken, PROTO_BROKEN, COLOR_CHAR_LINK, COLOR_NONE, false );
        create_node_text( "をクリックしてスレを再取得して下さい。", COLOR_CHAR );

        return pos;
    }

    // サブジェクト
    if( header->id_header == 1 ) {
        m_subject.assign( section[4] );

#ifdef _DEBUG
        std::cout << "subject = " << m_subject << std::endl;
#endif
    }

    // 自分の書き込みかチェック
    if( m_check_write && MESSAGE::get_log_manager()->has_items( m_url, m_loading_newthread ) ){

        if( m_buffer_write.capacity() < MAXSISE_OF_LINES ) {
            m_buffer_write.reserve( MAXSISE_OF_LINES );
        }

        // 簡易チェック
        // 最初の lng_check 文字だけ見る
        const bool newthread = ( header->id_header == 1 );
        const std::size_t lng_check = MIN( section[3].size(), 32 );
        parse_write( section[3], lng_check );
        if( MESSAGE::get_log_manager()->check_write( m_url, newthread, m_buffer_write.c_str(), lng_check ) ){

            // 全ての文字列で一致しているかチェック
            parse_write( section[3], 0 );

            const bool hit = MESSAGE::get_log_manager()->check_write( m_url, newthread, m_buffer_write.c_str(), 0 );
            if( hit ){
                m_posts.insert( header->id_header );
            }

#ifdef _DEBUG
            std::cout << "check_write id = " << header->id_header << " hit = " << hit << std::endl;
#endif
        }
    }

    return pos;
}


/** @brief 名前欄のパーサー、DATやHTMLを解析してノードツリーを構築する
 *
 * @details この関数はメンバー変数に処理中のデータを格納するため再入不可能(not reentrant)である。
 * @param[in,out] header ヘッダーノードに対してノードを追加する
 * @param[in] str        DATやHTMLのデータ
 * @param[in] color_name スレビューで使用する色のID (see colorid.h)
 */
void NodeTreeBase::parse_name( NODE* header, std::string_view str, const int color_name )
{
    NODE *node;

    // 後ろの空白を除く
    while( str.size() > 0 && str.back() == ' ' ) str.remove_suffix( 1 );

    const bool defaultname{ m_default_noname == str };

    // 文字列置換
    std::string str_name;
    const CORE::ReplaceStr_Manager* const mgr = CORE::get_replacestr_manager();
    if( mgr->list_get_active( CORE::REPLACETARGET_NAME ) ) {
        str_name = mgr->replace( str, CORE::REPLACETARGET_NAME );
        str = str_name;
    }

    node = header->headinfo->block[ BLOCK_NAMELINK ] = create_node_block();
    node->fontid = FONT_MAIL;

    // デフォルトの名前で無いときはリンクにする
    if( defaultname ) create_node_text( "名前", COLOR_CHAR, false, FONT_MAIL );
    else{
        constexpr const char namestr[] = "名前";
        create_node_link( namestr, PROTO_NAME, COLOR_CHAR, COLOR_NONE, false, FONT_MAIL );
    }

    node = header->headinfo->block[ BLOCK_NAME ] = create_node_block();
    node->fontid = FONT_MAIL;

    // デフォルト名無しと同じときはアンカーを作らない
    if( defaultname ){
        constexpr bool digitlink = false;
        constexpr bool bold = true;
        parse_html( str.data(), str.size(), color_name, digitlink, bold, FONT_MAIL );
    }
    else{

        std::size_t pos = 0;
        std::size_t i;

        while( pos < str.size() ) {

            // トリップなど</b>〜<b>の中の文字列は色を変えて数字をリンクにしない
            for( i = pos; i < str.size(); ++i ) {
                if( i + 4 <= str.size()
                        && str[i] == '<'
                        && str[i + 1] == '/'
                        && ( str[i + 2] == 'b' || str[i + 2] == 'B' )
                        && str[i + 3] == '>' ) {
                    i += 4;
                    break;
                }
            }

            // </b>の前までパース
            if( i != pos ){
                // デフォルト名無しと同じときはアンカーを作らない
                const bool digitlink{ m_default_noname.rfind( str.substr( pos, i - pos ), 0 ) != 0 };
                constexpr bool bold = true;
                parse_html( str.data() + pos, i - pos, color_name, digitlink, bold, FONT_MAIL );
            }
            if( i >= str.size() ) break;
            pos = i;

            // <b>の位置を探す
            std::size_t pos_end = str.size();
            for( i = pos; i < str.size(); ++i ) {
                if( i + 3 <= str.size()
                    && str[ i ] == '<'
                    && ( str[ i+1 ] == 'b' || str[ i+1 ] == 'B' )
                    && str[ i+2 ] == '>' ){
                    pos_end = i;
                    break;
                }
            }

#ifdef _DEBUG        
            std::cout << "NodeTreeBase::parseName trip = " << str.substr( pos, pos_end - pos )
                      << " begin = " << pos << " end = " << pos_end << std::endl;
#endif

            // </b><b>の中をパース
            constexpr bool digitlink = false; // 数字が入ってもリンクしない
            // NOTE: webブラウザでは bold 表示ではないが互換性のため既存の挙動を維持する
            constexpr bool bold = true;
            parse_html( str.data() + pos, pos_end - pos, COLOR_CHAR_NAME_B, digitlink, bold, FONT_MAIL );

            pos = pos_end;
        }
    }

    // plainな名前取得
    // 名前あぼーんや名前抽出などで使用する
    m_buf_text.clear();
    node = node->next_node;
    while( node ) {
        if( node->type == NODE_TEXT && node->text ) m_buf_text.append( node->text );
        if( node->type == NODE_SP ) m_buf_text.push_back( ' ' );
        node = node->next_node;
    }
    header->headinfo->name = m_heap.heap_alloc<char>( m_buf_text.size() + 1 );
    m_buf_text.copy( header->headinfo->name, m_buf_text.size() );
    header->headinfo->name[ m_buf_text.size() ] = '\0';
    m_buf_text.clear();
}


//
// メール
//
void NodeTreeBase::parse_mail( NODE* header, std::string_view str )
{
    // sage 以外の時は色を変える
    int color = COLOR_CHAR;
    if( str.find( "sage" ) == std::string_view::npos ) {
        color = COLOR_CHAR_AGE;
        header->headinfo->sage = false;
    }
    else header->headinfo->sage = true;

    NODE* node = header->headinfo->block[ BLOCK_MAIL ] = create_node_block();
    node->fontid = FONT_MAIL;

    // 文字列置換
    std::string str_mail;
    const CORE::ReplaceStr_Manager* const mgr = CORE::get_replacestr_manager();
    if( mgr->list_get_active( CORE::REPLACETARGET_MAIL ) ) {
        str_mail = mgr->replace( str, CORE::REPLACETARGET_MAIL );
        str = str_mail;
    }

    if( str.empty() ) {
        create_node_text( "[]", color, false, FONT_MAIL );
    }
    else{

        const bool digitlink = true;
        const bool bold = false;

        create_node_text( "[", color, false, FONT_MAIL );
        parse_html( str.data(), str.size(), color, digitlink, bold, FONT_MAIL );
        create_node_text( "]", color, false, FONT_MAIL );
    }
}


/** @brief 日付とID、及びBE、株、その他のパーサー、DATやHTMLを解析してノードツリーを構築する
 *
 * @details この関数はメンバー変数に処理中のデータを格納するため再入不可能(not reentrant)である。
 * @param[in,out] header ヘッダーノードに対してノードを追加する
 * @param[in] str        DATやHTMLのデータ
 */
void NodeTreeBase::parse_date_id( NODE* header, std::string_view str )
{
    std::string str_date;

    // 文字列置換
    const CORE::ReplaceStr_Manager* const mgr = CORE::get_replacestr_manager();
    if( mgr->list_get_active( CORE::REPLACETARGET_DATE ) ) {
        str_date = mgr->replace( str, CORE::REPLACETARGET_DATE );
        str = str_date;
    }

    constexpr bool digitlink = false;
    constexpr bool bold = false;

    std::size_t lng_text = 0;
    NODE *node;

    node = header->headinfo->block[ BLOCK_DATE ] = create_node_block();
    node->fontid = FONT_MAIL;

    for(;;){

        // 先頭の空白を飛ばす
        while( lng_text < str.size() && str[ lng_text ] == ' ' ) ++lng_text;

        // 空白ごとにブロック分けしてパースする
        const std::size_t start_block = lng_text;
        auto it = ( str.begin() + start_block );
        while( it != str.end() && *it != ' ' ) {
            if( *it == '<' ) it = std::find( it + 1, str.end(), '>' );
            else ++it;
        }
        // ブロックの長さ
        std::size_t lng_block = it - ( str.begin() + start_block );

        if( !lng_block ) break;

        if(
            // ID
            ( str[ start_block ] == 'I' && str[ start_block + 1 ] == 'D' )

            // HOST
            || ( str.compare( start_block, 4, "HOST" ) == 0 )

            // 発信元
            || ( str.compare( start_block, 9, "発信元" ) == 0 )

            ){

            // フラッシュ
            if( lng_text ){
                parse_html( str.data(), lng_text, COLOR_CHAR, digitlink, bold, FONT_MAIL );
            }

            std::size_t offset = 0;
            if( str[ start_block ] == 'I' ) offset = 3;
            else if( str[ start_block ] == 'H' ){
                offset = 5;

                // HOST: の場合は途中で空白が入るときがあるので最後までブロックを伸ばす
                lng_block = str.size() - start_block;
            }
            else if( str[ start_block ] == static_cast<char>(0xe7) ) offset = 10;

            // id 取得
            const std::size_t lng_id = (std::min)( lng_block, LNG_ID - sizeof(PROTO_ID) -1 );
            auto id_tmp = str.substr( start_block, lng_id );

            // リンク文字作成
            m_buf_link.assign( PROTO_ID );
            m_buf_link.append( id_tmp );

            // 後ろに●が付いていたら取り除く
            if( m_buf_link.compare( m_buf_link.size() - 3, 3, "\xE2\x97\x8F" ) == 0 ) {
                m_buf_link.resize( m_buf_link.size() - 3 );
            }

            // リンク作成
            node = header->headinfo->block[ BLOCK_ID_NAME ] = create_node_block();
            node->fontid = FONT_MAIL;
            create_node_link( id_tmp.substr( 0, offset ), m_buf_link, COLOR_CHAR, COLOR_NONE, false, FONT_MAIL );
            create_node_ntext( id_tmp.data() +offset, id_tmp.size() -offset, COLOR_CHAR, COLOR_NONE, false, FONT_MAIL );

            // 発言回数ノード作成
            node = create_node_idnum();
            node->fontid = FONT_MAIL;

            // 次のブロックへ移動
            str = str.substr( start_block + lng_block );
            lng_text = 0;
        }

        // BE:
        else if( str[ start_block ] == 'B' && str[ start_block + 1 ] == 'E' ){

            const std::size_t strlen_of_BE = 3; // = strlen( "BE:" );

            // フラッシュ
            if( lng_text ) {
                parse_html( str.data(), lng_text, COLOR_CHAR, digitlink, bold, FONT_MAIL );
            }

            // id 取得
            std::size_t lng_header = 0;
            while( str[ start_block + lng_header ] != '-' && lng_header < lng_block ) ++lng_header;
            const std::size_t lng_id = lng_header - strlen_of_BE;
            if( str[ start_block + lng_header ] == '-' ) ++lng_header;

            if( ! header->headinfo->block[ BLOCK_ID_NAME ] ) {
                node = header->headinfo->block[ BLOCK_ID_NAME ] = create_node_block();
            }
            else {
                node = create_node_space( NODE_SP, COLOR_NONE );
            }
            node->fontid = FONT_MAIL;

            // リンク文字作成
            m_buf_link.assign( PROTO_BE );
            m_buf_link.append( str.substr( start_block + strlen_of_BE, lng_id ) );

            // リンク作成
            auto view_str = str.substr( start_block + lng_header, lng_block - lng_header );
            create_node_link( view_str, m_buf_link, COLOR_CHAR, COLOR_NONE, false, FONT_MAIL );

            // 次のブロックへ移動
            str = str.substr( start_block + lng_block );
            lng_text = 0;
        }

        // 株などの<a href～>
        else if( str[ start_block ] == '<'
                 && ( str[ start_block + 1 ] == 'a' || str[ start_block + 1 ] == 'A' )
                 && str[ start_block + 2 ] == ' ' ){

            // フラッシュ
            if( lng_text ) {
                parse_html( str.data(), lng_text, COLOR_CHAR, digitlink, bold, FONT_MAIL );
            }

            // </a>までブロックの長さを伸ばす
            while( start_block + lng_block < str.size()
                   && ! ( ( str[ start_block + lng_block -1 ] == 'a' || str[ start_block + lng_block -1 ] == 'A' )
                          && str[ start_block + lng_block ] == '>' ) ) ++lng_block;
            ++lng_block;

            if( ! header->headinfo->block[ BLOCK_ID_NAME ] ) {
                node = header->headinfo->block[ BLOCK_ID_NAME ] = create_node_block();
            }
            else {
                node = create_node_space( NODE_SP, COLOR_NONE );
            }
            node->fontid = FONT_MAIL;

            parse_html( str.data() + start_block, lng_block, COLOR_CHAR, digitlink, bold, FONT_MAIL );

            // 次のブロックへ移動
            if( const auto lng = start_block + lng_block; lng < str.size() ) {
                str = str.substr( lng );
            }
            else {
                str = std::string_view{};
            }
            lng_text = 0;
        }

        // テキスト(日付含む)
        else {
            bool no_date = true;
            for( std::size_t i = start_block, end = start_block + lng_block; i < end; ++i ) {
                if( ( str[ i ] >= '0' && str[ i ] <= '9' ) || str[ i ] == ':' ) {
                    no_date = false;
                    break;
                }
            }

            // 日付や時刻を含まないまたは1文字(IDの末尾だけ)はIDノードにする
            if( ( no_date || lng_block == 1 ) && ! header->headinfo->block[ BLOCK_ID_NAME ] ) {
                if( lng_text ) {
                    // フラッシュ
                    parse_html( str.data(), lng_text, COLOR_CHAR, digitlink, bold, FONT_MAIL );
                    lng_text = 0;
                }

                // IDノード作成
                node = header->headinfo->block[ BLOCK_ID_NAME ] = create_node_block();
                node->fontid = FONT_MAIL;

                // 次のブロックへ移動
                str = str.substr( start_block );
            }

            lng_text += lng_block;
        }
    }

    // フラッシュ
    if( lng_text ) {
        // 末端の空白を削ってフラッシュ
        while( lng_text > 0 && str[ lng_text - 1 ] == ' ' ) --lng_text;
        parse_html( str.data(), lng_text, COLOR_CHAR, digitlink, bold, FONT_MAIL );
    }
}


/** @brief HTMLパーサー、HTMLやアンカーを解析してノードツリーを構築する
 *
 * @details この関数はメンバー変数に処理中のデータを格納するため再入不可能(not reentrant)であり
 *          多重に呼び出したり、複数のスレッドから呼び出すと編集中のデータが壊れて正常に動作しない。
 *          この関数を呼び出す関数は同じく再入不可能になるので注意。
 * @param[in] str        DATやHTMLのデータ
 * @param[in] lng_str    DATやHTMLのデータの長さ
 * @param[in] color_text スレビューで使用する色のID (see colorid.h)
 * @param[in] digitlink  true の時は先頭に数字が現れたらアンカーにする( parse_name() などで使う )<br>
 *                       false なら数字の前に >> がついてるときだけアンカーにする
 * @param[in] bold       ボールド表示
 * @param[in] fontid     スレビューで使用するフォントのID (see fontid.h)
 */
// (パッチ)
//
// 行頭の空白は全て除くパッチ
// Thanks to 「パッチ投稿スレ」の28氏
// http://jd4linux.sourceforge.jp/cgi-bin/bbs/test/read.cgi/support/1151836078/28
//
void NodeTreeBase::parse_html( const char* str, std::size_t lng_str, const int color_text,
                               bool digitlink, const bool bold, char fontid )
{
    const char* pos = str;
    const char* const pos_end = str + lng_str;
    int fgcolor = color_text;
    int fgcolor_bak = color_text;
    int bgcolor = COLOR_NONE;
    int bgcolor_bak = COLOR_NONE;
    bool in_bold = bold;
    std::vector<std::string>& colors = CORE::get_css_manager()->get_colors();
    NODE *node;

    m_parsed_text.clear();

    if( *pos == ' ' ){

        pos++;  // 一文字だけなら取り除く

        // 連続半角空白
        if( *pos == ' ' ){

            while( *pos == ' ' ) {
                m_parsed_text.push_back( *(pos++) );
            }
            create_node_multispace( m_parsed_text, bgcolor, fontid );
            m_parsed_text.clear();
        }
    }

    for( ; pos < pos_end; digitlink = false ) {

        ///////////////////////
        // 半角空白, LF(0x0A), CR(0x0D)
        if( *pos == ' ' || *pos == 10 || *pos == 13 ) {

            // フラッシュしてから半角空白ノードを作る
            create_node_ntext( m_parsed_text.data(), m_parsed_text.size(), fgcolor, bgcolor, in_bold, fontid );
            m_parsed_text.clear();

            ++pos;
            if( pos < pos_end ) {
                node = create_node_space( NODE_SP, bgcolor );
                if( fontid != FONT_MAIN ) node->fontid = fontid;
            }

create_multispace:
            // 連続スペースノードを作成
            if( *pos == ' ' ) {
                while( *pos == ' ' ) m_parsed_text.push_back( *pos++ );
                if( pos < pos_end ) {
                    create_node_multispace( m_parsed_text, bgcolor, fontid );
                    m_parsed_text.clear();
                }
            }

            continue;
        }

        ///////////////////////
        // HTMLタグ
        else if( *pos == '<' ) {

            bool br = false;

            // 改行 <br>
            if( ( *( pos + 1 ) == 'b' || *( pos + 1 ) == 'B' )
                && ( *( pos + 2 ) == 'r' || *( pos + 2 ) == 'R' )
                ) br = true;

            // <a href=～></a>
            else if( ( *( pos + 1 ) == 'a' || *( pos + 1 ) == 'A' ) && *( pos + 2 ) == ' ' ){

                // フラッシュ
                create_node_ntext( m_parsed_text.data(), m_parsed_text.size(), fgcolor, bgcolor, in_bold, fontid );
                m_parsed_text.clear();

                for( bool found = false; ! found; ) {
                    while( pos < pos_end && *pos != '=' ) ++pos;
                    // NOTE: 速度を重視するためhrefの判定は最後のfだけチェック
                    // `<a>`タグに含まれる'='、global属性、data属性で誤判定する可能性がある
                    found = ( pos[-1] == 'f' || pos[-1] == 'F' || pos >= pos_end );
                    ++pos;
                }

                if( *pos == ' ' ) ++pos;
                if( pos >= pos_end ) continue;

                char attr_separator = ' ';
                if( *pos == '"' ) {
                    attr_separator = '"';
                    ++pos;
                }

                std::string_view a_link = std::string_view( pos, pos_end - pos );

                while( pos < pos_end && *pos != attr_separator && *pos != '>' ) { ++pos; }
                if( pos >= pos_end ) continue;
                a_link = a_link.substr( 0, pos - a_link.data() );

                while( pos < pos_end && *pos != '>' ) ++pos;
                if( pos >= pos_end ) continue;
                ++pos;

                const char* const pos_str_start = pos;

                while( pos < pos_end
                        && ( *pos != '<' || pos[1] != '/' || ( pos[2] != 'a' && pos[2] != 'A' ) || pos[3] != '>' ) ) {
                    ++pos;
                }
                if( pos >= pos_end ) continue;
                std::string_view a_str( pos_str_start, pos - pos_str_start );

                while( pos < pos_end && *pos != '>' ) ++pos;
                if( pos >= pos_end ) continue;
                ++pos;

                if( ! a_link.empty() && ! a_str.empty() ) {

                    // res anchor linked by <a> element
                    if( a_str.size() > 8 && a_str.compare( 0, 8, "&gt;&gt;" ) == 0 && g_ascii_isdigit( a_str[8] ) ) {
                        // アンカーは後で処理するのでここは抜ける
                        pos = pos_str_start;
                    }

                    // BE link
                    else if( a_link.rfind( "javascript:be(", 0 ) == 0 ) {
                        m_buf_link.assign( PROTO_BE ); // 確保したメモリを再利用するため = は使わない
                        a_link.remove_prefix( 14 );
                        a_link.remove_suffix( 2 );
                        m_buf_link.append( a_link );
                        create_node_link( a_str, m_buf_link, COLOR_CHAR_LINK, bgcolor, in_bold, fontid );
                    }
                    else {

                        // Schema-less URL (net_path)
                        if( a_link.compare( 0, 2, "//" ) == 0 ) {
                            m_buf_link.assign( m_url, 0, m_url.find( '/' ) );
                            m_buf_link.append( a_link );
                            a_link = m_buf_link;
                        }
                        // Relative Reference
                        else if( a_link[0] == '/' || a_link.rfind( "../", 0 ) == 0 ) {
                            const std::size_t link_offset = ( a_link[0] == '/' ) ? 0 : 2;

                            // アンカーの判定
                            const std::size_t pos_root = m_url_readcgi.find( '/', 8 ); // httpsでも8文字目でOK
                            if( pos_root == std::string::npos ||
                                    m_url_readcgi.compare( pos_root, m_url_readcgi.size() - pos_root, a_link,
                                                           link_offset, m_url_readcgi.size() - pos_root ) == 0 ) {
                                // アンカーは後で処理するのでここは抜ける
                                pos = pos_str_start;
                                a_str = std::string_view{};
                            }
                            else if( pos_root + a_link.size() >= LNG_LINK ) {
                                // XXX リンクが長すぎる
                                pos = pos_str_start;
                                a_str = std::string_view{};
                            }
                            else {
                                // 相対リンクから絶対URLを作る
                                m_buf_link.assign( m_url_readcgi, 0, pos_root );
                                a_link.remove_prefix( link_offset );
                                m_buf_link.append( a_link );

                                a_link = m_buf_link;
                            }
                        }
                        else {
                            // 確保したメモリを再利用するため = は使わない
                            m_buf_link.assign( a_link );

                            while( remove_imenu( m_buf_link ) ); // ime.nuなどの除去
                            a_link = m_buf_link;
                        }

                        if( ! a_str.empty() ) {

                            for( std::size_t pos_tmp = 0, str_end = a_str.size(); pos_tmp < str_end; ) {
                                const char ch = a_str[ pos_tmp ];

                                if( ch == '\t' || ch == '\n' || ch == '\r' || ch == ' ' ) {
                                    // 空白の連続は削る
                                    if( ! m_parsed_text.empty() && m_parsed_text.back() != ' ' ) {
                                        m_parsed_text.push_back( ' ' );
                                    }
                                    ++pos_tmp;
                                    continue;
                                }
                                else if( ch == '<' ) {
                                    // タグは取り除く
                                    pos_tmp = a_str.find( '>', pos_tmp + 1 );
                                    if( pos_tmp != std::string_view::npos ) ++pos_tmp;
                                    continue;
                                }
                                else if( ch != '&' ) {
                                    // 表示用文字列にコピー
                                    m_parsed_text.push_back( a_str[ pos_tmp++ ] );
                                    continue;
                                }

                                // 特殊文字デコード
                                int n_in = 0;
                                int n_out = 0;
                                char out_char[kMaxBytesOfUTF8Char]{}; // FIXME: std::stringを受け付けるdecode_char()を作る
                                const int ret_decode = DBTREE::decode_char( a_str.data() + pos_tmp, n_in, out_char, n_out );
                                switch( ret_decode ) {

                                case NODE_HTAB:
                                case NODE_SP:
                                    // 空白の連続は無視する
                                    if( m_parsed_text.empty() || m_parsed_text.back() != ' ' ) {
                                        m_parsed_text.push_back( ' ' );
                                    }
                                    pos_tmp += n_in;
                                    break;

                                case NODE_ZWSP:
                                case NODE_TEXT:
                                    if( out_char[0] == '\xC2' && out_char[1] == '\xA0' ) {
                                        // &nbsp; は空白に置き換える
                                        m_parsed_text.push_back( ' ' );
                                    }
                                    else {
                                        m_parsed_text.append( out_char, n_out );
                                    }
                                    pos_tmp += n_in;
                                    break;

                                case NODE_NONE:
                                default:
                                    m_parsed_text.push_back( a_str[ pos_tmp++ ] );
                                }
                            }
#ifdef _DEBUG
                            std::cout << m_parsed_text << std::endl;
#endif
                            a_str = m_parsed_text;
                        }

                        // アンカーの時はa_strの長さが0でリンクを作らない
                        if( ! a_str.empty() ) {
                            create_node_link( a_str, a_link, COLOR_CHAR_LINK, bgcolor, in_bold, fontid );
                            m_parsed_text.clear();
                        }
                    }
                }
            }

            // </a>
            else if( *( pos + 1 ) == '/' && ( *( pos + 2 ) == 'a' || *( pos + 2 ) == 'A' ) && *( pos + 3 ) == '>' ) pos += 4;

            // 改行にするタグ
            else if(
                // <p>
                (
                    ( *( pos + 1 ) == 'p' || *( pos + 1 ) == 'P' )
                    && *( pos + 2 ) == '>'
                    )

                // </p>
                || (
                    ( *( pos + 2 ) == 'p' || *( pos + 2 ) == 'P' )
                    && *( pos + 3 ) == '>'
                    && *( pos + 1 ) == '/'
                    )

                // <dd>
                || (
                    ( *( pos + 1 ) == 'd' || *( pos + 1 ) == 'D' )
                    && ( *( pos + 2 ) == 'd' || *( pos + 2 ) == 'D' )
                    )

                // </dl>
                || (
                    ( *( pos + 2 ) == 'd' || *( pos + 2 ) == 'D' )
                    && ( *( pos + 3 ) == 'l' || *( pos + 3 ) == 'L' )
                    && *( pos + 1 ) == '/'
                    )

                // <ul>
                || (
                    ( pos[1] == 'u' || pos[1] == 'U' )
                    && ( pos[2] == 'l' || pos[2] == 'L' )
                    )

                // </ul>
                || (
                    ( *( pos + 2 ) == 'u' || *( pos + 2 ) == 'U' )
                    && ( *( pos + 3 ) == 'l' || *( pos + 3 ) == 'L' )
                    && *( pos + 1 ) == '/'
                    )

                // </li>
                || (
                    ( *( pos + 2 ) == 'l' || *( pos + 2 ) == 'L' )
                    && ( *( pos + 3 ) == 'i' || *( pos + 3 ) == 'I' )
                    && *( pos + 1 ) == '/'
                    )

                // </title>
                || (
                    ( *( pos + 2 ) == 't' || *( pos + 2 ) == 'T' )
                    && ( *( pos + 3 ) == 'i' || *( pos + 3 ) == 'I' )
                    && ( *( pos + 4 ) == 't' || *( pos + 4 ) == 'T' )
                    && ( *( pos + 5 ) == 'l' || *( pos + 5 ) == 'L' )
                    && ( *( pos + 6 ) == 'e' || *( pos + 6 ) == 'E' )
                    && *( pos + 1 ) == '/'
                    )

                ) br = true;

            // <li>はBULLET (•)にする
            else if( ( *( pos + 1 ) == 'l' || *( pos + 1 ) == 'L' )
                     && ( *( pos + 2 ) == 'i' || *( pos + 2 ) == 'I' )
                ){

                pos += 4;
                m_parsed_text.append( u8"\u30FB" ); // KATAKANA MIDDLE DOT
            }

            // 水平線 <HR>
            else if( ( *( pos + 1 ) == 'h' || *( pos + 1 ) == 'H' )
                     && ( *( pos + 2 ) == 'r' || *( pos + 2 ) == 'R' ) ){

                // フラッシュ
                create_node_ntext( m_parsed_text.data(), m_parsed_text.size(), fgcolor, bgcolor, in_bold, fontid );
                m_parsed_text.clear();

                // 水平線ノード作成
                node = create_node_hr();
                if (fontid != FONT_MAIN) node->fontid = fontid;

                pos += 4;
            }

            // ボールド <B>
            else if( ( pos[1] == 'b' || pos[1] == 'B' ) && pos[2] == '>' ) {

                // フラッシュ
                create_node_ntext( m_parsed_text.data(), m_parsed_text.size(), fgcolor, bgcolor, in_bold, fontid );
                m_parsed_text.clear();
                in_bold = true;
                pos += 3;
            }

            // </B>
            else if( pos[1] == '/' && ( pos[2] == 'b' || pos[2] == 'B' ) && pos[3] == '>' ) {

                // フラッシュ
                create_node_ntext( m_parsed_text.data(), m_parsed_text.size(), fgcolor, bgcolor, in_bold, fontid );
                m_parsed_text.clear();
                in_bold = bold;
                pos += 4;
            }

            // 閉じるタグの時は文字色を戻す
            else if( pos[1] == '/' ) {
                // フラッシュ
                create_node_ntext( m_parsed_text.data(), m_parsed_text.size(), fgcolor, bgcolor, in_bold, fontid );
                m_parsed_text.clear();

                fgcolor = fgcolor_bak;
                fgcolor_bak = color_text;
                bgcolor = bgcolor_bak;
                bgcolor_bak = COLOR_NONE;

                while( pos < pos_end && *pos != '>' ) ++pos;
                ++pos;
            }

            // その他のタグはタグを取り除いて中身だけを見る
            else {

                // フラッシュ
                create_node_ntext( m_parsed_text.data(), m_parsed_text.size(), fgcolor, bgcolor, in_bold, fontid );
                m_parsed_text.clear();

                // <span
                if( ( pos[1] == 's' || pos[1] == 'S' )
                    && ( pos[2] == 'p' || pos[2] == 'P' )
                    && ( pos[3] == 'a' || pos[3] == 'A' )
                    && ( pos[4] == 'n' || pos[4] == 'N' ) ) {

                    pos += 5;

                    // class属性を取り出す
                    std::string classname;
                    if( g_ascii_strncasecmp( pos, " class=\"", 8 ) == 0 ) {
                        pos += 8;
                        const char* pos_name = pos;
                        while( pos < pos_end && *pos != '"' ) ++pos;
                        classname = std::string( pos_name, pos - pos_name );
                    }

                    // 文字色を保存
                    fgcolor_bak = fgcolor;
                    bgcolor_bak = bgcolor;

                    if( ! classname.empty() && classname != "name" ) {

                        const CORE::Css_Manager* mgr = CORE::get_css_manager();
                        const int classid = mgr->get_classid( classname );
                        if( classid != -1 ) {
                            const CORE::CSS_PROPERTY& css = mgr->get_property( classid );
                            if( css.color != -1 ) fgcolor = css.color;
                            if( css.bg_color != -1 ) bgcolor = css.bg_color;
                        }
                        else {
                            fgcolor = COLOR_CHAR_NAME_B;
                        }
                    }
                }

                // <mark
                else if( ( pos[1] == 'm' || pos[1] == 'M' )
                         && ( pos[2] == 'a' || pos[2] == 'A' )
                         && ( pos[3] == 'r' || pos[3] == 'R' )
                         && ( pos[4] == 'k' || pos[4] == 'K' ) ) {

                    const CORE::Css_Manager* mgr = CORE::get_css_manager();
                    const CORE::CSS_PROPERTY& css = mgr->get_property( mgr->get_classid( "mark" ) );
                    fgcolor_bak = fgcolor;
                    bgcolor_bak = bgcolor;
                    if( css.color != -1 ) fgcolor = css.color;
                    if( css.bg_color != -1 ) bgcolor = css.bg_color;
                }

                if( CONFIG::get_use_color_html() ) {
                    // タグで指定された色を使う場合

                    while( pos < pos_end && *pos != '>' ) {
                        bool background = false;

                        while( pos < pos_end && *pos != '>' && *pos != ' '
                                && *pos != '"' && *pos != '\'' ) ++pos;

                        if( *pos == '>' ) break;

                        const bool pre_char{ *pos == ' ' || *pos == '"' || *pos == '\'' };
                        ++pos;
                        if( ! pre_char ) continue;

                        std::string_view attr_str( pos, pos_end - pos );
                        std::size_t attr_pos = 0;

                        if( attr_str.rfind( "color", 0 ) == 0 ) {
                            attr_pos += 5;
                        }
                        else if( attr_str.rfind( "fgcolor", 0 ) == 0 ) {
                            attr_pos += 7;
                        }
                        else if( attr_str.rfind( "foreground", 0 ) == 0 ) {
                            attr_pos += 10;
                        }
                        else if( attr_str.rfind( "bgcolor", 0 ) == 0 ) {
                            attr_pos += 7;
                            background = true;
                        }
                        else if( attr_str.rfind( "background-color", 0 ) == 0 ) {
                            attr_pos += 16;
                            background = true;
                        }
                        else if( attr_str.rfind( "background", 0 ) == 0 ) {
                            attr_pos += 10;
                            background = true;
                        }
                        else {
                            continue;
                        }

                        if( pos[attr_pos] == '=' || pos[attr_pos] == ':' ) ++attr_pos;
                        if( pos[attr_pos] == ' ' || pos[attr_pos] == '"' ) ++attr_pos;
                        pos += attr_pos;

                        const auto prop_end = attr_str.find_first_of( " \"';>", attr_pos );
                        std::string col_html{ attr_str.substr( attr_pos, prop_end - attr_pos ) };
                        if( col_html == "transparent" ) {
                            // 透明の場合は基本の背景色（でよいか？）
                            if( background ) bgcolor = COLOR_BACK;
                            else fgcolor = COLOR_BACK;
                            continue;
                        }

                        std::string col_str = MISC::htmlcolor_to_str( col_html ) ;
                        if( col_str.empty() ) {
                            MISC::ERRMSG( "unhandled color : " + col_html );
                            continue;
                        }

                        int num_color;
                        const auto it = std::find( colors.cbegin(), colors.cend(), col_str );
                        if( it == colors.cend() ) {
                            colors.push_back( std::move( col_str ) );
                            num_color = colors.size() - 1;
                        }
                        else {
                            num_color = it - colors.cbegin();
                        }

                        // 指定された文字色に変更する
                        if( background ) bgcolor = num_color + USRCOLOR_BASE;
                        else fgcolor = num_color + USRCOLOR_BASE;
                    }
                }

                while( pos < pos_end && *pos != '>' ) ++pos;
                ++pos;
            }

            // 改行実行
            if( br ){

                // フラッシュ
                create_node_ntext( m_parsed_text.data(), m_parsed_text.size(), fgcolor, bgcolor, in_bold, fontid );
                m_parsed_text.clear();

                // 改行ノード作成
                node = create_node_br();
                if (fontid != FONT_MAIN) node->fontid = fontid;

                while( *pos != '>' ) {
                    ++pos;
                }
                ++pos;

                if( *pos == ' ' ){

                    pos++;  // 一文字だけなら取り除く

                    // 連続半角空白
                    if( *pos == ' ' ){

                        while( *pos == ' ' ) {
                            m_parsed_text.push_back( *(pos++) );
                        }
                        create_node_multispace( m_parsed_text, bgcolor, fontid );
                        m_parsed_text.clear();
                    }
                }
            }

            continue;
        }


        ///////////////////////
        // アンカーのチェック
        constexpr std::string_view kProtoAnchor{ PROTO_ANCHORE };
        m_buf_text.clear(); // 画面に表示するリンク用バッファ
        m_buf_link.resize( kProtoAnchor.size() ); // 編集したリンク用バッファ
        int n_in = 0;
        ANCINFO ancinfo[ MAX_ANCINFO ];
        std::size_t lng_anc = 0;

        int mode = 0;
        if( digitlink ) mode = 2;

        if( check_anchor( mode, pos, n_in, m_buf_text, m_buf_link, ancinfo ) ) {

            // フラッシュしてからアンカーノードをつくる
            create_node_ntext( m_parsed_text.data(), m_parsed_text.size(), fgcolor, bgcolor, in_bold, fontid );
            m_parsed_text.clear();

            std::copy( kProtoAnchor.cbegin(), kProtoAnchor.cend(), m_buf_link.begin() );
            ++lng_anc;
            pos += n_in; 

            // , や = や +が続くとき
            // MAX_ANCINFOを超えた部分はリンクに含めない
            mode = 1;
            while( lng_anc < MAX_ANCINFO &&
                   check_anchor( mode, pos, n_in, m_buf_text, m_buf_link, ancinfo + lng_anc ) ) {

                ++lng_anc;
                pos += n_in; 
            }

            create_node_anc( m_buf_text, m_buf_link, COLOR_CHAR_LINK, bold, ancinfo, lng_anc, fontid );

            continue;
        }

        // digitlink = true の時は数字が長すぎるときは飛ばす( 例えば　名前: 12345678 みたいなとき )
        if( digitlink ){
            --n_in;
            while( n_in-- > 0 ) {
                m_parsed_text.push_back( *(pos++) );
            }
        }

        ///////////////////////
        // リンク(http)のチェック
        m_buf_link.clear();
        std::string tmpreplace; // Urlreplaceで変換した後のリンク文字列
        n_in = pos_end - pos;
        const int linktype = check_link( pos, n_in, m_buf_text, m_buf_link );

        if( linktype != MISC::SCHEME_NONE ){
            // リンクノードで実際にアクセスするURLの変換
            while( remove_imenu( m_buf_link ) ); // ime.nuなどの除去

            // Urlreplaceによる正規表現変換
            tmpreplace.assign( m_buf_link );
            if( CORE::get_urlreplace_manager()->exec( tmpreplace ) ){
                // 変換が行われた

                if( tmpreplace.size() > LNG_LINK ){
                    MISC::ERRMSG( "too long replaced url : " + m_buf_link );

                    // 変換後のURLが長すぎるので、元のURLのままにする
                    tmpreplace.assign( m_buf_link );
                }
                // 正常に変換された結果、スキームだけの簡易チェックをする
                else if( MISC::SCHEME_NONE == MISC::is_url_scheme( tmpreplace.c_str() ) ) {
                    // プロトコルスキームが消えていた
                    m_parsed_text.append( m_buf_text );
                    pos += n_in;
                    continue;
                }
            }

            // フラッシュしてからリンクノードつくる
            create_node_ntext( m_parsed_text.data(), m_parsed_text.size(), fgcolor, bgcolor, in_bold, fontid );
            m_parsed_text.clear();

            // ssspアイコン
            if( linktype == MISC::SCHEME_SSSP ){
                node = create_node_sssp( tmpreplace );
                if (fontid != FONT_MAIN) node->fontid = fontid;
            }

            else {
                // Urlreplaceによる画像コントロールを取得する
                const int imgctrl = CORE::get_urlreplace_manager()->get_imgctrl( tmpreplace );

                // youtubeなどのサムネイル画像リンク
                if( imgctrl & CORE::IMGCTRL_THUMBNAIL ){
                    create_node_thumbnail( m_buf_text, m_buf_link, tmpreplace, COLOR_CHAR_LINK, bold, fontid );
                }

                // 画像リンク
                else if( DBIMG::get_type_ext( tmpreplace ) != DBIMG::T_UNKNOWN ){
                    node = create_node_img( m_buf_text, tmpreplace, COLOR_IMG_NOCACHE, bold );
                    if ( fontid != FONT_MAIN ) node->fontid = fontid;
                }

                // 一般リンク
                else create_node_link( m_buf_text, tmpreplace, COLOR_CHAR_LINK, bgcolor, bold, fontid );
            }

            pos += n_in;

            continue;
        }

        ///////////////////////
        // 特殊文字デコード
        if( *pos == '&' ){

            int n_out = 0;
            char out_char[kMaxBytesOfUTF8Char]{};
            const int ret_decode = DBTREE::decode_char( pos, n_in, out_char, n_out );

            if( ret_decode != NODE_NONE ){

                // 文字以外の空白ノードならフラッシュして空白ノード追加
                if( ret_decode != NODE_TEXT ){
                    create_node_ntext( m_parsed_text.data(), m_parsed_text.size(), fgcolor, bgcolor, in_bold, fontid );
                    m_parsed_text.clear();
                    node = create_node_space( ret_decode, bgcolor );
                    if ( fontid != FONT_MAIN ) node->fontid = fontid;
                }
                else if( out_char[0] == '\xC2' && out_char[1] == '\xA0' ) {
                    // &nbsp; は空白に置き換える
                    m_parsed_text.push_back( ' ' );
                }
                else {
                    m_parsed_text.append( out_char, n_out );
                }

                pos += n_in;

                continue;
            }
        }

        ///////////////////////
        // 水平タブ(0x09)
        else if( *pos == '\t' ) {

            // フラッシュしてからタブノードをつくる
            create_node_ntext( m_parsed_text.data(), m_parsed_text.size(), fgcolor, bgcolor, in_bold, fontid );
            m_parsed_text.clear();
            node = create_node_space( NODE_HTAB, bgcolor );
            if( fontid != FONT_MAIN ) node->fontid = fontid;
            ++pos;

            goto create_multispace;
        }

        ///////////////////////
        // フォームフィード(0x0C)
        else if( *pos == '\f' ) {

            // フラッシュしてからZWSPノードをつくる
            create_node_ntext( m_parsed_text.data(), m_parsed_text.size(), fgcolor, bgcolor, in_bold, fontid );
            m_parsed_text.clear();
            node = create_node_space( NODE_ZWSP, bgcolor );
            if( fontid != FONT_MAIN ) node->fontid = fontid;
            ++pos;

            continue;
        }

        ///////////////////////
        // NBSP(0xC2 0xA0)
        else if( *pos == '\xC2' && pos[1] == '\xA0' ) {
            // 空白に置き換える
            m_parsed_text.push_back( ' ' );
            pos += 2;

            continue;
        }

        ///////////////////////
        // その他のASCIIおよびマルチバイト文字
        switch( MISC::utf8bytes( pos ) ) {
            case 4: m_parsed_text.push_back( *pos++ );
                    [[fallthrough]];
            case 3: m_parsed_text.push_back( *pos++ );
                    [[fallthrough]];
            case 2: m_parsed_text.push_back( *pos++ );
                    [[fallthrough]];
            case 1: m_parsed_text.push_back( *pos++ );
                    break;
            default:
                // Iconvでエンコード変換済みなので不正な文字が
                // ここで検出されるのは何かがおかしい
                MISC::ERRMSG( "invalid char = " + std::to_string( static_cast<unsigned char>( *pos ) ) );

                // U+FFFD (REPLACEMENT CHARACTER) に置き換える
                m_parsed_text.append( "\xEF\xBF\xBD" );
                ++pos;
                break;
        }
    }

    create_node_ntext( m_parsed_text.data(), m_parsed_text.size(), fgcolor, bgcolor, in_bold, fontid );
    m_parsed_text.clear();
}


//
// 書き込みログ比較用文字列作成
//
// m_buffer_write に作成した文字列をセットする
//
// max_lng_write > 0 のときは m_buffer_write の文字数が max_lng_write 以上になったら停止
//
void NodeTreeBase::parse_write( std::string_view str, const std::size_t max_lng_write )
{
#ifdef _DEBUG
    std::cout << "NodeTreeBase::parse_write lng = " << str.size() << " max = " << max_lng_write << std::endl;
#endif

    bool head = true;
    const char* pos = str.data();
    const char* pos_end = str.data() + str.size();

    int offset_num;
    int lng_num;

    m_buffer_write.clear();

    // 行頭の空白は全て除く
    while( *pos == ' ' ) ++pos;
    
    for( ; pos < pos_end
         && ( max_lng_write == 0 || m_buffer_write.size() < max_lng_write )
         ; ++pos ){

        // タグ
        if( *pos == '<' ){ 

            // <br>
            if( ( *( pos + 1 ) == 'b' || *( pos + 1 ) == 'B' )
                && ( *( pos + 2 ) == 'r' || *( pos + 2 ) == 'R' )
                ){
                m_buffer_write.push_back( '\n' );
                pos += 3;
            }

            // その他のタグは無視
            else while( pos < pos_end && *pos != '>' ) ++pos;

            continue;
        }

        // gt
        else if( *pos == '&' && ( *( pos + 1 ) == 'g' && *( pos + 2 ) == 't' && *( pos + 3 ) == ';' ) ){

            m_buffer_write.push_back( '>' );
            pos += 3;

            continue;
        }

        // lt
        else if( *pos == '&' && ( *( pos + 1 ) == 'l' && *( pos + 2 ) == 't' && *( pos + 3 ) == ';' ) ){

            m_buffer_write.push_back( '<' );
            pos += 3;

            continue;
        }

        // amp
        else if( *pos == '&' && ( *( pos + 1 ) == 'a' && *( pos + 2 ) == 'm' && *( pos + 3 ) == 'p' && *( pos + 4 ) == ';' ) ){

            m_buffer_write.push_back( '&' );
            pos += 4;

            continue;
        }

        // quot
        else if( *pos == '&' && ( *( pos + 1 ) == 'q' && *( pos + 2 ) == 'u' && *( pos + 3 ) == 'o' && *( pos + 4 ) == 't' && *( pos + 5 ) == ';' ) ){

            m_buffer_write.push_back( '"' );
            pos += 5;

            continue;
        }

        // 先頭のsssp
        else if( head && *pos == 's' && ( *( pos + 1 ) == 's' && *( pos + 2 ) == 's' && *( pos + 3 ) == 'p' ) ){

            // 次のタグ(改行)が来るまで進める
            while( pos < pos_end && *pos != '>' ) ++pos;
            
            continue;
        }

        // 水平タブ
        else if( *pos == '\t' ){

            // 空白に置き換える
            m_buffer_write.push_back( ' ' );

            continue;
        }

        // 数字参照
        else if( *pos == '&' && *( pos + 1 ) == '#' && ( lng_num = MISC::spchar_number_ln( pos, offset_num ) ) != -1 ){

            const char32_t uch = MISC::decode_spchar_number( pos, offset_num, lng_num );
            char utf8[kMaxBytesOfUTF8Char]{};
            const int n_out = MISC::utf32toutf8( uch, utf8 );
            m_buffer_write.append( utf8, n_out );
            pos += offset_num + lng_num;

            continue;
        }

        head = false;
        m_buffer_write.push_back( *pos );
    }
}


/** @brief アンカーが現れたかチェックして文字列を取得する関数
 *
 * @param[in] mode      アンカー記号をチェックするモード
 *                      - 0 なら >> が先頭に無ければアンカーにしない
 *                      - 1 なら,か+か=があればアンカーにする
 *                      - 2 なら数字が先頭に来たらアンカーにする
 * @param[in] str_in    入力文字列の先頭アドレス
 *
 * @param[out] n_in     str_in から何バイト読み取ったか
 * @param[out] str_out  画面に表示される文字列
 * @param[out] str_link リンクの文字列
 * @param[out] ancinfo  ancinfo->anc_from 番から ancinfo->anc_to 番までのアンカーが現れた
 *
 * @return アンカーが現れれば true
 */
bool NodeTreeBase::check_anchor( const int mode, const char* str_in,
                                 int& n_in, std::string& str_out, std::string& str_link, ANCINFO* ancinfo ) const
{
    char tmp_out[ 64 ];
    int lng_out = 0;
    const char* pos = str_in;
    n_in = 0;

    // ">" を最大2回チェック
    if( mode == 0 ){ 

        for( int i = 0; i < 2; ++i ){

            // '>' 
            if( *pos == '&' && *( pos + 1 ) == 'g' && *( pos + 2 ) == 't' ){
                tmp_out[ lng_out++ ] = '>';
                pos += 4;
            }
            // utf-8で"＞"
            else if( ( unsigned char )( *pos ) == 0xef && ( unsigned char ) ( *( pos + 1 ) ) == 0xbc
                     && ( unsigned char ) ( *( pos + 2 ) ) == 0x9e ){
                tmp_out[ lng_out++ ] = static_cast< char >( 0xef );
                tmp_out[ lng_out++ ] = static_cast< char >( 0xbc );
                tmp_out[ lng_out++ ] = static_cast< char >( 0x9e );
                pos += 3;
            }
            else if( i == 0 ) return false;
        }
    }

    // カンマかイコールかプラスをチェック
    else if( mode == 1 ){  

        if( *( pos ) == '=' || *( pos ) == ',' || *( pos ) == '+' ){

            tmp_out[ lng_out++ ] = *( pos );

            str_link.push_back( *pos );

            ++pos;
        }

        // utf-8で"、"
        else if( ( unsigned char )( *pos ) == 0xe3 && ( unsigned char ) ( *( pos + 1 ) ) == 0x80
                 && ( unsigned char ) ( *( pos + 2 ) ) == 0x81 ){

            tmp_out[ lng_out++ ] = static_cast< char >( 0xe3 );
            tmp_out[ lng_out++ ] = static_cast< char >( 0x80 );
            tmp_out[ lng_out++ ] = static_cast< char >( 0x81 );

            str_link.push_back( ',' );

            pos += 3;
        }

        else return false;
    }

    // 数字かチェック
    size_t n, dig;
    int num = MISC::str_to_uint( pos, dig, n );
    if( dig == 0 || dig > MAX_RES_DIGIT || num == 0 ){

        // モード2で数字が長すぎるときは飛ばす
        if( mode == 2 && dig > MAX_RES_DIGIT ) n_in = ( int )( pos - str_in ) + n;

        return false;
    }

    // アンカーが現れたのでとりあえず作成する

    // 画面に表示する文字
    str_out.append( tmp_out, lng_out );
    str_out.append( pos, n );
    pos += n;

    // </a>をキャンセル
    if( *( pos ) == '<' && *( pos + 1 ) == '/' && ( *( pos + 2 ) == 'a' || *( pos + 2 ) == 'A' ) && *( pos + 3 ) == '>' ){

        pos += 4;

        // もう一度数字チェック
        // >>1１ を書き込むと <a href="..">&gt;&gt;1</a>１ となるため
        size_t n2, dig2;
        const int num2 = MISC::str_to_uint( pos, dig2, n2 );
        if( dig2 > 0 && dig2 <= MAX_RES_DIGIT ){

            for( size_t i = 0; i < dig2; ++i ) num *= 10;
            num += num2;

            str_out.append( pos, n2 );
            pos += n2;
        }
    }

    ancinfo->anc_from = ancinfo->anc_to = num;

    // アンカー文字
    std::snprintf( tmp_out, 63, "%d", ancinfo->anc_from );

    // "-" でつながってる場合同じことをもう一回
    int offset = 0;
    if( *( pos ) == '-' ) offset = 1;

    // utf-8で"−"
    else if( ( unsigned char )( * pos ) == 0xef && ( unsigned char ) ( *( pos + 1 ) ) == 0xbc
             && ( unsigned char ) ( *( pos + 2 ) ) == 0x8d ) offset = 3;

    // 半角"-"
    else if( ( unsigned char )( * pos ) == 0xef && ( unsigned char ) ( *( pos + 1 ) ) == 0xbd
             && ( unsigned char ) ( *( pos + 2 ) ) == 0xb0 ) offset = 3;

    if( offset ){
        
        ancinfo->anc_to = MAX( ancinfo->anc_from, MISC::str_to_uint( pos + offset, dig, n ) );
        if( dig && dig <= MAX_RES_DIGIT && ancinfo->anc_to ){

            // 画面に表示する文字
            str_out.append( pos, offset + n );

            // アンカー文字をもう一度作成
            std::snprintf( tmp_out, 63, "%d-%d", ancinfo->anc_from, ancinfo->anc_to );
            pos += offset + n;
        }
    }

    str_link.append( tmp_out );

    //"&gt;&gt;数字-数字</a>"のパターンの時に</a>をのぞく
    if( *( pos ) == '<' && *( pos + 1 ) == '/' && ( *( pos + 2 ) == 'a' || *( pos + 2 ) == 'A' ) && *( pos + 3 ) == '>' ) pos += 4;
    
    n_in = ( int )( pos - str_in );

    return true;
}


/** @brief リンクが現れたかチェックして文字列を取得する関数
 *
 * @details パーセントエンコーディングのオプションが有効のときはデコードして表示文字列に格納する。
 * @param[in] str_in     入力文字列の先頭アドレス
 * @param[in,out] lng_in (in) str_in のバッファサイズ <br> (out) str_in から何バイト読み取ったか
 * @param[out] str_text  リンクの表示文字列
 * @param[out] str_link  リンクの文字列
 *
 * @return リンクのタイプ(例えば MISC::SCHEME_HTTP など)
 * @note MISC::is_url_scheme() と MISC::is_url_char() の仕様に合わせる事
 */
int NodeTreeBase::check_link( const char* str_in, int& lng_in, std::string& str_text, std::string& str_link ) const
{
    // http://, https://, ftp://, ttp(s)://, tp(s):// のチェック
    int delim_pos = 0;
    const int linktype = MISC::is_url_scheme( str_in, &delim_pos );

    if( linktype == MISC::SCHEME_NONE ) return linktype;

    // CONFIG::get_loose_url() == true の時はRFCで規定されていない文字も含める
    const bool loose_url = CONFIG::get_loose_url();

    const char* pos_in = str_in;
    const char* const pos_in_end = str_in + lng_in;

    // URLスキームを修正
    switch( linktype ){
        case MISC::SCHEME_SSSP:
        case MISC::SCHEME_HTTP: str_text.push_back( *pos_in++ );
                                [[fallthrough]];
        case MISC::SCHEME_TTP:  str_text.push_back( *pos_in++ );
                                [[fallthrough]];
        case MISC::SCHEME_TP:   str_text.push_back( *pos_in++ );
    }

    // str_inの文字列をdelim_posまでコピー
    str_link.append( "htt" );
    for( ; pos_in < str_in + delim_pos; ++pos_in ) {
        str_text.push_back( *pos_in );
        str_link.push_back( *pos_in );
    }

    // str_inの残り文字列をstr_linkにコピー
    while( pos_in < pos_in_end ) {
        int n_in_tmp, n_out_tmp;
        const char ch = *pos_in;

        // 文字参照を変換
        char out_char[kMaxBytesOfUTF8Char]{};
        if( ch == '&' && DBTREE::decode_char( pos_in, n_in_tmp, out_char, n_out_tmp ) != NODE_NONE ) {
            str_link.append( out_char, n_out_tmp );
            // URLとして扱う文字かどうか
            if( n_out_tmp == 1 && MISC::is_url_char( str_link.c_str(), loose_url ) ) {
                str_text.push_back( str_link.back() );
                pos_in += n_in_tmp;
            }
            else {
                break;
            }
        }

        // URLとして扱う文字かどうか
        else if( MISC::is_url_char( pos_in, loose_url ) ) {
            str_text.push_back( ch );
            str_link.push_back( ch );
            ++pos_in;
        }
        else {
            break;
        }

        if( str_text.size() >= LNG_LINK || str_link.size() >= LNG_LINK ) return MISC::SCHEME_NONE;

        // loose_urlで含める"^"と"|"をエンコードする
        // "[]"はダウンローダに渡す用途のためにエンコードしないでおく
        if( loose_url ) {

            if( ch == '^' ) {      // '^' → "%5E"(+2Byte)
                if( str_link.size() + 2 >= LNG_LINK ) return MISC::SCHEME_NONE;

                str_link.back() = '%';
                str_link.append( "5E" );
            }
            else if( ch == '|' ) { // '|' → "%7C"(+2Byte)
                if( str_link.size() + 2 >= LNG_LINK ) return MISC::SCHEME_NONE;

                str_link.back() = '%';
                str_link.append( "7C" );
            }
        }
    }

    // 入力から読み取ったバイト数
    lng_in = pos_in - str_in;

    // パーセントエンコーディングの処理
    if( CONFIG::get_percent_decode() && str_text.find( '%' ) != std::string::npos ) {

        std::string tmp = MISC::url_decode( str_text );

        const Encoding enc = MISC::detect_encoding( tmp );

        if( enc == Encoding::sjis || enc == Encoding::eucjp || enc == Encoding::jis ) {
            tmp = MISC::Iconv( tmp, Encoding::utf8, enc );
        }

        if( enc != Encoding::unknown ) {
            // 改行はパーセントエンコード( 元に戻す )
            tmp = MISC::replace_newlines_to_str( tmp, "%0A" );

            if( tmp.size() <= str_text.capacity() ) {
                // 確保したメモリを再利用するため = は使わない
                str_text.assign( tmp );
            }
        }
    }

    // URLとして短かすぎる場合は除外する( 最短ドメイン名の例 "1.cc" )
    if( lng_in - delim_pos < 4 ) return MISC::SCHEME_NONE;


#ifdef _DEBUG
    std::cout << str_link << std::endl
              << "lng_link = " << str_link.size() << " lng_in = " << lng_in << std::endl;
#endif

    return linktype;
}



// あぼーんしているか
bool NodeTreeBase::get_abone( int number ) const
{
    const NODE* head = res_header( number );
    if( ! head ) return false;
    if( ! head->headinfo ) return false;

    return head->headinfo->abone;
}



// あぼーんのクリア
void NodeTreeBase::clear_abone()
{
    for( int i = 1; i <= m_id_header; ++i ){
        NODE* tmphead = m_vec_header[ i ];
        if( tmphead && tmphead->headinfo ) tmphead->headinfo->abone = false;
    }
}



// あぼーん情報を親クラスのarticlebaseからコピーする
void NodeTreeBase::copy_abone_info( const std::list< std::string >& list_abone_id,
                                    const std::list< std::string >& list_abone_name,
                                    const std::list< std::string >& list_abone_word,
                                    const std::list< std::string >& list_abone_regex,
                                    const std::unordered_set< int >& abone_reses,
                                    const bool abone_transparent, const bool abone_chain, const bool abone_age,
                                    const bool abone_default_name, const bool abone_noid,
                                    const bool abone_board, const bool abone_global )
{
    m_list_abone_id = list_abone_id;
    m_list_abone_name = list_abone_name;

    m_list_abone_id_board = DBTREE::get_abone_list_id_board( m_url );
    m_list_abone_name_board = DBTREE::get_abone_list_name_board( m_url );

    std::list<std::string> list_str;
    const auto make_pattern = []( const std::string& query ) {
        const bool icase = CONFIG::get_abone_icase();
        constexpr bool newline = true;
        const bool wchar = CONFIG::get_abone_wchar();
        return JDLIB::RegexPattern( query, icase, newline, false, wchar );
    };

    // 設定ファイルには改行は"\\n"で保存されているので "\n"　に変換する
    m_list_abone_word = MISC::replace_str_list( list_abone_word, "\\n", "\n" );
    list_str = MISC::replace_str_list( list_abone_regex, "\\n", "\n" );
    m_list_abone_regex.clear();
    std::transform( list_str.cbegin(), list_str.cend(), std::back_inserter( m_list_abone_regex ), make_pattern );

    m_list_abone_word_board = DBTREE::get_abone_list_word_board( m_url );
    m_list_abone_word_board = MISC::replace_str_list( m_list_abone_word_board, "\\n", "\n" );
    list_str = DBTREE::get_abone_list_regex_board( m_url );
    list_str = MISC::replace_str_list( list_str, "\\n", "\n" );
    m_list_abone_regex_board.clear();
    std::transform( list_str.cbegin(), list_str.cend(), std::back_inserter( m_list_abone_regex_board ), make_pattern );

    m_list_abone_word_global = MISC::replace_str_list( CONFIG::get_list_abone_word(), "\\n", "\n" );
    list_str = MISC::replace_str_list( CONFIG::get_list_abone_regex(), "\\n", "\n" );
    m_list_abone_regex_global.clear();
    std::transform( list_str.cbegin(), list_str.cend(), std::back_inserter( m_list_abone_regex_global ), make_pattern );

    m_abone_reses = abone_reses;

    if( CONFIG::get_abone_transparent() ) m_abone_transparent = true;
    else m_abone_transparent = abone_transparent;

    if( CONFIG::get_abone_chain() ) m_abone_chain = true;
    else m_abone_chain = abone_chain;

    m_abone_age = abone_age;
    m_abone_default_name = abone_default_name;
    m_abone_noid = abone_noid;
    m_abone_board = abone_board;
    m_abone_global = abone_global;
}


//
// 全レスのあぼーん状態の更新
//
// 発言数や参照数も更新する
//
void NodeTreeBase::update_abone_all()
{
    // あぼーん更新
    clear_abone();
    update_abone( 1, m_id_header );

    // 発言数更新
    clear_id_name();
    update_id_name( 1, m_id_header );

    // 参照状態更新
    clear_reference();
    update_reference( 1, m_id_header );

    // フォント判定更新
    update_fontid( 1, m_id_header );
}


//
// from_number番から to_number 番までのレスのあぼーん状態を更新
//
void NodeTreeBase::update_abone( const int from_number, const int to_number )
{
    if( empty() ) return;
    if( to_number < from_number ) return;

    for( int i = from_number ; i <= to_number; ++i ){
        if( check_abone_res( i ) )  continue;
        if( check_abone_id( i ) )  continue;
        if( check_abone_name( i ) ) continue;
        if( check_abone_mail( i ) ) continue;
        if( check_abone_word( i ) ) continue;
        if( check_abone_chain( i ) ) continue;
    }
}



//
// number番のあぼーん判定(レスあぼーん)
//
// あぼーんの時はtrueを返す
//
bool NodeTreeBase::check_abone_res( const int number )
{
    if( m_abone_reses.find( number ) == m_abone_reses.end() ) return false;

    NODE* head = res_header( number );
    if( ! head ) return false;
    if( ! head->headinfo ) return false;

    head->headinfo->abone = true;
    return true;
}


//
// number番のあぼーん判定( id / board id )
//
// あぼーんの時はtrueを返す
//
bool NodeTreeBase::check_abone_id( const int number )
{
    const bool check_id = ! m_list_abone_id.empty();
    const bool check_id_board = ! m_list_abone_id_board.empty();

    if( !m_abone_noid && !check_id && !check_id_board ) return false;

    NODE* head = res_header( number );
    if( ! head ) return false;
    if( ! head->headinfo ) return false;
    if( head->headinfo->abone ) return true;
    if( ! head->headinfo->block[ BLOCK_ID_NAME ] ) {
        // ID無し
        if( m_abone_noid ) {
            head->headinfo->abone = true;
            return true;
        }
        return false;
    }

    NODE* idnode = head->headinfo->block[ BLOCK_ID_NAME ]->next_node;
    if( ! idnode || ! idnode->linkinfo ) return false;

    const char* const link_id = idnode->linkinfo->link + std::strlen( PROTO_ID );
    const auto equal_id = [link_id]( const std::string& id ) { return id == link_id; };

    // ローカルID
    if( check_id ){
        if( std::any_of( m_list_abone_id.cbegin(), m_list_abone_id.cend(), equal_id ) ) {
            head->headinfo->abone = true;
            return true;
        }
    }

    // 板レベル ID
    if( check_id_board ){
        if( std::any_of( m_list_abone_id_board.cbegin(), m_list_abone_id_board.cend(), equal_id ) ) {
            head->headinfo->abone = true;
            return true;
        }
    }

    return false;
}


//
// number番のあぼーん判定(name /  board name /  global name )
//
// あぼーんの時はtrueを返す
//
bool NodeTreeBase::check_abone_name( const int number )
{
    const bool check_name = ! m_list_abone_name.empty();
    const bool check_name_board = ! m_list_abone_name_board.empty();
    const bool check_name_global = ! CONFIG::get_list_abone_name().empty();

    if( !m_abone_default_name && !check_name && !check_name_board && !check_name_global ) return false;

    NODE* head = res_header( number );
    if( ! head ) return false;
    if( ! head->headinfo ) return false;
    if( head->headinfo->abone ) return true;
    if( ! head->headinfo->name ) return false;

    // デフォルト名無し
    if( m_abone_default_name && head->headinfo->block[ BLOCK_NAMELINK ] ){
        const NODE* idnode = head->headinfo->block[ BLOCK_NAMELINK ]->next_node;
        // デフォルトのときはリンクがない
        if( idnode && ! idnode->linkinfo ){
            head->headinfo->abone = true;
            return true;
        }
    }

    const std::string_view name_str( head->headinfo->name );

    // ローカル name
    if( check_name ){
        for( const std::string& name : m_list_abone_name ) {
            if( name_str.find( name ) != std::string_view::npos ) {
                head->headinfo->abone = true;
                return true;
            }
        }
    }

    // 板レベル name
    if( check_name_board ){
        for( const std::string& name : m_list_abone_name_board ) {
            if( name_str.find( name ) != std::string_view::npos ) {
                head->headinfo->abone = true;
                return true;
            }
        }
    }

    // 全体 name
    if( check_name_global ){
        for( const std::string& name : CONFIG::get_list_abone_name() ) {
            if( name_str.find( name ) != std::string_view::npos ) {
                head->headinfo->abone = true;
                return true;
            }
        }
    }

    return false;
}


//
// number番のあぼーん判定( mail )
//
// あぼーんの時はtrueを返す
//
bool NodeTreeBase::check_abone_mail( const int number )
{
    if( ! m_abone_age ) return false;

    NODE* head = res_header( number );
    if( ! head ) return false;
    if( ! head->headinfo ) return false;
    if( head->headinfo->abone ) return true;

    if( ! head->headinfo->sage ){
        head->headinfo->abone = true;        
        return true;
    }

    return false;
}


//
// number番のあぼーん判定( word, regex  /  board word, board regex / global word, global regex )
//
// あぼーんの時はtrueを返す
//
bool NodeTreeBase::check_abone_word( const int number )
{
    const bool check_word = ! m_list_abone_word.empty();
    const bool check_regex = ! m_list_abone_regex.empty();

    const bool check_word_board = ( m_abone_board && ! m_list_abone_word_board.empty() );
    const bool check_regex_board = ( m_abone_board && ! m_list_abone_regex_board.empty() );

    const bool check_word_global = ( m_abone_global && ! m_list_abone_word_global.empty() );
    const bool check_regex_global = ( m_abone_global && ! m_list_abone_regex_global.empty() );

    if( !check_word && !check_regex
        && !check_word_board && !check_regex_board
        && !check_word_global && !check_regex_global ) return false;

    NODE* head = res_header( number );
    if( ! head ) return false;
    if( ! head->headinfo ) return false;
    if( head->headinfo->abone ) return true;

    const std::string res_str = get_res_str( number );
    JDLIB::Regex regex;
    const size_t offset = 0;

    // ローカル NG word
    if( check_word ){

        for( const std::string& word : m_list_abone_word ) {
            if( res_str.find( word ) != std::string::npos ) {
                head->headinfo->abone = true;
                return true;
            }
        }
    }

    // ローカル NG regex
    if( check_regex ){

        for( const JDLIB::RegexPattern& pattern : m_list_abone_regex ) {
            if( regex.match( pattern, res_str, offset ) ){
                head->headinfo->abone = true;
                return true;
            }
        }
    }

    // 板レベル NG word
    if( check_word_board ){

        for( const std::string& word : m_list_abone_word_board ) {
            if( res_str.find( word ) != std::string::npos ) {
                head->headinfo->abone = true;
                return true;
            }
        }
    }

    // 板レベル NG regex
    if( check_regex_board ){

        for( const JDLIB::RegexPattern& pattern : m_list_abone_regex_board ) {
            if( regex.match( pattern, res_str, offset ) ){
                head->headinfo->abone = true;
                return true;
            }
        }
    }

    // 全体 NG word
    if( check_word_global ){

        for( const std::string& word : m_list_abone_word_global ) {
            if( res_str.find( word ) != std::string::npos ) {
                head->headinfo->abone = true;
                return true;
            }
        }
    }

    // 全体 NG regex
    if( check_regex_global ){

        for( const JDLIB::RegexPattern& pattern : m_list_abone_regex_global ) {
            if( regex.match( pattern, res_str, offset ) ){
                head->headinfo->abone = true;
                return true;
            }
        }
    }

    return false;
}




//
// number番のあぼーん判定(連鎖)
//
// あぼーんしているレスにアンカーを張っているときはtrueを返す
//
bool NodeTreeBase::check_abone_chain( const int number )
{
    if( !m_abone_chain ) return false;

    NODE* head = res_header( number );
    if( ! head ) return false;
    if( ! head->headinfo ) return false;
    if( head->headinfo->abone ) return true;

    bool abone = false;
    for( int block = 0; block < BLOCK_NUM; ++block ){

        NODE* node = head->headinfo->block[ block ];
        while( node ){

            // アンカーノードの時は node->linkinfo->ancinfo != nullptr;
            if( node->type == NODE_LINK && node->linkinfo->ancinfo ){

                int anc = 0;
                for(;;){

                    int anc_from = node->linkinfo->ancinfo[ anc ].anc_from;
                    int anc_to = node->linkinfo->ancinfo[ anc ].anc_to;
                    if( anc_from == 0 ) break;
                    ++anc;

                    // number-1 番以下のレスだけを見る
                    if( anc_from >= number ) continue;
                    anc_to = MIN( anc_to, number -1 );

                    // anc_from から anc_to まで全てあぼーんされているかチェック
                    // ひとつでもあぼーんされていないレスが見付かったらあぼーんしない
                    while( anc_from <= anc_to ){

                        NODE* tmphead = res_header( anc_from++ );
                        if( tmphead && ! tmphead->headinfo->abone ) return false;
                    }

                    abone = true;
                }
            }
            node = node->next_node;
        }
    }

    head->headinfo->abone = abone;
    return abone;
}



// 参照数(num_reference)と色のクリア
void NodeTreeBase::clear_reference()
{
    for( int i = 1; i <= m_id_header; ++i ){
        NODE* tmphead = m_vec_header[ i ];
        if( tmphead && tmphead->headinfo && tmphead->headinfo->block[ BLOCK_NUMBER ]->next_node ){
            tmphead->headinfo->num_reference = 0;
            tmphead->headinfo->block[ BLOCK_NUMBER ]->next_node->color_text = COLOR_CHAR_LINK_RES;
        }
    }
}



//
// from_number番から to_number 番までのレスが参照しているレスの参照数を更新
//
void NodeTreeBase::update_reference( int from_number, int to_number )
{
    if( empty() ) return;
    if( to_number < from_number ) return;
    for( int i = from_number ; i <= to_number; ++i ) check_reference( i );
}



//
// number番のレスが参照しているレスのレス番号の参照数(num_reference)と色をチェック
//
// TBD {update,check}_id_nameとの一貫性からこの関数もupdate_referenceと統合したほうが良いかも？
void NodeTreeBase::check_reference( const int number )
{
    NODE* head = res_header( number );
    if( ! head ) return;

    // 既にあぼーんしているならチェックしない
    if( head->headinfo->abone ) return;

    // 2重チェック防止用
    std::unordered_set< int > checked;
    checked.reserve( m_id_header +1 );

    const bool posted = !m_posts.empty();

    // 過去のレスから number 番へのアンカーがあった場合
    if( m_map_future_refer.size() ){

        std::map< int, std::vector< int > >::iterator it_map = m_map_future_refer.find( number );
        if( it_map != m_map_future_refer.end() ){

            const auto& refs = it_map->second;

            inc_reference( head, refs.size() );

#ifdef _DEBUG
            std::cout << "found number = " << number << " size = " << refs.size() << std::endl;
#endif
            // 過去のレスへ自分の書き込みへの参照マークを付ける
            if( posted && m_posts.find( number ) != m_posts.end() ) {

                for( const int from : refs ) {
#ifdef _DEBUG
                    std::cout << "from " << from << std::endl;
#endif
                    NODE* tmphead = res_header( from );
                    if( tmphead && ! tmphead->headinfo->abone ){
                        m_refer_posts.insert( from );
                    }
                }
            }

            m_map_future_refer.erase( it_map );
#ifdef _DEBUG
            std::cout << "map_future_refer size = " << m_map_future_refer.size() << std::endl;
#endif
        }
    }


    for( int block = 0; block < BLOCK_NUM; ++block ){

        NODE* node = head->headinfo->block[ block ];

        while( node ){

            if( node->type == NODE_LINK ){

                // アンカーノードの時は node->linkinfo->ancinfo != nullptr;
                if( node->linkinfo->ancinfo ){

                    int anc = 0;
                    for(;;){

                        int anc_from = node->linkinfo->ancinfo[ anc ].anc_from;
                        int anc_to = node->linkinfo->ancinfo[ anc ].anc_to;
                        if( anc_from == 0 ) break;
                        ++anc;

                        anc_to = MIN( anc_to, CONFIG::get_max_resnumber() );

                        // >>1-1000 みたいなアンカーは弾く
                        if( anc_to - anc_from >= RANGE_REF ) continue;

                        for( int i = anc_from; i <= anc_to ; ++i ){

                            // 既にチェックしている
                            if( checked.find( i ) != checked.end() ) continue;

                            // 自分自身
                            if( i == number ) continue;

                            // 未来へのレス
                            if( i > number ){
#ifdef _DEBUG
                                std::cout << "future ref " << i << " from " << number << std::endl;
#endif
                                std::map< int, std::vector< int > >::iterator it_map = m_map_future_refer.find( i );
                                if( it_map != m_map_future_refer.end() ){

                                    ( (*it_map).second ).push_back( number );
#ifdef _DEBUG
                                    std::cout << "found size = " << ( (*it_map).second ).size() << std::endl;
#endif
                                }
                                else{
#ifdef _DEBUG
                                    std::cout << "not found\n";
#endif
                                    std::pair< int, std::vector< int > > tmp_pair;
                                    tmp_pair.first = i;
                                    tmp_pair.second.push_back( number );
                                    m_map_future_refer.insert( tmp_pair );
                                }

                                continue;
                            }


                            // 過去へのレス
                            NODE* tmphead = res_header( i );
                            if( tmphead
                                && ! tmphead->headinfo->abone // 対象スレがあぼーんしていたらカウントしない
                                && tmphead->headinfo->block[ BLOCK_NUMBER ]
                                ){

                                checked.insert( i );

                                // 自分の書き込みに対するレス
                                if( posted && m_posts.find( i ) != m_posts.end() ) {
                                    m_refer_posts.insert( number );

#ifdef _DEBUG
                                    std::cout << "ref " << i << " from " << number << std::endl;
#endif
                                }

                                inc_reference( tmphead, 1 );
                            }
                        }
                    }
                }

            }

            node = node->next_node;

        } // while( node )

    } // for( block )
}


//
// 参照数を count だけ増やしてして色を変更
//
void NodeTreeBase::inc_reference( NODE* head, const int count )
{
    head->headinfo->num_reference += count;

    // 参照回数大
    if( head->headinfo->num_reference >= m_num_reference[ LINK_HIGH ] )
        head->headinfo->block[ BLOCK_NUMBER ]->next_node->color_text = COLOR_CHAR_LINK_HIGH;

    // 参照回数中
    else if( head->headinfo->num_reference >= m_num_reference[ LINK_LOW ] )
        head->headinfo->block[ BLOCK_NUMBER ]->next_node->color_text = COLOR_CHAR_LINK_LOW;

    // 参照無し
    else head->headinfo->block[ BLOCK_NUMBER ]->next_node->color_text = COLOR_CHAR_LINK_RES;
}


/**
 * @brief 発言数( num_id_name )と何番目の投稿( posting_order )とIDの色のクリア
 */
void NodeTreeBase::clear_id_name()
{
    for( int i = 1; i <= m_id_header; ++i ){
        NODE* tmphead = m_vec_header[ i ];
        if( tmphead && tmphead->headinfo && tmphead->headinfo->block[ BLOCK_ID_NAME ] ){
            tmphead->headinfo->posting_order = 0;
            tmphead->headinfo->num_id_name = 0;
            tmphead->headinfo->block[ BLOCK_ID_NAME ]->next_node->color_text = COLOR_CHAR;
        }
    }
}



/**
 * @brief from_number 番から to_number 番までの発言数と何番目の投稿を更新
 */
void NodeTreeBase::update_id_name( const int from_number, const int to_number )
{
    if( ! CONFIG::get_check_id() ) return;
    if( empty() ) return;
    if( to_number < from_number ) return;

    //まずIDをキーにしたレス番号の一覧を集計
    for( int i = from_number ; i <= to_number; ++i ) {
        NODE* header = res_header( i );
        if( ! header ) continue;
        NODE* inode = header->headinfo->block[ BLOCK_ID_NAME ];
        if( ! inode || ! inode->next_node || ! inode->next_node->linkinfo ) continue;

        std::string str_id = inode->next_node->linkinfo->link;
        m_map_id_name_resnumber[ str_id ].insert( i );
    }

    //集計したものを元に各ノードの情報を更新
    for( const auto& [id_unused, resnumber_set] : m_map_id_name_resnumber ){ // ID, レス番号の一覧
        const int num_id_name = resnumber_set.size();
        // resnumber_set の型 std::set はソートされた連想コンテナなので
        // レスの順番計算はインクリメントするだけでできる
        int posting_order = 0;
        for( const auto num : resnumber_set ) {
            NODE* header = res_header( num );
            if( ! header ) continue;
            if( ! header->headinfo->block[ BLOCK_ID_NAME ] ) continue;
            set_num_id_name( *header->headinfo, num_id_name, ++posting_order );
        }
     }
}


/** @brief 発言数( num_id_name )と何番目の投稿( posting_order )を更新
 *
 * @details IDノードの色も変更する
 * @param[in] headinfo      IDのヘッダ拡張情報
 * @param[in] num_id_name   発言数
 * @param[in] posting_order 何番目の投稿
 */
void NodeTreeBase::set_num_id_name( HEADERINFO& headinfo, const int num_id_name, const int posting_order )
{
    NODE* idnode = headinfo.block[ BLOCK_ID_NAME ]->next_node;
    if( ! idnode ) return;

    headinfo.posting_order = posting_order;
    headinfo.num_id_name = num_id_name;

    if( num_id_name >= m_num_id[ LINK_HIGH ] ) idnode->color_text = COLOR_CHAR_LINK_ID_HIGH;
    else if( num_id_name >= m_num_id[ LINK_LOW ] ) idnode->color_text = COLOR_CHAR_LINK_ID_LOW;
    else idnode->color_text = COLOR_CHAR;
}


//
// from_number番から to_number 番までのレスのフォント判定を更新
//
void NodeTreeBase::update_fontid( const int from_number, const int to_number )
{
    if( empty() ) return;
    if( to_number < from_number ) return;
    for( int i = from_number ; i <= to_number; ++i ) check_fontid( i );
}


//
// number番のレスのフォント判定を更新
//
// TBD {update,check}_id_nameとの一貫性からこの関数もupdate_fontidと統合したほうが良いかも？
void NodeTreeBase::check_fontid( const int number )
{
    NODE* head = res_header( number );
    if( ! head ) return;
    if( ! head->headinfo ) return;
    if( head->fontid != FONT_EMPTY ) return;

    // ヘッダノードには、フォント判定済みの意味を兼ねて、デフォルトフォントを設定しておく
    head->fontid = FONT_DEFAULT;
    if( ! m_aa_regex.compiled() ) return;

    char fontid_mes = FONT_DEFAULT; // 本文のフォント(fontid.h)

    // AAフォント判定
    const std::string res_str = get_res_str( number );
    JDLIB::Regex regex;
    const size_t offset = 0;

    if( regex.match( m_aa_regex, res_str, offset ) ){
        fontid_mes = FONT_AA;
#ifdef _DEBUG
        std::cout << "NodeTreeBase::check_fontid() fontid = " << FONT_AA
                << " res = " << number << std::endl;
#endif
    }

    // 本文のフォントを設定
    if( fontid_mes != FONT_DEFAULT ){ 
        NODE *node = head->headinfo->block[ BLOCK_MES ];
        while (node) {
            node->fontid = fontid_mes;
            node = node->next_node;
        }
    }
}



/** @brief http://ime.nu/ などをリンクから削除
 *
 * @param[in,out] str_link リンクの文字列
 * @return 取り除いたらtrueを返す
 */
// Thanks to 「パッチ投稿」スレの24氏
//
// http://jd4linux.sourceforge.jp/cgi-bin/bbs/test/read.cgi/support/1151836078/24
//
//static member
bool NodeTreeBase::remove_imenu( std::string& str_link )
{
    std::size_t host_start;
    if( str_link.compare( 0, 8, "https://" ) == 0 ) host_start = 8;
    else if( str_link.compare( 0, 7, "http://" ) == 0 ) host_start = 7;
    else return false;

    std::size_t cut_end;
    if( str_link.compare( host_start, 14, "jump.5ch.net/?" ) == 0
            || str_link.compare( host_start, 14, "pinktower.com/" ) == 0
            || str_link.compare( host_start, 14, "jump.2ch.net/?" ) == 0 ) {
        cut_end = host_start + 14;
    }
    else if( str_link.compare( host_start, 7, "ime.nu/" ) == 0
            || str_link.compare( host_start, 7, "ime.st/" ) == 0
            || str_link.compare( host_start, 7, "nun.nu/" ) == 0 ) {
        cut_end = host_start + 7;
    }
    else if( str_link.compare( host_start, 26, "machi.to/bbs/link.cgi?URL=" ) == 0 ) {
        cut_end = host_start + 26;
    }
    else {
        return false;
    }

    // "http://ime.nu/"等、URLがそれだけだった場合は削除しない
    if( str_link.size() == cut_end ) return false;

    // "httpbin.org"のようなホストがあるのでスラッシュまでチェック
    if( str_link.compare( cut_end, 8, "https://" ) == 0
            || str_link.compare( cut_end, 7, "http://" ) == 0 ) {
        // プロトコルが続く場合は頭から削る
        host_start = 0;
    }

    str_link.erase( host_start, cut_end - host_start );

    return true;
}


// 自分の書き込みにレスしたか
bool NodeTreeBase::is_refer_posted( const int number ) const
{
    return m_refer_posts.find( number ) != m_refer_posts.end();
}


// 書き込みマークセット
void NodeTreeBase::set_posted( const int number, const bool set )
{
    if( set ) {
        m_posts.insert( number );
    }
    else {
        m_posts.erase( number );
    }

    // 自分の書き込みに対するレス
    const std::list< int > res_num = get_res_reference( number );

    // レスされたマークを設定する
    if( set ){
        for( const int n : res_num ) {
            m_refer_posts.insert( n );
        }
    }
    
    // レスされてなくなったので、マークを解除する
    else{
        for( const int n : res_num ) {

            // レスアンカーのリストを取得
            std::list< ANCINFO* > anchors = get_res_anchors( n );
            for( const ANCINFO* anchor : anchors ) {

                // 他の自分の書き込みに対するレスになっていないか？
                const auto end = m_posts.end();
                for( int i = anchor->anc_from; i <= anchor->anc_to; i++ ){
                    // 他の自分の書き込みに対するレス
                    if( m_posts.find( i ) != end ) goto KEEP_POSTMARK;
                }
            }

            // マークを解除する
            m_refer_posts.erase( n );
KEEP_POSTMARK:;
        }
    }
}


// 書き込み履歴のリセット
void NodeTreeBase::clear_post_history()
{
    m_posts.clear();
    m_refer_posts.clear();
}
