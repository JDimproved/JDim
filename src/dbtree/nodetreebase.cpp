// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "nodetreebase.h"
#include "spchar_decoder.h"
#include "interface.h"

#include "jdlib/miscutil.h"
#include "jdlib/miscmsg.h"
#include "jdlib/loaderdata.h"
#include "jdlib/jdregex.h"

#include "dbimg/imginterface.h"

#include "message/logmanager.h"

#include "config/globalconf.h"

#include "global.h"
#include "httpcode.h"
#include "colorid.h"
#include "fontid.h"
#include "command.h"
#include "cache.h"
#include "session.h"
#include "urlreplacemanager.h"

#include <sstream>
#include <fstream>


#ifndef MAX
#define MAX( a, b ) ( a > b ? a : b )
#endif


#ifndef MIN
#define MIN( a, b ) ( a < b ? a : b )
#endif


constexpr size_t SECTION_NUM = 5;
constexpr size_t LNG_RES = 16;
constexpr int LNG_ID = 256;
constexpr size_t LNG_LINK = 256;
constexpr size_t MAX_ANCINFO = 64;
constexpr int RANGE_REF = 20;
constexpr size_t MAX_LINK_DIGIT = 4;  // レスアンカーでMAX_LINK_DIGIT 桁までリンクにする

constexpr size_t MAXSISE_OF_LINES = 512 * 1024;  // ロード時に１回の呼び出しで読み込まれる最大データサイズ
constexpr size_t SIZE_OF_HEAP = MAXSISE_OF_LINES + 64;

constexpr size_t INITIAL_RES_BUFSIZE = 128;  // レスの文字列を返すときの初期バッファサイズ


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
&& ( std::string( node->linkinfo->link ).find( "http" ) == 0 \
|| std::string( node->linkinfo->link ).find( "https" ) == 0  \
|| std::string( node->linkinfo->link ).find( "ftp" ) == 0 )  \
)


using namespace DBTREE;


NodeTreeBase::NodeTreeBase( const std::string& url, const std::string& modified )
    : SKELETON::Loadable(),
      m_url( url ),
      m_lng_dat( 0 ),
      m_resume ( RESUME_NO ),
      m_resume_cached( false ),
      m_broken( false ),
      m_heap( SIZE_OF_HEAP ),
      m_buffer_lines( NULL ),
      m_parsed_text( NULL ),
      m_buffer_write( NULL ),
      m_check_update( false ),
      m_check_write( false ),
      m_loading_newthread( false ),
      m_fout ( NULL )
{
    set_date_modified( modified );

    clear();

    // ヘッダのポインタの配列作成
    m_vec_header = ( NODE** ) m_heap.heap_alloc( sizeof( NODE* ) * MAX_RESNUMBER );

    // ルートヘッダ作成。中は空。
    m_id_header = -1; // ルートヘッダIDが 0 になるように -1
    NODE* tmpnode = create_node_header(); 
    m_vec_header[ m_id_header ] = tmpnode;

    m_default_noname = DBTREE::default_noname( m_url );

    // 参照で色を変える回数
    m_num_reference[ LINK_HIGH ] = CONFIG::get_num_reference_high();
    m_num_reference[ LINK_LOW ] = CONFIG::get_num_reference_low();

    // 発言数で色を変える回数
    m_num_id[ LINK_HIGH ] = CONFIG::get_num_id_high();
    m_num_id[ LINK_LOW ] = CONFIG::get_num_id_low();

    // レスにアスキーアートがあると判定する正規表現
    if( CONFIG::get_aafont_enabled() ){
        m_aa_regex = CONFIG::get_regex_res_aa();
    } else {
        m_aa_regex = std::string();
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
    
    clear();
}


bool NodeTreeBase::empty()
{
    return  m_url.empty();
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

    if( m_buffer_lines ) free( m_buffer_lines );
    m_buffer_lines = NULL;
    m_byte_buffer_lines_left = 0;

    if( m_parsed_text ) free( m_parsed_text );
    m_parsed_text = NULL;

    if( m_buffer_write ) free( m_buffer_write );
    m_buffer_write = NULL;
    
    if( m_fout ) fclose( m_fout );
    m_fout = NULL;

    m_ext_err = std::string();
}


//
// 総レス数
//
// ロード中は m_id_header 番のレスはまだ処理中なので m_id_header -1 を返す
//
int NodeTreeBase::get_res_number()
{
    if( is_loading() ) return m_id_header -1;
    
    return m_id_header;
}


//
// number 番のレスのヘッダのポインタを返す
//
NODE* NodeTreeBase::res_header( int number )
{
    if( number > m_id_header || number <= 0 ) return NULL;
    
    return m_vec_header[ number ];
}


//
// 指定したID の重複数( = 発言数 )
//
// 下の get_num_id_name( int number ) と違って検索するので遅い
//
int NodeTreeBase::get_num_id_name( const std::string& id )
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
int NodeTreeBase::get_num_id_name( const int number )
{
    NODE* head = res_header( number );
    if( ! head ) return 0;

    // IDの数を数えていない場合
    if( ! CONFIG::get_check_id() ) return get_num_id_name( get_id_name( number ) );

    return head->headinfo->num_id_name;
}


//
// 指定した発言者IDを持つレス番号をリストにして取得
//
std::list< int > NodeTreeBase::get_res_id_name( const std::string& id_name )
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
std::list< int > NodeTreeBase::get_res_str_num( const std::string& str_num, std::list< bool >& list_joint )
{
#ifdef _DEBUG
    std::cout << "NodeTreeBase::get_res_str_num " << str_num << std::endl;
#endif

    std::list< int > list_resnum;

    // "," ごとにブロック分けする (例) "1-2,3-4+5" -> "1-2","3-4+5"
    std::list< std::string > list_str_num = MISC::StringTokenizer( str_num, ',' );
    std::list< std::string >::iterator it = list_str_num.begin();
    for( ; it != list_str_num.end(); ++it ){

        // "=" ごとにブロック分けする (例) "1-2=3-4+5" -> "1-2","3-4+5"
        std::list< std::string > list_str_num_eq = MISC::StringTokenizer( ( *it ), '=' );
        std::list< std::string >::iterator it_eq = list_str_num_eq.begin();
        for( ; it_eq != list_str_num_eq.end(); ++it_eq ){

            // true なら前のスレと結合
            bool joint = false;

            // "+"ごとにブロックを分ける (例) "1+2-3+4" -> "1","2-3","4"
            std::list< std::string > list_str_num_pl = MISC::StringTokenizer( ( *it_eq ), '+' );
            std::list< std::string >::iterator it_pl = list_str_num_pl.begin();
            for( ; it_pl != list_str_num_pl.end(); ++it_pl ){

                // num_from から num_to まで表示
                int num_from = MAX( 1, atol( ( *it_pl ).c_str() ) );
                int num_to = 0;

                if( num_from <= m_id_header  ){

                    size_t i;
                    if( ( i = ( *it_pl ).find( "-" ) ) != std::string::npos ) num_to = atol( ( *it_pl ).substr( i +1 ).c_str() );
                    num_to = MIN( MAX( num_to, num_from ), m_id_header );

                    for( int i2 = num_from; i2 <= num_to ; ++i2 ) {

                        //  透明あぼーんしていない　or あぼーんしていないなら追加
                        if( ! m_abone_transparent || ! get_abone( i2 ) ){ 
#ifdef _DEBUG
                            std::cout << *it_pl << " " << num_from << " - " << num_to
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
std::list< int > NodeTreeBase::get_res_with_url()
{
    std::list< int > list_resnum;
    for( int i = 1; i <= m_id_header; ++i ){

        NODE* head = res_header( i );
        if( head ){

            for( int block = 0; block < BLOCK_NUM; ++block ){

                NODE* node = head->headinfo->block[ block ];

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
// 含まれる URL をリストにして取得
//
std::list< std::string > NodeTreeBase::get_urls()
{
    std::list< std::string > list_urls;
    for( int i = 1; i <= m_id_header; ++i ){

        NODE* head = res_header( i );
        if( head ){

            for( int block = 0; block < BLOCK_NUM; ++block ){

                NODE* node = head->headinfo->block[ block ];

                while( node ){
                    if( IS_URL( node ) ) list_urls.push_back( node->linkinfo->link );
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
std::list< int > NodeTreeBase::get_res_reference( const int number )
{
    std::list< int > res_num;
    res_num.push_back( number );
    return get_res_reference( res_num );
}


//
// res_num に含まれるレスを参照しているレス番号をリストにして取得
//
std::list< int > NodeTreeBase::get_res_reference( const std::list< int >& res_num )
{
    std::list< int > list_resnum;

    if( ! res_num.size() ) return list_resnum;

    std::list< int >::const_iterator it_res = res_num.begin();

    for( int i = 1; i <= m_id_header; ++i ){

        //  透明あぼーんは除外
        if( m_abone_transparent && get_abone( i ) ) continue;

        NODE* head = res_header( i );
        if( head ){

            for( int block = 0; block < BLOCK_NUM; ++block ){

                NODE* node = head->headinfo->block[ block ];

                while( node ){

                    if( node->type == NODE_LINK ){

                        // アンカーノードの時は node->linkinfo->ancinfo != NULL;
                        if( node->linkinfo->ancinfo ){

                            int anc = 0;
                            int anc_from;
                            int anc_to;
                            for(;;){

                                anc_from = node->linkinfo->ancinfo[ anc ].anc_from;
                                anc_to = node->linkinfo->ancinfo[ anc ].anc_to;
                                if( anc_from == 0 ) break;
                                ++anc;

                                it_res = res_num.begin();
                                for( ; it_res != res_num.end(); ++it_res ){

                                    const int number = ( *it_res );

                                    
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
    std::list < int >::iterator it;
    for( it = list_resnum.begin(); it != list_resnum.end(); ++it ) std::cout << *it << std::endl;
#endif
    
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

                // アンカーノードの時は node->linkinfo->ancinfo != NULL;
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
const std::list< int > NodeTreeBase::get_res_query( const std::string& query, const bool mode_or )
{
    std::list< int > list_resnum;
    if( query.empty() ) return list_resnum;

    std::list< JDLIB::Regex > list_regex;
    const size_t offset = 0;
    const bool icase = true; // 大文字小文字区別しない
    const bool newline = true; // . に改行をマッチさせない
    const bool usemigemo = true; // migemo使用
    const bool wchar = true; // 全角半角の区別をしない

    const std::list< std::string > list_query = MISC::split_line( query );
    std::list< std::string >::const_iterator it_query;
    for( it_query = list_query.begin(); it_query != list_query.end() ; ++it_query ){

        const std::string &query = ( *it_query );
        
        list_regex.push_back( JDLIB::Regex() );
        list_regex.back().compile( query, icase, newline, usemigemo, wchar );
    }

    for( int i = 1; i <= m_id_header ; ++i ){

        const std::string res_str = get_res_str( i );

        bool apnd = true;
        if( mode_or ) apnd = false;

        std::list< JDLIB::Regex >::iterator it_regex;
        for( it_regex = list_regex.begin(); it_regex != list_regex.end() ; ++it_regex ){

            JDLIB::Regex &regex = ( *it_regex );
            const bool ret = regex.exec( res_str, offset );

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
else if( node->type == DBTREE::NODE_HTAB ) str_res += "\t"; \
else if( node->text ) str_res += node->text; \
node = node->next_node; \
} }while(0) \
        
const std::string NodeTreeBase::get_res_str( int number, bool ref )
{
    std::string str_res;

#ifdef _DEBUG
    std::cout << "NodeTreeBase::get_res_str : num = " << number << std::endl;
#endif

    NODE* head = res_header( number );
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
// number　番のレスの生文字列を返す
//
const std::string NodeTreeBase::get_raw_res_str( int number )
{
#ifdef _DEBUG
    std::cout << "NodeTreeBase::get_raw_res_str : num = " << number << std::endl;
#endif
    std::string retstr;

    std::string str;
    std::string path_cache = CACHE::path_dat( m_url );
    if( ! CACHE::load_rawdata( path_cache, str ) ) return std::string();

    char* rawlines = ( char* ) malloc( str.size() + 64 );
    strcpy( rawlines, str.c_str() );

    // dat形式に変換

    int id_header = m_id_header;
    m_id_header = 0;
    init_loading();

    int byte;
    const char* datlines = raw2dat( rawlines, byte );
    if( byte ){

        std::list< std::string > lines = MISC::get_lines( datlines );
        std::list< std::string >::iterator it = lines.begin();
        for( int i = 1; it != lines.end() && i < number ; ++it, ++i );
        if( it != lines.end() ) retstr = *it;
    }

#ifdef _DEBUG
    std::cout << retstr << std::endl;
#endif

    clear();
    m_id_header = id_header;
    if( rawlines ) free( rawlines );

    return retstr;
}




//
// number番を書いた人の名前を取得
//
const std::string NodeTreeBase::get_name( int number )
{
    NODE* head = res_header( number );
    if( ! head ) return std::string();
    if( ! head->headinfo->name ) return std::string();

    return head->headinfo->name;
}


//
// number番の名前の重複数( = 発言数 )
//
int NodeTreeBase::get_num_name( int number )
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
std::list< int > NodeTreeBase::get_res_name( const std::string& name )
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
const std::string NodeTreeBase::get_time_str( int number )
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
const std::string NodeTreeBase::get_id_name( int number )
{
    NODE* head = res_header( number );
    if( ! head ) return std::string();
    if( ! head->headinfo->block[ BLOCK_ID_NAME ] ) return std::string();

    return head->headinfo->block[ BLOCK_ID_NAME ]->next_node->linkinfo->link;
}



//
// 基本ノード作成
//
NODE* NodeTreeBase::create_node()
{
    NODE* tmpnode = ( NODE* ) m_heap.heap_alloc( sizeof( NODE ) );

    tmpnode->id_header = m_id_header;
    tmpnode->fontid = FONT_EMPTY; // フォントID未設定
    if( m_node_previous ) m_node_previous->next_node = tmpnode;
    m_node_previous = tmpnode;
    
    return tmpnode;
}


//
// ヘッダノード作成
//
NODE* NodeTreeBase::create_node_header()
{
    ++m_id_header;
    m_node_previous = NULL;

    NODE* tmpnode = create_node();
    tmpnode->type =  NODE_HEADER;

    // ヘッダ情報
    tmpnode->headinfo = ( HEADERINFO* )m_heap.heap_alloc( sizeof( HEADERINFO ) );
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
    m_node_previous = NULL;

    NODE* tmpnode = create_node();
    tmpnode->type =  NODE_BLOCK;
    
    return tmpnode;
}


//
// 発言回数(IDの出現数)ノード
//
NODE* NodeTreeBase::create_node_idnum()
{
    const char* dummy = " (10000)";

    NODE* tmpnode = create_node_text( dummy, COLOR_CHAR );
    tmpnode->type = NODE_IDNUM;
    tmpnode->text[ 0 ] = '\0'; // メモリだけ確保して文字を消す
    return tmpnode;
}


//
// 改行ノード作成
//
NODE* NodeTreeBase::create_node_br()
{
    NODE* tmpnode = create_node();
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
NODE* NodeTreeBase::create_node_space( const int type )
{
    NODE* tmpnode = create_node();
    tmpnode->type = type;
    return tmpnode;
}


//
// 連続半角スペース
//
NODE* NodeTreeBase::create_node_multispace( const char* text, const int n )
{
    NODE* tmpnode = create_node_ntext( text, n, COLOR_CHAR, false );
    tmpnode->type = NODE_MULTISP;
    return tmpnode;
}


//
// 水平タブノード
//
NODE* NodeTreeBase::create_node_htab()
{
    NODE* tmpnode = create_node();
    tmpnode->type = NODE_HTAB;
    return tmpnode;
}


//
// リンクノード作成
//
// bold : 太字か
//
NODE* NodeTreeBase::create_node_link( const char* text, const int n, const char* link, const int n_link, const int color_text, const bool bold )
{
    NODE* tmpnode = create_node_ntext( text, n, color_text, bold );

    if( tmpnode ){
        tmpnode->type = NODE_LINK;

        // リンク情報作成
        char *tmplink = ( char* )m_heap.heap_alloc( n_link +4 );
        memcpy( tmplink, link, n_link );
        tmplink[ n_link ] = '\0';

        // リンク情報セット
        tmpnode->linkinfo = ( LINKINFO* )m_heap.heap_alloc( sizeof( LINKINFO ) );
        tmpnode->linkinfo->link = tmplink;
    }
    
    return tmpnode;
}


//
// アンカーノード作成
//
NODE* NodeTreeBase::create_node_anc( const char* text, const int n, const char* link, const int n_link,
                                     const int color_text,  const bool bold,
                                     const ANCINFO* ancinfo, const int lng_ancinfo )
{
    NODE* tmpnode = create_node_link( text, n, link, n_link, color_text, bold );
    if( tmpnode ){

        tmpnode->linkinfo->ancinfo = ( ANCINFO* )m_heap.heap_alloc( sizeof( ANCINFO ) * ( lng_ancinfo + 1 ) );
        memcpy( tmpnode->linkinfo->ancinfo, ancinfo, sizeof( ANCINFO ) * lng_ancinfo );
    }
    
    return tmpnode;
}


//
// SSSPノード
//
NODE* NodeTreeBase::create_node_sssp( const char* link, const int n_link )
{
    NODE* tmpnode = create_node();
    tmpnode->type = NODE_SSSP;

    // リンク情報作成
    char *tmplink = ( char* )m_heap.heap_alloc( n_link +4 );
    memcpy( tmplink, link, n_link );
    tmplink[ n_link ] = '\0';

    // リンク情報セット
    tmpnode->linkinfo = ( LINKINFO* )m_heap.heap_alloc( sizeof( LINKINFO ) );
    tmpnode->linkinfo->link = tmplink;
    tmpnode->linkinfo->image = true;
    tmpnode->linkinfo->imglink = tmpnode->linkinfo->link;

    return tmpnode;
}


//
// 画像ノード作成
//
NODE* NodeTreeBase::create_node_img( const char* text, const int n, const char* link, const int n_link, const int color_text,  const bool bold )
{
    NODE* tmpnode = create_node_link( text, n, link, n_link, color_text, bold );
    if( tmpnode ){
        tmpnode->linkinfo->image = true;
        tmpnode->linkinfo->imglink = tmpnode->linkinfo->link;
    }

    return tmpnode;
}


//
// サムネイル画像ノード ( youtubeなどのサムネイル表示用 )
//
NODE* NodeTreeBase::create_node_thumbnail( const char* text, const int n, const char* link, const int n_link, const char* thumb, const int n_thumb, const int color_text, const bool bold )
{
    NODE* tmpnode = create_node_link( text, n, link, n_link, color_text, bold );

    if( tmpnode ){
        // サムネイル画像のURLをセット
        char *tmpthumb = ( char* )m_heap.heap_alloc( n_thumb +4 );
        memcpy( tmpthumb, thumb, n_thumb );
        tmpthumb[ n_thumb ] = '\0';

        tmpnode->linkinfo->imglink = tmpthumb;
    }

    return tmpnode;
}



//
// テキストノード作成
//
NODE* NodeTreeBase::create_node_text( const char* text, const int color_text, const bool bold )
{
    return create_node_ntext( text, strlen( text ), color_text, bold );
}



//
// テキストノード作成( サイズ指定 )
//
NODE* NodeTreeBase::create_node_ntext( const char* text, const int n, const int color_text, const bool bold )
{
    if( n <= 0 ) return NULL;
    
    NODE* tmpnode = create_node();

    if( tmpnode ){
        tmpnode->type = NODE_TEXT;

        tmpnode->text = ( char* )m_heap.heap_alloc( n +8 );
        memcpy( tmpnode->text, text, n ); tmpnode->text[ n ] = '\0';
        tmpnode->color_text = color_text;
        tmpnode->bold = bold;
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
    if( is_loading() ) return NULL;
    if( html.empty() ) return NULL;

#ifdef _DEBUG
    std::cout << "NodeTreeBase::append_html url = " << m_url << " html = " << html << std::endl;
#endif

    NODE* header = create_node_header();
    m_vec_header[ m_id_header ] = header;

    init_loading();
    header->headinfo->block[ BLOCK_MES ] = create_node_block();

    const bool digitlink = false;
    const bool bold = false;
    const bool ahref = true;
    parse_html( html.c_str(), html.length(), COLOR_CHAR, digitlink, bold, ahref );

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
    if( is_loading() ) return NULL;
    if( dat.empty() ) return NULL;

    init_loading();
    receive_data( dat.c_str(), dat.length() );
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

            const char* data = str.data();
            size_t size = 0;
            m_check_update = false;
            m_check_write = false;
            m_loading_newthread = false;
            set_resume( false );
            init_loading();
            const size_t str_length = str.length();
            while( size < str_length ){
                size_t size_tmp = MIN( MAXSISE_OF_LINES - m_byte_buffer_lines_left, str_length - size );
                receive_data( data + size, size_tmp );
                size += size_tmp;
            }
            receive_finish();

            // レジューム時のチェックデータをキャッシュ
            set_resume_data( data, str_length );
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
        const size_t length_chk = MIN( RESUME_CHKSIZE, length );
        memcpy( m_resume_head, data, length_chk );
        m_resume_head[ length_chk ] = '\0';
    }
}


//
// ロード実行前に呼ぶ初期化関数
//
void NodeTreeBase::init_loading()
{
    clear();

    // 一時バッファ作成
    if( ! m_buffer_lines ) m_buffer_lines = ( char* ) malloc( MAXSISE_OF_LINES ); 
    if( ! m_parsed_text ) m_parsed_text = ( char* ) malloc( MAXSISE_OF_LINES );
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
            if( m_fout == NULL ){
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
void NodeTreeBase::receive_data( const char* data, size_t size )
{
    // BOF防止
    size = MIN( MAXSISE_OF_LINES, size );

    if( is_loading()
        && ( get_code() != HTTP_OK && get_code() != HTTP_PARTIAL_CONTENT ) ){

#ifdef _DEBUG
        std::cout << "NodeTreeBase::receive_data : code = " << get_code() << std::endl;
        std::cout << data << std::endl;
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
        if( data[ 0 ] == '\n' ){

            ++data;
            --size;
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

    if( !size ) return;
    
    // バッファが '\n' で終わるように調整
    const char* pos = data + size;
    if( *pos == '\n' && pos != data ) --pos;
    if( *pos == '\0' ) --pos; // '\0' を除く
    while( *pos != '\n' && pos != data ) --pos;

    // 前回の残りのデータに新しいデータを付け足して add_raw_lines()にデータを送る
    size_t size_in = ( int )( pos - data );
    if( size_in > 0 ){
        size_in ++; // '\n'を加える
        memcpy( m_buffer_lines + m_byte_buffer_lines_left , data, size_in );
        m_buffer_lines[ m_byte_buffer_lines_left + size_in ] = '\0';
        add_raw_lines( m_buffer_lines, m_byte_buffer_lines_left + size_in );
    }

    // add_raw_lines() でレジュームに失敗したと判断したら、バッファをクリアする
    if( m_resume == RESUME_FAILED ){
        m_byte_buffer_lines_left = 0;
        return;
    }

    // 残りの分をバッファにコピーしておく
    m_byte_buffer_lines_left = size - size_in;
    if( m_byte_buffer_lines_left ) memcpy( m_buffer_lines, data + size_in, m_byte_buffer_lines_left );
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
        && get_code() != HTTP_OLD
        ){
        is_error = true;

        std::ostringstream err;
        err << m_url << std::endl
            << "load failed. : " << get_str_code();
        if( get_code() == HTTP_REDIRECT || get_code() == HTTP_REDIRECT ) err << " location = " << location();
        MISC::ERRMSG( err.str() );
    }

    if( ! m_check_update ){

        if( ! is_error ){
            // 特殊スレのdatには、最後の行に'\n'がない場合がある
            if( m_byte_buffer_lines_left > 0 ){
                // 正常に読込完了した場合で、バッファが残っていれば add_raw_lines()にデータを送る
                m_buffer_lines[ m_byte_buffer_lines_left ] = '\0';
                add_raw_lines( m_buffer_lines, m_byte_buffer_lines_left );
                // バッファをクリア
                m_byte_buffer_lines_left = 0;
            }
        }

        // Requested Range Not Satisfiable
        if( get_code() == HTTP_RANGE_ERR ){
            m_broken = true;
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
        && ! get_date_modified().empty() ) CACHE::set_filemtime( CACHE::path_dat( m_url ), get_time_modified() );

    m_check_update = false;
    m_check_write = false;
    m_loading_newthread = false;
}


//
// 鯖から生の(複数)行のデータを受け取ってdat形式に変換して add_one_dat_line() に出力
//
void NodeTreeBase::add_raw_lines( char* rawlines, size_t size )
{
    // 時々サーバ側のdatファイルが壊れていてデータ中に \0 が
    // 入っている時があるので取り除く
    for( size_t i = 0; i < size; ++i ){
        if( rawlines[ i ] == '\0' ){
            MISC::ERRMSG( "EOF was inserted in the middle of the raw data" );
            rawlines[ i ] = ' ';
        } 
    }

    // 保存前にrawデータを加工
    rawlines = process_raw_lines( rawlines );

    size_t lng = strlen( rawlines );
    if( ! lng ) return;

    // サーバが range を無視してデータを送ってきたときのレジューム処理
    if( m_resume == RESUME_MODE2 ){

#ifdef _DEBUG
        std::cout << "NodeTreeBase::add_raw_lines : resume\n";
#endif

        // 先頭からdatを送ってきたかチェック
        const size_t length_chk = MIN( lng, MIN( RESUME_CHKSIZE, strlen( m_resume_head ) ) );
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

    while( * ( pos = add_one_dat_line( pos ) ) != '\0' ) ++pos;

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
    m_vec_header[ m_id_header ] =  header;

    // レス番号
    char tmplink[ LNG_RES ], tmpstr[ LNG_RES ];
    snprintf( tmpstr, LNG_RES, "%d", header->id_header );
    snprintf( tmplink, LNG_RES,"%s%d", PROTO_RES, header->id_header );    

    header->headinfo->block[ BLOCK_NUMBER ] = create_node_block();
    create_node_link( tmpstr, strlen( tmpstr ) , tmplink, strlen( tmplink ), COLOR_CHAR_LINK_RES, true );

    const char* section[ SECTION_NUM ];
    int section_lng[ SECTION_NUM ];
   
    // セクション分けしながら壊れてないかチェック
    for( i = 0; i < SECTION_NUM ; ++i ){
        
        section[ i ] = pos;
        while( !( *pos == '<' && *( pos + 1 ) == '>' ) && ( *pos != '\0' && *pos != '\n' ) ) ++pos;
        section_lng[ i ] = ( int )( pos - section[ i ] );
        
        if( ( *pos == '\0' || *pos == '\n' ) && i < SECTION_NUM -1 ) break; // 壊れてる

        if ( !( *pos == '\0' || *pos == '\n' ) ) pos += 2; // "<>"の分
    }
   
    // 壊れている
    if( i != SECTION_NUM ){

#ifdef _DEBUG
        std::cout << header->id_header << " is broken section = " << i << std::endl;
        std::cout << datline << std::endl;
#endif

        m_broken = true;
        header->headinfo->block[ BLOCK_NAME ] = create_node_block();

        create_node_text( " 壊れています", COLOR_CHAR );

        header->headinfo->block[ BLOCK_MES ] = create_node_block();
        const char str_broken[] = "ここ";
        create_node_link( str_broken, strlen( str_broken ) , PROTO_BROKEN, strlen( PROTO_BROKEN ), COLOR_CHAR_LINK, false );
        create_node_text( "をクリックしてスレを再取得して下さい。", COLOR_CHAR );
        
        return pos;
    }
    
    // 名前
    int color_name = COLOR_CHAR_NAME;
    if( ! section_lng[ 1 ] ) color_name = COLOR_CHAR_NAME_NOMAIL;
    parse_name( header, section[ 0 ], section_lng[ 0 ], color_name );
    
    // メール
    parse_mail( header, section[ 1 ], section_lng[ 1 ] );

    // 日付とID
    parse_date_id( header, section[ 2 ], section_lng[ 2 ] );

    // 本文
    header->headinfo->block[ BLOCK_MES ] = create_node_block();

    const bool digitlink = false;
    const bool bold = false;
    const bool ahref = false;
    parse_html( section[ 3 ], section_lng[ 3 ], COLOR_CHAR, digitlink, bold, ahref );

    // サブジェクト
    if( header->id_header == 1 ){
        m_subject = std::string( section[ 4 ] ).substr( 0, section_lng[ 4 ] );

#ifdef _DEBUG
        std::cout << "subject = " << m_subject << std::endl;
#endif
    }

    // 自分の書き込みかチェック
    if( m_check_write && MESSAGE::get_log_manager()->has_items( m_url, m_loading_newthread ) ){

        if( ! m_buffer_write ) m_buffer_write = ( char* ) malloc( MAXSISE_OF_LINES ); 

        // 簡易チェック
        // 最初の lng_check 文字だけ見る
        const bool newthread = ( header->id_header == 1 );
        const int lng_check = MIN( section_lng[ 3 ], 32 );
        parse_write( section[ 3 ], section_lng[ 3 ], lng_check );
        if( MESSAGE::get_log_manager()->check_write( m_url, newthread, m_buffer_write, lng_check ) ){

            // 全ての文字列で一致しているかチェック
            parse_write( section[ 3 ], section_lng[ 3 ], 0 );

            const bool hit = MESSAGE::get_log_manager()->check_write( m_url, newthread, m_buffer_write, 0 );
            if( hit ){
                if( ! m_vec_posted.size() ) m_vec_posted.resize( MAX_RESNUMBER );
                m_vec_posted[ header->id_header ] = true;
            }

#ifdef _DEBUG
            std::cout << "check_write id = " << header->id_header << " hit = " << hit << std::endl;
#endif
        }
    }

    return pos;
}


//
// 名前
//
void NodeTreeBase::parse_name( NODE* header, const char* str, const int lng, const int color_name )
{
    bool digitlink = true;
    const bool bold = true;
    const bool ahref = false;

    const bool defaultname = ( strncmp( m_default_noname.data(), str, lng ) == 0 );

    header->headinfo->block[ BLOCK_NAMELINK ] = create_node_block();

    // デフォルトの名前で無いときはリンクにする
    if( defaultname ) create_node_text( "名前", COLOR_CHAR );
    else{
        const char namestr[] = "名前";
        create_node_link( namestr, strlen( namestr ) , PROTO_NAME, strlen( PROTO_NAME ), COLOR_CHAR, false );
    }

    NODE* node = header->headinfo->block[ BLOCK_NAME ] = create_node_block();

    // デフォルト名無しと同じときはアンカーを作らない
    if( defaultname ){
        digitlink = false;
        parse_html( str, lng, color_name, digitlink, bold, ahref );
    }
    else{

        int pos = 0;
        int i;

        while( pos < lng ){

            // トリップなど</b>〜<b>の中の文字列は色を変えて数字をリンクにしない
            for( i = pos; i < lng; ++i ){
                if( str[ i ] == '<'
                    && str[ i+1 ] == '/'
                    && ( str[ i+2 ] == 'b' || str[ i+2 ] == 'B' )
                    && str[ i+3 ] == '>' ) break;
            }

            // </b>の前までパース
            if( i != pos ){
                digitlink = true;
                parse_html( str + pos, i - pos, color_name, digitlink, bold, ahref );
            }
            if( i >= lng ) break;
            pos = i + 4; // 4 = strlen( "</b>" );

            // <b>の位置を探す
            int pos_end = lng;
            for( i = pos; i < lng; ++i ){
                if( str[ i ] == '<'
                    && ( str[ i+1 ] == 'b' || str[ i+1 ] == 'B' )
                    && str[ i+2 ] == '>' ){
                    pos_end = i;
                    break;
                }
            }

#ifdef _DEBUG        
            char tmp_str[256];
            memset( tmp_str, 0, 256);
            memcpy( tmp_str, str + pos, pos_end - pos );

            std::cout << "NodeTreeBase::parseName trip = " << tmp_str
                      << " begin = " << pos << " end = " << pos_end << std::endl;
#endif

            // </b><b>の中をパース
            digitlink = false; // 数字が入ってもリンクしない
            parse_html( str + pos, pos_end - pos, COLOR_CHAR_NAME_B, digitlink, bold, ahref );

            pos = pos_end + 3; // 3 = strlen( "<b>" );
        }
    }

    // plainな名前取得
    // 名前あぼーんや名前抽出などで使用する
    if( defaultname ){
        header->headinfo->name = ( char* )m_heap.heap_alloc( lng +2 );
        memcpy( header->headinfo->name, str, lng );
    }
    else{
        std::string str_tmp;
        node = node->next_node;
        while( node ){
            if( node->text ) str_tmp += node->text;
            node = node->next_node;
        }
        header->headinfo->name = ( char* )m_heap.heap_alloc( str_tmp.length() +2 );
        memcpy( header->headinfo->name, str_tmp.c_str(), str_tmp.length() );
    }
}


//
// メール
//
void NodeTreeBase::parse_mail( NODE* header, const char* str, const int lng )
{
    // sage 以外の時は色を変える
    int color = COLOR_CHAR;
    int i = 0;
    while( str[ i ] != 's' && i < lng ) ++i;
    if( str[ i ] != 's' || str[ i+1 ] != 'a' || str[ i+2 ] != 'g' || str[ i+3 ] != 'e' ){
        color = COLOR_CHAR_AGE;
        header->headinfo->sage = FALSE;
    }
    else header->headinfo->sage = TRUE;

    header->headinfo->block[ BLOCK_MAIL ] = create_node_block();

    if( ! lng )  create_node_text( "[]", color );

    else{
        create_node_text( "[", color );

        const bool digitlink = true;
        const bool bold = false;
        const bool ahref = false;
        parse_html( str, lng, color, digitlink, bold, ahref );

        create_node_text( "]", color );
    }
}


//
// 日付とID、及びBE、株、その他
//
void NodeTreeBase::parse_date_id( NODE* header, const char* str, const int lng )
{
    int start = 0;
    int lng_text = 0;
    int lng_link_tmp;
    char tmplink[ LNG_LINK ];

    int lng_id_tmp;
    char tmpid[ LNG_ID ];

    header->headinfo->block[ BLOCK_DATE ] = create_node_block();

    for(;;){

        // 先頭の空白を飛ばす
        while( start + lng_text < lng && str[ start + lng_text ] == ' ' ) ++lng_text;

        // 空白ごとにブロック分けしてパースする
        int start_block = start + lng_text;
        int lng_block = 0; // ブロックの長さ
        while( start_block + lng_block < lng && str[ start_block + lng_block ] != ' ' ) ++lng_block;
        if( !lng_block ) break;

        if(
            // ID ( ??? の時は除く )
            ( str[ start_block ] == 'I' && str[ start_block + 1 ] == 'D' && str[ start_block + 3 ] != '?' )

            // HOST
            || ( str[ start_block + 0 ] == 'H' && str[ start_block + 1 ] == 'O' && str[ start_block + 2 ] == 'S' && str[ start_block + 3 ] == 'T' )

            // 発言元
            || (    str[ start_block + 0 ] == (char)0xe7 && str[ start_block + 1 ] == (char)0x99 && str[ start_block + 2 ] == (char)0xba // 発
                 && str[ start_block + 3 ] == (char)0xe4 && str[ start_block + 4 ] == (char)0xbf && str[ start_block + 5 ] == (char)0xa1 // 言
                 && str[ start_block + 6 ] == (char)0xe5 && str[ start_block + 7 ] == (char)0x85 && str[ start_block + 8 ] == (char)0x83 // 元
                )

            ){

            // フラッシュ
            if( lng_text ){
                if( *( str + start + lng_text - 1 ) == ' ' ) --lng_text;
                create_node_ntext( str + start, lng_text, COLOR_CHAR );
            }

            int offset = 0;
            if( str[ start_block ] == 'I' ) offset = 3;
            else if( str[ start_block ] == 'H' ){
                offset = 5;

                // HOST: の場合は途中で空白が入るときがあるので最後までブロックを伸ばす
                lng_block = lng - start_block;
            }
            else if( str[ start_block ] == (char)0xe7 ) offset = 10;

            // id 取得
            lng_id_tmp = MIN( lng_block, LNG_ID - 16 );
            memcpy( tmpid, str + start_block, lng_id_tmp );
            tmpid[ lng_id_tmp ] = '\0';
            
            // リンク文字作成
            memcpy( tmplink, PROTO_ID, sizeof( PROTO_ID ) );
            memcpy( tmplink + sizeof( PROTO_ID ) - 1, tmpid, lng_id_tmp + 1 );
            lng_link_tmp = strlen( tmplink );

            // 後ろに●が付いていたら取り除く
            if( tmplink[ lng_link_tmp - 3 ] == (char)0xe2 && tmplink[ lng_link_tmp - 2 ] == (char)0x97 && tmplink[ lng_link_tmp - 1 ] == (char)0x8f ){
                lng_link_tmp -= 3;
                tmplink[ lng_link_tmp ] = '\0';
            }

            // リンク作成
            header->headinfo->block[ BLOCK_ID_NAME ] = create_node_block();
            create_node_link( tmpid, offset, tmplink, lng_link_tmp, COLOR_CHAR, false );
            create_node_ntext( tmpid +offset, lng_id_tmp -offset, COLOR_CHAR);

            // 発言回数ノード作成
            create_node_idnum();

            // 次のブロックへ移動
            start = start_block + lng_block;
            lng_text = 0;
        }

        // BE:
        else if( str[ start_block ] == 'B' && str[ start_block + 1 ] == 'E' ){

            const int strlen_of_BE = 3; // = strlen( "BE:" );

            // フラッシュ
            if( lng_text ) create_node_ntext( str + start, lng_text, COLOR_CHAR );

            // id 取得
            int lng_header = 0;
            while( str[ start_block + lng_header ] != '-' && lng_header < lng_block ) ++lng_header;
            lng_id_tmp = lng_header - strlen_of_BE;
            if( str[ start_block + lng_header ] == '-' ) ++lng_header;
            memcpy( tmpid, str + start_block + strlen_of_BE, lng_id_tmp );
            tmpid[ lng_id_tmp ] = '\0';

            // リンク文字作成
            memcpy( tmplink, PROTO_BE, sizeof( PROTO_BE ) );
            memcpy( tmplink + sizeof( PROTO_BE ) -1, tmpid, lng_id_tmp + 1 );

            // リンク作成
            create_node_link( "?", 1, tmplink, strlen( tmplink ), COLOR_CHAR, false );
            create_node_ntext( str + start_block + lng_header, lng_block - lng_header, COLOR_CHAR);

            // 次のブロックへ移動
            start = start_block + lng_block;
            lng_text = 0;
        }

        // 株などの<a href～>
        else if( str[ start_block ] == '<'
                 && ( str[ start_block + 1 ] == 'a' || str[ start_block + 1 ] == 'A' )
                 && str[ start_block + 2 ] == ' ' ){

            // フラッシュ
            if( lng_text ) create_node_ntext( str + start, lng_text, COLOR_CHAR );

            // </a>までブロックの長さを伸ばす
            while( start_block + lng_block < lng
                   && ! ( ( str[ start_block + lng_block -1 ] == 'a' || str[ start_block + lng_block -1 ] == 'A' )
                          && str[ start_block + lng_block ] == '>' ) ) ++lng_block;
            ++lng_block;

            const bool digitlink = false;
            const bool bold = false;
            const bool ahref = true;
            parse_html( str + start_block, lng_block, COLOR_CHAR, digitlink, bold, ahref );

            // 次のブロックへ移動
            start = start_block + lng_block;
            lng_text = 0;
        }

        // テキスト(日付含む)
        else lng_text += lng_block;
    }

    // フラッシュ
    if( lng_text ) create_node_ntext( str + start, lng_text, COLOR_CHAR );
}


//
// HTMLパーサ
//
// digitlink : true の時は先頭に数字が現れたらアンカーにする( parse_name() などで使う )
//             false なら数字の前に >> がついてるときだけアンカーにする
//
// bold : ボールド表示
//
// ahref : <a href=～></a> からリンクノードを作成する
// (例) parse_html( "<a href=\"hoge.com\">hoge</a>", 27, COLOR_CHAR, false, false );
//
// (パッチ)
//
// 行頭の空白は全て除くパッチ
// Thanks to 「パッチ投稿スレ」の28氏
// http://jd4linux.sourceforge.jp/cgi-bin/bbs/test/read.cgi/support/1151836078/28
//
void NodeTreeBase::parse_html( const char* str, const int lng, const int color_text,
                               bool digitlink, const bool bold, const bool ahref )
{
    const char* pos = str;
    const char* pos_end = str + lng;
    int lng_text = 0;
    
    if( *pos == ' ' ){

        pos++;  // 一文字だけなら取り除く

        // 連続半角空白
        if( *pos == ' ' ){

            while( *pos == ' ' ) m_parsed_text[ lng_text++ ] = *(pos++);
            create_node_multispace( m_parsed_text, lng_text ); lng_text = 0;
        }
    }
   
    for( ; pos < pos_end; ++pos, digitlink = false ){


        ///////////////////////
        // HTMLタグ
        if( *pos == '<' ){ 

            bool br = false;

            // 改行 <br>
            if( ( *( pos + 1 ) == 'b' || *( pos + 1 ) == 'B' )
                && ( *( pos + 2 ) == 'r' || *( pos + 2 ) == 'R' )
                ) br = true;

            //  ahref == true かつ <a href=～></a>
            else if( ahref &&
                     ( *( pos + 1 ) == 'a' || *( pos + 1 ) == 'A' ) && *( pos + 2 ) == ' ' ){

                // フラッシュ
                create_node_ntext( m_parsed_text, lng_text, color_text, bold ); lng_text = 0;

                while( pos < pos_end && *pos != '=' ) ++pos;
                ++pos;

                if( *pos == ' ' ) ++pos;
                if( pos >= pos_end ) continue;

                bool dbq = false;
                if( *pos == '"' ){
                    dbq = true;
                    ++pos;
                }

                const char* pos_link_start = pos;
                int lng_link = 0;

                while( pos < pos_end && ( ( ( dbq && *pos != '"' ) || ( !dbq && *pos != ' ' ) ) && *pos != '>' ) ){ ++pos; ++lng_link; }
                if( pos >= pos_end ) continue;

                while( pos < pos_end && *pos != '>' ) ++pos;
                if( pos >= pos_end ) continue;
                ++pos;

                const char* pos_str_start = pos;
                int lng_str = 0;

                bool exec_decode = false;
                while( pos < pos_end && *pos != '<' ){
                    if( *pos == '&' ) exec_decode = true;
                    ++pos;
                    ++lng_str;
                }
                if( pos >= pos_end ) continue;

                while( pos < pos_end && *pos != '>' ) ++pos;
                if( pos >= pos_end ) continue;
                ++pos;

                if( lng_link && lng_str ){

                    // 特殊文字デコード
                    if( exec_decode ){

                        for( int pos_tmp = 0; pos_tmp < lng_str; ++ pos_tmp ){
                            int n_in = 0;
                            int n_out = 0;
                            const int ret_decode = DBTREE::decode_char( pos_str_start + pos_tmp, n_in, m_parsed_text + lng_text, n_out, false );
                            if( ret_decode != NODE_NONE ){
                                lng_text += n_out;
                                pos_tmp += n_in;
                                pos_tmp--;
                            }
                            else m_parsed_text[ lng_text++ ] = *( pos_str_start + pos_tmp );
                        }
#ifdef _DEBUG
                        m_parsed_text[ lng_text ] = '\0';
                        std::cout << m_parsed_text << std::endl;
#endif
                        pos_str_start = m_parsed_text;
                        lng_str = lng_text;
                        lng_text = 0;
                    }

                    create_node_link( pos_str_start, lng_str , pos_link_start, lng_link, COLOR_CHAR_LINK, false );
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

            // <li>は・にする
            else if( ( *( pos + 1 ) == 'l' || *( pos + 2 ) == 'L' )
                     && ( *( pos + 2 ) == 'i' || *( pos + 3 ) == 'I' )
                ){

                pos += 4;

                int n_in = 0;
                int n_out = 0;
                DBTREE::decode_char( "&#12539;", n_in, m_parsed_text + lng_text, n_out, false );
                lng_text += n_out;
            }

            // 水平線 <HR>
            else if( ( *( pos + 1 ) == 'h' || *( pos + 1 ) == 'H' )
                     && ( *( pos + 2 ) == 'r' || *( pos + 2 ) == 'R' ) ){

                // フラッシュ
                create_node_ntext( m_parsed_text, lng_text, color_text, bold ); lng_text = 0;

                // 水平線ノード作成
                create_node_hr();

                pos += 4;
            }

            // その他のタグは無視。タグを取り除いて中身だけを見る
            else {

                // フラッシュ
                create_node_ntext( m_parsed_text, lng_text, color_text, bold ); lng_text = 0;
                
                while( pos < pos_end && *pos != '>' ) ++pos;
                ++pos;
            }

            // 改行実行
            if( br ){

                // フラッシュ
                if( *( pos - 1 ) == ' ' ) --lng_text; // 改行前の空白を取り除く
                create_node_ntext( m_parsed_text, lng_text, color_text, bold ); lng_text = 0;

                // 改行ノード作成
                create_node_br();

                while( *pos != '>' ) {
                    ++pos;
                }
                ++pos;

                if( *pos == ' ' ){

                    pos++;  // 一文字だけなら取り除く

                    // 連続半角空白
                    if( *pos == ' ' ){

                        while( *pos == ' ' ) m_parsed_text[ lng_text++ ] = *(pos++);
                        create_node_multispace( m_parsed_text, lng_text ); lng_text = 0;
                    }
                }
            }

            // forのところで++されるので--しておく
            --pos;
            continue;
        }


        ///////////////////////
        // アンカーのチェック
        int n_in = 0;
        int n_out = 0;
        char tmpstr[ LNG_LINK +16 ]; // 画面に表示する文字列
        char tmplink[ LNG_LINK +16 ]; // 編集したリンク文字列
        int lng_str = 0, lng_link = strlen( PROTO_ANCHORE );
        ANCINFO ancinfo[ MAX_ANCINFO ];
        int lng_anc = 0;

        int mode = 0;
        if( digitlink ) mode = 2;

        if( check_anchor( mode , pos, n_in, tmpstr + lng_str, tmplink + lng_link, LNG_LINK - lng_link, ancinfo + lng_anc ) ){

            // フラッシュしてからアンカーノードをつくる
            create_node_ntext( m_parsed_text, lng_text, color_text, bold ); lng_text = 0;

            memcpy( tmplink, PROTO_ANCHORE, strlen( PROTO_ANCHORE ) );
            lng_str += strlen( tmpstr ) - lng_str;
            lng_link += strlen( tmplink ) - lng_link;
            ++lng_anc;
            pos += n_in; 

            // , や = や +が続くとき
            mode = 1;
            while( check_anchor( mode, pos, n_in, tmpstr + lng_str, tmplink + lng_link ,
                                 LNG_LINK - lng_link, ancinfo + lng_anc ) ){

                lng_str += strlen( tmpstr ) - lng_str;
                lng_link += strlen( tmplink ) - lng_link;
                ++lng_anc;
                pos += n_in; 
            }

            create_node_anc( tmpstr, lng_str, tmplink, lng_link, COLOR_CHAR_LINK, bold, ancinfo, lng_anc );

            // forのところで++されるので--しておく
            --pos;
            continue;
        }

        // digitlink = true の時は数字が長すぎるときは飛ばす( 例えば　名前: 12345678 みたいなとき )
        if( digitlink ){
            --n_in;
            while( n_in-- > 0 ) m_parsed_text[ lng_text++ ] = *(pos++);
        }

        ///////////////////////
        // リンク(http)のチェック
        char tmpreplace[ LNG_LINK +16 ]; // Urlreplaceで変換した後のリンク文字列
        int lng_replace = 0;
        int linktype = check_link( pos, (int)( pos_end - pos ), n_in, tmplink, LNG_LINK );
        if( linktype != MISC::SCHEME_NONE ){
            // リンクノードで実際にアクセスするURLの変換
            while( remove_imenu( tmplink ) ); // ime.nuなどの除去
            lng_link = convert_amp( tmplink, strlen( tmplink ) ); // &amp; → &

            // Urlreplaceによる正規表現変換
            std::string tmpurl( tmplink, lng_link );
            if( CORE::get_urlreplace_manager()->exec( tmpurl ) == false ){
                // 変換されてない
                lng_replace = lng_link;
                memcpy( tmpreplace, tmplink, lng_replace +1 );

            } else {
                if( tmpurl.size() > LNG_LINK ){
                    MISC::ERRMSG( std::string( "too long replaced url : " ) + tmplink );

                    // 変換後のURLが長すぎるので、元のURLのままにする
                    lng_replace = lng_link;
                    memcpy( tmpreplace, tmplink, lng_replace +1 );
                } else {
                    // 正常に変換された
                    lng_replace = tmpurl.size();
                    memcpy( tmpreplace, tmpurl.c_str(), lng_replace +1 );

                    // 正規表現変換の結果、スキームだけの簡易チェックをする
                    int delim_pos = 0;
                    if( MISC::SCHEME_NONE == MISC::is_url_scheme( tmpreplace, &delim_pos ) ){
                        // スキーム http:// が消えていた
                        linktype = MISC::SCHEME_NONE;
                    }
                }
            }
        }
        // リンクノードか再チェック
        if( linktype != MISC::SCHEME_NONE ){
            // フラッシュしてからリンクノードつくる
            create_node_ntext( m_parsed_text, lng_text, color_text, bold ); lng_text = 0;

            // リンクノードの表示テキスト
            memcpy( tmpstr, pos, n_in );
            tmpstr[ n_in ] = '\0';
            lng_str = convert_amp( tmpstr, n_in ); // &amp; → &

            // ssspアイコン
            if( linktype == MISC::SCHEME_SSSP ){
                create_node_sssp( tmpreplace, lng_replace );
            }

            else {
                // Urlreplaceによる画像コントロールを取得する
                int imgctrl = CORE::get_urlreplace_manager()->get_imgctrl( std::string( tmpreplace, lng_replace ) );

                // youtubeなどのサムネイル画像リンク
                if( imgctrl & CORE::IMGCTRL_THUMBNAIL ){
                    create_node_thumbnail( tmpstr, lng_str, tmplink , lng_link, tmpreplace, lng_replace, COLOR_CHAR_LINK, bold );
                }

                // 画像リンク
                else if( DBIMG::get_type_ext( tmpreplace, lng_replace ) != DBIMG::T_UNKNOWN ){
                    create_node_img( tmpstr, lng_str, tmpreplace , lng_replace, COLOR_IMG_NOCACHE, bold );
                }
    
                // 一般リンク
                else create_node_link( tmpstr, lng_str, tmpreplace , lng_replace, COLOR_CHAR_LINK, bold );
            }

            pos += n_in;

            // forのところで++されるので--しておく
            --pos;
            continue;
        }

        ///////////////////////
        // 特殊文字デコード
        if( *pos == '&' ){

            const int ret_decode = DBTREE::decode_char( pos, n_in, m_parsed_text + lng_text, n_out, false );

            if( ret_decode != NODE_NONE ){

                // 文字以外の空白ノードならフラッシュして空白ノード追加
                if( ret_decode != NODE_TEXT ){
                    create_node_ntext( m_parsed_text, lng_text, color_text, bold );
                    lng_text = 0;
                    create_node_space( ret_decode );
                }
                else lng_text += n_out;

                pos += n_in;

                // forのところで++されるので--しておく
                --pos;
                continue;
            }
        }

        ///////////////////////
        // 水平タブ(0x09)
        if( *pos == 0x09 ){

            // フラッシュしてからタブノードをつくる
            create_node_ntext( m_parsed_text, lng_text, color_text, bold ); lng_text = 0;
            create_node_htab();
            continue;
        }

        ///////////////////////
        // LF(0x0A), CR(0x0D)
        if( *pos == 0x0A || *pos == 0x0D ){

            // 無視する
            continue;
        }

        ///////////////////////
        // 連続半角空白
        if( *pos == ' ' && *( pos + 1 ) == ' ' ){

            m_parsed_text[ lng_text++ ] = *(pos++);

            // フラッシュしてから連続半角ノードを作る
            create_node_ntext( m_parsed_text, lng_text, color_text, bold ); lng_text = 0;

            while( *pos == ' ' ) m_parsed_text[ lng_text++ ] = *(pos++);
            create_node_multispace( m_parsed_text, lng_text ); lng_text = 0;

            // forのところで++されるので--しておく
            --pos;
            continue;
        }

        m_parsed_text[ lng_text++ ] = *pos;
    }

    create_node_ntext( m_parsed_text, lng_text, color_text, bold );
}


//
// 書き込みログ比較用文字列作成
//
// m_buffer_write に作成した文字列をセットする
//
// max_lng_write > 0 のときは m_buffer_write の文字数が max_lng_write 以上になったら停止
//
void NodeTreeBase::parse_write( const char* str, const int lng, const int max_lng_write )
{
    if( ! m_buffer_write ) return;

#ifdef _DEBUG
    std::cout << "NodeTreeBase::parse_write lng = " << lng << " max = " << max_lng_write << std::endl;
#endif

    bool head = true;
    const char* pos = str;
    const char* pos_end = str + lng;

    int offset_num;
    int lng_num;

    char* pos_write = m_buffer_write;

    // 行頭の空白は全て除く
    while( *pos == ' ' ) ++pos;
    
    for( ; pos < pos_end
         && ( max_lng_write == 0 || (int)( pos_write - m_buffer_write ) < max_lng_write )
         ; ++pos ){

        // タグ
        if( *pos == '<' ){ 

            // <br>
            if( ( *( pos + 1 ) == 'b' || *( pos + 1 ) == 'B' )
                && ( *( pos + 2 ) == 'r' || *( pos + 2 ) == 'R' )
                ){
                *(pos_write++) = '\n';
                pos += 3;
            }

            // その他のタグは無視
            else while( pos < pos_end && *pos != '>' ) ++pos;

            continue;
        }

        // gt
        else if( *pos == '&' && ( *( pos + 1 ) == 'g' && *( pos + 2 ) == 't' && *( pos + 3 ) == ';' ) ){

            *(pos_write++) = '>';
            pos += 3;

            continue;
        }

        // lt
        else if( *pos == '&' && ( *( pos + 1 ) == 'l' && *( pos + 2 ) == 't' && *( pos + 3 ) == ';' ) ){

            *(pos_write++) = '<';
            pos += 3;

            continue;
        }

        // amp
        else if( *pos == '&' && ( *( pos + 1 ) == 'a' && *( pos + 2 ) == 'm' && *( pos + 3 ) == 'p' && *( pos + 4 ) == ';' ) ){

            *(pos_write++) = '&';
            pos += 4;

            continue;
        }

        // quot
        else if( *pos == '&' && ( *( pos + 1 ) == 'q' && *( pos + 2 ) == 'u' && *( pos + 3 ) == 'o' && *( pos + 4 ) == 't' && *( pos + 5 ) == ';' ) ){

            *(pos_write++) = '"';
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
            *(pos_write++) = ' ';

            continue;
        }

        // 数字参照
        else if( *pos == '&' && *( pos + 1 ) == '#' && ( lng_num = MISC::spchar_number_ln( pos, offset_num ) ) != -1 ){

            const int num = MISC::decode_spchar_number( pos, offset_num, lng_num );
            const int n_out = MISC::ucs2toutf8( num, pos_write );
            pos_write += n_out;
            pos += offset_num + lng_num;

            continue;
        }

        head = false;
        *(pos_write++) = *pos;
    }

    *pos_write = '\0';
}


//
// アンカーが現れたかチェックして文字列を取得する関数
//
// 入力
// mode : 0 なら >> が先頭に無ければアンカーにしない、1 なら,か+か=があればアンカーにする、2 なら数字が先頭に来たらアンカーにする
// str_in : 入力文字列の先頭アドレス
// lng_link : str_linkのバッファサイズ
//
// 出力
// n_in : str_in から何バイト読み取ったか
// str_out : (画面に表示される)文字列
// str_link : リンクの文字列
// ancinfo : ancinfo->anc_from 番から ancinfo->anc_to 番までのアンカーが現れた
//
// 戻り値 : アンカーが現れれば true
//
bool NodeTreeBase::check_anchor( const int mode, const char* str_in,
                                 int& n_in, char* str_out, char* str_link, int lng_link, ANCINFO* ancinfo )
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

            str_link[ 0 ] = *( pos );
            ++str_link;
            --lng_link;

            ++pos;
        }

        // utf-8で"、"
        else if( ( unsigned char )( *pos ) == 0xe3 && ( unsigned char ) ( *( pos + 1 ) ) == 0x80
                 && ( unsigned char ) ( *( pos + 2 ) ) == 0x81 ){

            tmp_out[ lng_out++ ] = static_cast< char >( 0xe3 );
            tmp_out[ lng_out++ ] = static_cast< char >( 0x80 );
            tmp_out[ lng_out++ ] = static_cast< char >( 0x81 );

            str_link[ 0 ] = ',';
            ++str_link;
            --lng_link;

            pos += 3;
        }

        else return false;
    }

    // 数字かチェック
    size_t n, dig;
    int num = MISC::str_to_uint( pos, dig, n );
    if( dig == 0 || dig > MAX_LINK_DIGIT || num == 0 ){

        // モード2で数字が長すぎるときは飛ばす
        if( mode == 2 && dig > MAX_LINK_DIGIT ) n_in = ( int )( pos - str_in ) + n; 

        return false;
    }

    // アンカーが現れたのでとりあえず作成する

    // 画面に表示する文字
    memcpy( str_out, tmp_out, lng_out );
    memcpy( str_out + lng_out, pos, n );
    str_out[ lng_out + n ] = '\0';
    pos += n;
    lng_out += n;    

    // </a>をキャンセル
    if( *( pos ) == '<' && *( pos + 1 ) == '/' && ( *( pos + 2 ) == 'a' || *( pos + 2 ) == 'A' ) && *( pos + 3 ) == '>' ){

        pos += 4;

        // もう一度数字チェック
        // >>1１ を書き込むと <a href="..">&gt;&gt;1</a>１ となるため
        size_t n2, dig2;
        const int num2 = MISC::str_to_uint( pos, dig2, n2 );
        if( dig2 > 0 && dig2 <= MAX_LINK_DIGIT ){

            for( size_t i = 0; i < dig2; ++i ) num *= 10;
            num += num2;

            memcpy( str_out + lng_out, pos, n2 );
            str_out[ lng_out + n2 ] = '\0';
            pos += n2;
            lng_out += n2;
        }
    }

    ancinfo->anc_from = ancinfo->anc_to = num;

    // アンカー文字
    snprintf( str_link, lng_link, "%d", ancinfo->anc_from );

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
        if( dig && dig <= MAX_LINK_DIGIT && ancinfo->anc_to ){

            // 画面に表示する文字            
            memcpy( str_out + lng_out, pos, offset + n );
            str_out[ lng_out + offset + n ] = '\0';

            // アンカー文字をもう一度作成
            snprintf( str_link, lng_link, "%d-%d", ancinfo->anc_from, ancinfo->anc_to );
            pos += offset + n;
            lng_out += offset + n;
        }
    }

    //"&gt;&gt;数字-数字</a>"のパターンの時に</a>をのぞく
    if( *( pos ) == '<' && *( pos + 1 ) == '/' && ( *( pos + 2 ) == 'a' || *( pos + 2 ) == 'A' ) && *( pos + 3 ) == '>' ) pos += 4;
    
    n_in = ( int )( pos - str_in );

    return true;
}



//
// リンクが現れたかチェックして文字列を取得する関数
//
// 入力
// str_in : 入力文字列の先頭アドレス
// lng_str : str_inのバッファサイズ
// lng_link : str_linkのバッファサイズ
// linktype : is_url_scheme()のリタンコード
// delim_pos : is_url_scheme()で得たスキーム文字列の長さ
//
// 出力
// n_in : str_in から何バイト読み取ったか
// str_link : リンクの文字列
//
// 戻り値 : リンクのタイプ(例えばSCHEME_HTTPなど)
//
// 注意 : MISC::is_url_scheme() と MISC::is_url_char() の仕様に合わせる事
//
int NodeTreeBase::check_link_impl( const char* str_in, const int lng_in, int& n_in, char* str_link, const int lng_link, const int linktype, const int delim_pos )
{
    // CONFIG::get_loose_url() == true の時はRFCで規定されていない文字も含める
    const bool loose_url = CONFIG::get_loose_url();

    // リンクの長さを取得
    n_in = delim_pos;
    int n_in_tmp, n_out_tmp;
    char buf[16];

    while( n_in < lng_in ){

        // URLとして扱う文字かどうか
        if ( MISC::is_url_char( str_in + n_in, loose_url ) == false ) break;

        // HTML特殊文字( &〜; )
        if ( *( str_in + n_in ) == '&' &&
             DBTREE::decode_char( str_in + n_in, n_in_tmp, buf, n_out_tmp, false ) != DBTREE::NODE_NONE ){

             // デコード結果が"&(&amp;)"でないもの
             if( n_out_tmp != 1 || buf[0] != '&' ) break;
        }

        n_in++;
    }

    // URLとして短かすぎる場合は除外する( 最短ドメイン名の例 "1.cc" )
    if( n_in - delim_pos < 4 ) return MISC::SCHEME_NONE;

    // URL出力バッファより長いときも除外する( 一般に256バイトを超えるとキャッシュをファイル名として扱えなくなる )
    if( lng_link <= n_in ) return MISC::SCHEME_NONE;

    char *pos = str_link;

    // URLスキームを修正
    int str_pos = 0;
    switch( linktype ){

        // ttp -> http
        case MISC::SCHEME_TTP:

            if( n_in + 1 >= lng_link ) return MISC::SCHEME_NONE;

            *pos = 'h';
            pos++;
            break;

        // tp -> http
        case MISC::SCHEME_TP:

            if( n_in + 2 >= lng_link ) return MISC::SCHEME_NONE;

            *pos  = 'h';
            *(++pos) = 't';
            pos++;
            break;

        // sssp -> http
        case MISC::SCHEME_SSSP:

            *pos = 'h';
            *(++pos) = 't';
            *(++pos) = 't';
            pos++;
            str_pos = 3;
            break;
    }

    // srr_inの文字列をstr_linkにコピー
    int i = str_pos;
    for( ; i < n_in; i++, pos++ ){

        *pos = str_in[ i ];

        // loose_urlで含める"^"と"|"をエンコードする
        // "[]"はダウンローダに渡す用途のためにエンコードしないでおく
        if( loose_url == true ){

            if( str_in[ i ] == '^' ){

                // '^' → "%5E"(+2Byte)
                if( n_in + 2 >= lng_link ) return MISC::SCHEME_NONE;

                *pos = '%';
                *(++pos) = '5';
                *(++pos) = 'E';
            }
            else if( str_in[ i ] == '|' ){

                // '|' → "%7C"(+2Byte)
                if( n_in + 2 >= lng_link ) return MISC::SCHEME_NONE;

                *pos = '%';
                *(++pos) = '7';
                *(++pos) = 'C';
            }
        }
    }

    // str_linkの終端
    *pos = '\0';

#ifdef _DEBUG
    std::cout << str_link << std::endl
              << "len = " << strlen( str_link ) << " lng_link = " << lng_link << " n_in = " << n_in << std::endl;
#endif

    return linktype;
}



// あぼーんしているか
bool NodeTreeBase::get_abone( int number )
{
    NODE* head = res_header( number );
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
                                    const std::vector< char >& vec_abone_res,
                                    const bool abone_transparent, const bool abone_chain, const bool abone_age,
                                    const bool abone_board, const bool abone_global )
{
    m_list_abone_id = list_abone_id;
    m_list_abone_name = list_abone_name;

    m_list_abone_id_board = DBTREE::get_abone_list_id_board( m_url );
    m_list_abone_name_board = DBTREE::get_abone_list_name_board( m_url );

    // 設定ファイルには改行は"\\n"で保存されているので "\n"　に変換する
    m_list_abone_word = MISC::replace_str_list( list_abone_word, "\\n", "\n" );
    m_list_abone_regex = MISC::replace_str_list( list_abone_regex, "\\n", "\n" );

    m_list_abone_word_board = DBTREE::get_abone_list_word_board( m_url );
    m_list_abone_word_board = MISC::replace_str_list( m_list_abone_word_board, "\\n", "\n" );
    m_list_abone_regex_board = DBTREE::get_abone_list_regex_board( m_url );
    m_list_abone_regex_board = MISC::replace_str_list( m_list_abone_regex_board, "\\n", "\n" );

    m_list_abone_word_global = MISC::replace_str_list( CONFIG::get_list_abone_word(), "\\n", "\n" );
    m_list_abone_regex_global = MISC::replace_str_list( CONFIG::get_list_abone_regex(), "\\n", "\n" );

    m_vec_abone_res = vec_abone_res;

    if( CONFIG::get_abone_transparent() ) m_abone_transparent = true;
    else m_abone_transparent = abone_transparent;

    if( CONFIG::get_abone_chain() ) m_abone_chain = true;
    else m_abone_chain = abone_chain;

    m_abone_age = abone_age;
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
    if( ! m_vec_abone_res.size() ) return false;
    if( ! m_vec_abone_res[ number ] ) return false;

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

    if( !check_id && !check_id_board ) return false;

    NODE* head = res_header( number );
    if( ! head ) return false;
    if( ! head->headinfo ) return false;
    if( head->headinfo->abone ) return true;
    if( ! head->headinfo->block[ BLOCK_ID_NAME ] ) return false;

    const int ln_protoid = strlen( PROTO_ID );

    // ローカルID
    if( check_id ){
        std::list< std::string >::iterator it = m_list_abone_id.begin();
        for( ; it != m_list_abone_id.end(); ++it ){

            // std::string の find は遅いのでstrcmp使う
            if( strcmp( head->headinfo->block[ BLOCK_ID_NAME ]->next_node->linkinfo->link + ln_protoid, ( *it ).c_str() ) == 0 ){
                head->headinfo->abone = true;
                return true;
            }
        }
    }

    // 板レベル ID
    if( check_id_board ){
        std::list< std::string >::iterator it = m_list_abone_id_board.begin();
        for( ; it != m_list_abone_id_board.end(); ++it ){

            // std::string の find は遅いのでstrcmp使う
            if( strcmp( head->headinfo->block[ BLOCK_ID_NAME ]->next_node->linkinfo->link + ln_protoid, ( *it ).c_str() ) == 0 ){
                head->headinfo->abone = true;
                return true;
            }
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

    if( !check_name && !check_name_board && !check_name_global ) return false;

    NODE* head = res_header( number );
    if( ! head ) return false;
    if( ! head->headinfo ) return false;
    if( head->headinfo->abone ) return true;
    if( ! head->headinfo->name ) return false;

    std::list< std::string >::const_iterator it;
    const std::string name_str( head->headinfo->name );

    // ローカル name
    if( check_name ){
        it = m_list_abone_name.begin();
        for( ; it != m_list_abone_name.end(); ++it ){
            if( name_str.find( *it ) != std::string::npos ){
                head->headinfo->abone = true;
                return true;
            }
        }
    }

    // 板レベル name
    if( check_name_board ){
        it = m_list_abone_name_board.begin();
        for( ; it != m_list_abone_name_board.end(); ++it ){
            if( name_str.find( *it ) != std::string::npos ){
                head->headinfo->abone = true;
                return true;
            }
        }
    }

    // 全体 name
    if( check_name_global ){
        it = CONFIG::get_list_abone_name().begin();
        for( ; it != CONFIG::get_list_abone_name().end(); ++it ){
            if( name_str.find( *it ) != std::string::npos ){
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
    const bool icase = CONFIG::get_abone_icase();
    const bool newline = true;
    const bool usemigemo = false;
    const bool wchar = CONFIG::get_abone_wchar();

    // ローカル NG word
    if( check_word ){

        std::list< std::string >::iterator it = m_list_abone_word.begin();
        for( ; it != m_list_abone_word.end(); ++it ){
            if( res_str.find( *it ) != std::string::npos ){
                head->headinfo->abone = true;
                return true;
            }
        }
    }

    // ローカル NG regex
    if( check_regex ){

        std::list< std::string >::iterator it = m_list_abone_regex.begin();
        for( ; it != m_list_abone_regex.end(); ++it ){
            if( regex.exec( *it, res_str, offset, icase, newline, usemigemo, wchar ) ){
                head->headinfo->abone = true;
                return true;
            }
        }
    }

    // 板レベル NG word
    if( check_word_board ){

        std::list< std::string >::iterator it = m_list_abone_word_board.begin();
        for( ; it != m_list_abone_word_board.end(); ++it ){
            if( res_str.find( *it ) != std::string::npos ){
                head->headinfo->abone = true;
                return true;
            }
        }
    }

    // 板レベル NG regex
    if( check_regex_board ){

        std::list< std::string >::iterator it = m_list_abone_regex_board.begin();
        for( ; it != m_list_abone_regex_board.end(); ++it ){
            if( regex.exec( *it, res_str, offset, icase, newline, usemigemo, wchar ) ){
                head->headinfo->abone = true;
                return true;
            }
        }
    }

    // 全体 NG word
    if( check_word_global ){

        std::list< std::string >::iterator it = m_list_abone_word_global.begin();
        for( ; it != m_list_abone_word_global.end(); ++it ){
            if( res_str.find( *it ) != std::string::npos ){
                head->headinfo->abone = true;
                return true;
            }
        }
    }

    // 全体 NG regex
    if( check_regex_global ){

        std::list< std::string >::iterator it = m_list_abone_regex_global.begin();
        for( ; it != m_list_abone_regex_global.end(); ++it ){
            if( regex.exec( *it, res_str, offset, icase, newline, usemigemo, wchar ) ){
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

            // アンカーノードの時は node->linkinfo->ancinfo != NULL;
            if( node->type == NODE_LINK && node->linkinfo->ancinfo ){

                int anc = 0;
                int anc_from;
                int anc_to;
                for(;;){

                    anc_from = node->linkinfo->ancinfo[ anc ].anc_from;
                    anc_to = node->linkinfo->ancinfo[ anc ].anc_to;
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
void NodeTreeBase::check_reference( const int number )
{
    NODE* head = res_header( number );
    if( ! head ) return;

    // 既にあぼーんしているならチェックしない
    if( head->headinfo->abone ) return;

    // 2重チェック防止用
    bool checked[ MAX_RESNUMBER ];
    memset( checked, 0, sizeof( bool ) * MAX_RESNUMBER );

    const bool posted = m_vec_posted.size();

    // 過去のレスから number 番へのアンカーがあった場合
    if( m_map_future_refer.size() ){

        std::map< int, std::vector< int > >::iterator it_map = m_map_future_refer.find( number );
        if( it_map != m_map_future_refer.end() ){

            const int size = ( (*it_map).second ).size();

            inc_reference( head, size );

#ifdef _DEBUG
            std::cout << "found number = " << number << " size = " << size << std::endl;
#endif
            // 過去のレスへ自分の書き込みへの参照マークを付ける
            if( posted && m_vec_posted[ number ] ){

                for( int i = 0; i < size; ++ i ){

                    const int from = ( (*it_map).second )[ i ];
#ifdef _DEBUG
                    std::cout << "from " << from << std::endl;
#endif
                    NODE* tmphead = res_header( from );
                    if( tmphead && ! tmphead->headinfo->abone ){
                        if( ! m_vec_refer_posted.size() ) m_vec_refer_posted.resize( MAX_RESNUMBER );
                        m_vec_refer_posted[ from ] = true;
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

                // アンカーノードの時は node->linkinfo->ancinfo != NULL;
                if( node->linkinfo->ancinfo ){

                    int anc = 0;
                    int anc_from;
                    int anc_to;
                    for(;;){

                        anc_from = node->linkinfo->ancinfo[ anc ].anc_from;
                        anc_to = node->linkinfo->ancinfo[ anc ].anc_to;
                        if( anc_from == 0 ) break;
                        ++anc;

                        anc_to = MIN( anc_to, MAX_RESNUMBER );

                        // >>1-1000 みたいなアンカーは弾く
                        if( anc_to - anc_from >= RANGE_REF ) continue;

                        for( int i = anc_from; i <= anc_to ; ++i ){

                            // 既にチェックしている
                            if( checked[ i ] ) continue;

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

                                checked[ i ] = true;

                                // 自分の書き込みに対するレス
                                if( posted && m_vec_posted[ i ] ){
                                    if( ! m_vec_refer_posted.size() ) m_vec_refer_posted.resize( MAX_RESNUMBER );
                                    m_vec_refer_posted[ number ] = true;

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


// 発言数(( num_id_name ))とIDの色のクリア
void NodeTreeBase::clear_id_name()
{
    for( int i = 1; i <= m_id_header; ++i ){
        NODE* tmphead = m_vec_header[ i ];
        if( tmphead && tmphead->headinfo && tmphead->headinfo->block[ BLOCK_ID_NAME ] ){
            tmphead->headinfo->num_id_name = 0;
            tmphead->headinfo->block[ BLOCK_ID_NAME ]->next_node->color_text = COLOR_CHAR;
        }
    }
}



//
// from_number番から to_number 番までの発言数の更新
//
void NodeTreeBase::update_id_name( const int from_number, const int to_number )
{
    if( ! CONFIG::get_check_id() ) return;

    if( empty() ) return;
    if( to_number < from_number ) return;
    for( int i = from_number ; i <= to_number; ++i ) check_id_name( i );
}



//
// number番のレスの発言数を更新
//
void NodeTreeBase::check_id_name( const int number )
{
    NODE* header = res_header( number );
    if( ! header ) return;
//    if( header->headinfo->abone ) return;
    if( ! header->headinfo->block[ BLOCK_ID_NAME ] ) return;

    const char* str_id = header->headinfo->block[ BLOCK_ID_NAME ]->next_node->linkinfo->link;

    // 同じIDのレスを持つ一つ前のレスを探す
    NODE* tmphead;
    NODE* prehead = NULL;
    for( int i = header->id_header -1 ; i >= 1 ; --i ){

        tmphead = m_vec_header[ i ];

        if( tmphead
//            && ! tmphead->headinfo->abone // 対象スレがあぼーんしていたらカウントしない
            && tmphead->headinfo->block[ BLOCK_ID_NAME ]
            && str_id[ 0 ] == tmphead->headinfo->block[ BLOCK_ID_NAME ]->next_node->linkinfo->link[ 0 ]
            && strcmp( str_id, tmphead->headinfo->block[ BLOCK_ID_NAME ]->next_node->linkinfo->link ) == 0 ){
            prehead = tmphead;
            break;
        }
    }

    // 見つからなかった
    if( ! prehead ) set_num_id_name( header, 1, NULL );

    // 見つかった
    else{

        set_num_id_name( header, prehead->headinfo->num_id_name+1, prehead );

        // 以前に出た同じIDのレスの発言数を更新
        tmphead = prehead;
        while( tmphead ){

            set_num_id_name( tmphead, tmphead->headinfo->num_id_name+1, tmphead->headinfo->pre_id_name_header );
            tmphead = tmphead->headinfo->pre_id_name_header;
        }
    }
}


//
// 発言数( num_id_name )の更新
//
// IDノードの色も変更する
//
void NodeTreeBase::set_num_id_name( NODE* header, const int num_id_name, NODE* pre_id_name_header )
{
    if( ! header->headinfo->block[ BLOCK_ID_NAME ] ) return;

    header->headinfo->num_id_name = num_id_name;        
    header->headinfo->pre_id_name_header = pre_id_name_header;

    if( num_id_name >= m_num_id[ LINK_HIGH ] ) header->headinfo->block[ BLOCK_ID_NAME ]->next_node->color_text = COLOR_CHAR_LINK_ID_HIGH;
    else if( num_id_name >= m_num_id[ LINK_LOW ] ) header->headinfo->block[ BLOCK_ID_NAME ]->next_node->color_text = COLOR_CHAR_LINK_ID_LOW;
    else header->headinfo->block[ BLOCK_ID_NAME ]->next_node->color_text = COLOR_CHAR;
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
void NodeTreeBase::check_fontid( const int number )
{
    NODE* head = res_header( number );
    if( ! head ) return;
    if( ! head->headinfo ) return;
    if( head->fontid != FONT_EMPTY ) return;

    // ヘッダノードには、フォント判定済みの意味を兼ねて、デフォルトフォントを設定しておく
    head->fontid = FONT_DEFAULT;
    if( m_aa_regex.empty() ) return;

    char fontid_mes = FONT_DEFAULT; // 本文のフォント(fontid.h)

    // AAフォント判定
    const std::string res_str = get_res_str( number );
    JDLIB::Regex regex;
    const size_t offset = 0;
    const bool icase = false;
    const bool newline = true;
    const bool usemigemo = false;
    const bool wchar = false;

    if( regex.exec( m_aa_regex, res_str, offset, icase, newline, usemigemo, wchar ) ){
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




//
// http://ime.nu/ などをリンクから削除
//
// 取り除いたらtrueを返す
//
// Thanks to 「パッチ投稿」スレの24氏
//
// http://jd4linux.sourceforge.jp/cgi-bin/bbs/test/read.cgi/support/1151836078/24
//
bool NodeTreeBase::remove_imenu( char* str_link )
{
    char *p = str_link;

    if ( memcmp( p, "http", strlen( "http" ) ) != 0 ) return false;
    p += strlen( "http" );
    if ( *p == 's' ) p++;
    if ( memcmp( p, "://", strlen( "://" ) ) != 0 ) return false;
    p += strlen( "://" );

    const char *cut_sites[] = { "ime.nu/", "ime.st/", "nun.nu/", "pinktower.com/", 0 };
    const char **q = cut_sites;
    while ( *q ) {
        size_t cs_len = strlen( *q );
        if ( memcmp( p, *q, cs_len ) == 0 ) {
            // "http://ime.nu/"等、URLがそれだけだった場合は削除しない
            if ( p[cs_len] == '\0' ) return false;
            memmove( p, p + cs_len, strlen( p + cs_len ) + 1 );
            return true;
        }
        q ++;
    }
    return false;
}


// 文字列中の"&amp;"を"&"に変換する
int NodeTreeBase::convert_amp( char* text, const int n )
{
    int m = n;

    int i;
    for( i = 0; i < m; i++ ){

        if( text[ i ] == '&' &&
            m > (i + 4) &&
            text[i + 1] == 'a' &&
            text[i + 2] == 'm' &&
            text[i + 3] == 'p' &&
            text[i + 4] == ';' ){

            // &の次, &amp;の次, &amp;の次からの長さ
            memmove( text + i + 1, text + i + 5, n - i - 5 );

            // "amp;"の分減らす
            m -= 4;
        }
    }

    text[m] = '\0';
    return m;
}


// 自分の書き込みにレスしたか
bool NodeTreeBase::is_refer_posted( const int number )
{
    if( ! m_vec_refer_posted.size() ) return false;
    if( m_vec_refer_posted.size() <= ( size_t )number ) return false;

    return m_vec_refer_posted[ number ];
}


// 書き込みマークセット
void NodeTreeBase::set_posted( const int number, const bool set )
{
    if( ! m_vec_posted.size() ) m_vec_posted.resize( MAX_RESNUMBER );
    if( ! m_vec_refer_posted.size() ) m_vec_refer_posted.resize( MAX_RESNUMBER );

    m_vec_posted[ number ] = set;

    // 自分の書き込みに対するレス
    std::list< int > res_num = get_res_reference( number );
    std::list< int >::const_iterator it_res = res_num.begin();

    // レスされたマークを設定する
    if( set ){
        for( ; it_res != res_num.end(); ++it_res ){
            const int n = ( *it_res );
            m_vec_refer_posted[ n ] = true;
        }
    }
    
    // レスされてなくなったので、マークを解除する
    else{
        for( ; it_res != res_num.end(); ++it_res ){
            const int n = ( *it_res );

            // レスアンカーのリストを取得
            std::list< ANCINFO* > anchors = get_res_anchors( n );
            std::list< ANCINFO* >::const_iterator it_anchor = anchors.begin();
            for( ; it_anchor != anchors.end(); ++it_anchor ){

                // 他の自分の書き込みに対するレスになっていないか？
                ANCINFO* anchor = ( *it_anchor );
                for( int i = anchor->anc_from; i <= anchor->anc_to; i++ ){
                    // 他の自分の書き込みに対するレス
                    if( m_vec_posted[ i ] ) goto KEEP_POSTMARK;
                }
            }

            // マークを解除する
            m_vec_refer_posted[ n ] = false;
KEEP_POSTMARK:;
        }
    }
}


// 書き込み履歴のリセット
void NodeTreeBase::clear_post_history()
{
    m_vec_posted.clear();
    m_vec_refer_posted.clear();
}
