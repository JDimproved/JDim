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

#include "config/globalconf.h"

#include "global.h"
#include "httpcode.h"
#include "colorid.h"
#include "command.h"
#include "cache.h"
#include "session.h"

#include <sstream>
#include <fstream>


#ifndef MAX
#define MAX( a, b ) ( a > b ? a : b )
#endif


#ifndef MIN
#define MIN( a, b ) ( a < b ? a : b )
#endif


#define SECTION_NUM 5
#define LNG_RES 16
#define LNG_ID 64
#define LNG_LINK 256
#define MAX_ANCINFO 64
#define RANGE_REF 20
#define MAX_LINK_DIGIT 4  // レスアンカーでMAX_LINK_DIGIT 桁までリンクにする

#define MAXSISE_OF_LINES ( 512 * 1024 )   // ロード時に１回の呼び出しで読み込まれる最大データサイズ
#define SIZE_OF_HEAP ( MAXSISE_OF_LINES + 64 )

using namespace DBTREE;


NodeTreeBase::NodeTreeBase( const std::string url, const std::string& modified )
    : SKELETON::Loadable(),
      m_url( url ),
      m_lng_dat( 0 ),
      m_resume ( 0 ),
      m_broken( 0 ),
      m_heap( SIZE_OF_HEAP ),
      m_buffer_lines( 0 ),
      m_parsed_text( 0 ),
      m_fout ( 0 )
{
    set_date_modified( modified );

    clear();

    // ヘッダのポインタの配列作成
    m_vec_header = ( NODE** ) m_heap.heap_alloc( sizeof( NODE* ) * MAX_RESNUMBER );

    // ルートヘッダ作成。中は空。
    m_id_header = -1; // ルートヘッダIDが 0 になるように -1
    NODE* tmpnode = create_header_node(); 
    m_vec_header[ m_id_header ] = tmpnode;

    m_default_noname = DBTREE::default_noname( m_url );

#ifdef _DEBUG
    std::cout << "NodeTreeBase::NodeTreeBase url = " << m_url << " modified = " << date_modified()
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
    if( m_buffer_lines ) free( m_buffer_lines );
    m_buffer_lines = NULL;
    m_byte_buffer_lines_left = 0;

    if( m_parsed_text ) free( m_parsed_text );
    m_parsed_text = NULL;
    
    if( m_fout ) fclose( m_fout );
    m_fout = NULL;

    m_ext_err = std::string();
}


//
// 総レス数
//
// ロード中は m_id_header 番のレスはまだ処理中なので m_id_header -1 を返す
//
const int NodeTreeBase::get_res_number()
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
// 下のnum_id_name( int number )と違って検索するので遅い
//
int NodeTreeBase::get_num_id_name( const std::string& id )
{
    for( int i = 1; i <= m_id_header; ++i ){

        if( get_id_name( i ) == id ) return get_num_id_name( i );
    }

    return 0;
}


//
// number番の ID の重複数
//
int NodeTreeBase::get_num_id_name( int number )
{
    NODE* head = res_header( number );
    if( ! head ) return 0;

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

                    if( node->type == NODE_LINK
                        && node->linkinfo->link
                        && (
                            std::string( node->linkinfo->link ).find( "http" ) == 0
                            || std::string( node->linkinfo->link ).find( "https" ) == 0
                            || std::string( node->linkinfo->link ).find( "ftp" ) == 0
                            )
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
// number番のレスを参照しているレスの番号全てをリストにして取得
//
std::list< int > NodeTreeBase::get_res_reference( int number )
{
    std::list< int > list_resnum;
    for( int i = number + 1; i <= m_id_header; ++i ){

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

                                // >>1-1000 みたいなアンカーは弾く
                                if( anc_to - anc_from < RANGE_REF && anc_from <= number && number <= anc_to
                                    && ( ! m_abone_transparent || ! get_abone( i ) ) //  透明あぼーんしていない　or あぼーんしていない
                                    ) {
                                    list_resnum.push_back( i );
                                    block = BLOCK_NUM;
                                    break;
                                }
                            }
                            if( anc_from != 0 ) break;
                        }
                    }

                    node = node->next_node;

                } // while( node )

            } // for( block )

        }

    }

#ifdef _DEBUG
    std::cout << "NodeTreeBase::get_reference\n";
    std::list < int >::iterator it;
    for( it = list_resnum.begin(); it != list_resnum.end(); ++it ) std::cout << *it << std::endl;
#endif
    
    return list_resnum;
}
        

//
// query を含むレス番号をリストにして取得
//
// mode_or == true なら OR抽出
//
std::list< int > NodeTreeBase::get_res_query( const std::string& query, bool mode_or )
{
    std::list< int > list_resnum;
    if( query.empty() ) return list_resnum;

    JDLIB::Regex regex;
    std::list< std::string > list_query = MISC::split_line( query );

    for( int i = 1; i <= m_id_header ; ++i ){

        std::string res_str = get_res_str( i );

        bool apnd = true;
        if( mode_or ) apnd = false;
        std::list< std::string >::iterator it = list_query.begin();
        for( ; it != list_query.end(); ++it ){

            bool ret = regex.exec( ( *it ), res_str, 0, true );

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

    str_res += ref_prefix;

    NODE* node;
    for( int block = 0; block < BLOCK_MES; ++ block )  GETNODESTR( block );
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
// number番のレスの時刻
// 内部で regex　を使っているので遅い
//
const std::string NodeTreeBase::get_time( int number )
{
    std::string res_str = get_res_str( number );
    if( res_str.empty() ) return std::string();

    std::string time_str;
    JDLIB::Regex regex;

    if( regex.exec( " 名前：.+]：([0-9]*/[0-9]*/[0-9]*[^ ]* [0-9]*:[0-9]*[^ ]*).*$", res_str ) ){
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
NODE* NodeTreeBase::createNode()
{
    NODE* tmpnode = ( NODE* ) m_heap.heap_alloc( sizeof( NODE ) );

    tmpnode->id_header = m_id_header;
    if( m_node_previous ) m_node_previous->next_node = tmpnode;
    m_node_previous = tmpnode;
    
    return tmpnode;
}


//
// ヘッダノード作成
//
NODE* NodeTreeBase::create_header_node()
{
    ++m_id_header;
    m_node_previous = NULL;

    NODE* tmpnode = createNode();
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
NODE* NodeTreeBase::create_block_node()
{
    m_node_previous = NULL;

    NODE* tmpnode = createNode();
    tmpnode->type =  NODE_BLOCK;
    
    return tmpnode;
}


//
// 発言回数(IDの出現数)ノード
//
NODE* NodeTreeBase::createIDNumNode()
{
    const char* dummy = " (10000)";

    NODE* tmpnode = createTextNode( dummy, COLOR_CHAR );
    tmpnode->type = NODE_IDNUM;
    tmpnode->text[ 0 ] = '\0'; // メモリだけ確保して文字を消す
    return tmpnode;
}


//
// 改行ノード作成
//
NODE* NodeTreeBase::createBrNode()
{
    NODE* tmpnode = createNode();
    tmpnode->type = NODE_BR;
    return tmpnode;
}


//
// スペースノード
//
NODE* NodeTreeBase::createSpNode( const int& type )
{
    NODE* tmpnode = createNode();
    tmpnode->type = type;
    return tmpnode;
}


//
// 水平タブノード
//
NODE* NodeTreeBase::createHTabNode()
{
    NODE* tmpnode = createNode();
    tmpnode->type = NODE_HTAB;
    return tmpnode;
}


//
// リンクノード作成
//
// bold : 太字か
//
NODE* NodeTreeBase::create_linknode( const char* text, int n, const char* link, int n_link, int color_text, bool bold )
{
    NODE* tmpnode = createTextNodeN( text, n, color_text, bold );
    if( tmpnode ){
        tmpnode->type = NODE_LINK;
        
        // リンク情報セット
        tmpnode->linkinfo = ( LINKINFO* )m_heap.heap_alloc( sizeof( LINKINFO ) );

        tmpnode->linkinfo->link = ( char* )m_heap.heap_alloc( n_link +4 );
        memcpy( tmpnode->linkinfo->link, link, n_link ); tmpnode->linkinfo->link[ n_link ] = '\0';
    }
    
    return tmpnode;
}


//
// アンカーノード作成
//
NODE* NodeTreeBase::create_ancnode( const char* text, int n, const char* link, int n_link, int color_text,  bool bold,
                                    ANCINFO* ancinfo, int lng_ancinfo )
{
    NODE* tmpnode = create_linknode( text, n, link, n_link, color_text, bold );
    if( tmpnode ){

        tmpnode->linkinfo->ancinfo = ( ANCINFO* )m_heap.heap_alloc( sizeof( ANCINFO ) * ( lng_ancinfo + 1 ) );
        memcpy( tmpnode->linkinfo->ancinfo, ancinfo, sizeof( ANCINFO ) * lng_ancinfo );
    }
    
    return tmpnode;
}


//
// 画像ノード作成
//
NODE* NodeTreeBase::create_imgnode( const char* text, int n, const char* link, int n_link, int color_text,  bool bold )
{
    NODE* tmpnode = create_linknode( text, n, link, n_link, color_text, bold );
    if( tmpnode ) tmpnode->linkinfo->image = true;
    
    return tmpnode;
}


//
// テキストノード作成
//
NODE* NodeTreeBase::createTextNode( const char* text, int color_text, bool bold )
{
    return createTextNodeN( text, strlen( text ), color_text, bold );
}



//
// テキストノード作成( サイズ指定 )
//
NODE* NodeTreeBase::createTextNodeN( const char* text, int n, int color_text, bool bold )
{
    if( n <= 0 ) return NULL;
    
    NODE* tmpnode = createNode();

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

    NODE* header = create_header_node();
    m_vec_header[ m_id_header ] = header;

    init_loading();
    header->headinfo->block[ BLOCK_MES ] = create_block_node();
    parse_html( html.c_str(), html.length(), COLOR_CHAR, false, false, true );
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
            init_loading();
            while( size < str.length() ){
                size_t size_tmp = MIN( MAXSISE_OF_LINES, str.length() - size );
                receive_data( data + size, size_tmp );
                size += size_tmp;
            }
            receive_finish();
        }
    }
}


//
// ロード実行前に呼ぶ初期化関数
//
// charcode は入力データの文字コード( empty()ならコード変換しない)
//
void NodeTreeBase::init_loading()
{
    clear();

    // 一時バッファ作成
    if( ! m_buffer_lines ) m_buffer_lines = ( char* ) malloc( MAXSISE_OF_LINES ); 
    if( ! m_parsed_text ) m_parsed_text = ( char* ) malloc( MAXSISE_OF_LINES );
}



//
// ロード開始
//
void NodeTreeBase::download_dat()
{
#ifdef _DEBUG    
    std::cout << "NodeTreeBase::download_dat : " << m_url << std::endl;
#endif

    if( is_loading() ) return;

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

    // 保存ディレクトリ作成(無ければ)
    if( CACHE::mkdir_boardroot( m_url ) ){
    
        // 保存ファイルオープン
        std::string path_cache = CACHE::path_dat( m_url );

#ifdef _DEBUG
        std::cout << "open " << path_cache.c_str() << std::endl;
#endif

        m_fout = fopen( path_cache.c_str(), "ab" );
        if( m_fout == NULL ){
            MISC::ERRMSG( "fopen failed : " + path_cache );
        }
    }
    else{
        MISC::ERRMSG( "could not create " + DBTREE::url_boardbase( m_url ) );
    }

    // ロード開始
    // ロード完了したら receive_finish() が呼ばれる
    JDLIB::LOADERDATA data;
    create_loaderdata( data );
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
#endif

        return;
    }

    // レジューム処理
    // レジュームした時に先頭が '\n' かチェック
    // '\n'で無ければあぼーんがあったということ
    if( m_resume ){

        if( data[ 0 ] != '\n' ){
            m_broken = true;
            MISC::ERRMSG( "failed to resume" );
        }
        else{
            ++data;
            --size;
        }

        m_resume = false;
    }

    if( !size ) return;
    
    // バッファが '\n' で終わるように調整
    const char* pos = data + size -1;
    while( *pos != '\n' && pos != data ) --pos;

    // 前回の残りのデータに新しいデータを付け足して add_raw_lines()にデータを送る
    size_t size_in = ( int )( pos - data ) + 1;
    if( size_in > 1 ){
        memcpy( m_buffer_lines + m_byte_buffer_lines_left , data, size_in );
        m_buffer_lines[ m_byte_buffer_lines_left + size_in ] = '\0';
        add_raw_lines( m_buffer_lines, m_byte_buffer_lines_left + size_in );
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
    if( get_code() != HTTP_INIT
        && get_code() != HTTP_OK
        && get_code() != HTTP_PARTIAL_CONTENT
        && get_code() != HTTP_NOT_MODIFIED
        ){
        std::ostringstream err;
        err << m_url << std::endl
            << "load failed. : " << get_str_code();
        if( get_code() == HTTP_REDIRECT ) err << " location = " << location();
        MISC::ERRMSG( err.str() );
    }

    // Requested Range Not Satisfiable
    if( get_code() == HTTP_RANGE_ERR ) m_broken = true;

    // データがロードされなかったらキャッシュを消す
    if( get_res_number() == 0 ){

        std::string path = CACHE::path_dat( m_url );
        if( CACHE::file_exists( path ) == CACHE::EXIST_FILE ) unlink( path.c_str() );
        set_date_modified( std::string() );
    }

    // その他、何かエラーがあったらmodifiedをクリアしておく
    if( !get_ext_err().empty() ) set_date_modified( std::string() );

#ifdef _DEBUG
    std::cout << "NodeTreeBase::receive_finish lng = " << m_lng_dat << " code = " << get_code() << " " << get_str_code() << std::endl;
#endif    
    
    // 親 article クラスにシグナルを打ってツリー構造が変わったことを教える
    m_sig_finished.emit();

    clear();
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
            m_broken = true;
            rawlines[ i ] = ' ';
        } 
    }

    // 保存前にrawデータを加工
    rawlines = process_raw_lines( rawlines );

    // キャッシュに保存
    size_t lng = strlen( rawlines );
    if( !lng ) return;
    m_lng_dat += lng;
    if( m_fout ){
#ifdef _DEBUG
        std::cout << "NodeTreeBase::add_raw_lines save " << lng << " bytes\n";
#endif       
        if( fwrite( rawlines, 1, lng, m_fout ) < lng ){
            MISC::ERRMSG( "write failed in NodeTreeBase::add_raw_lines\n" );
        }
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

    int i;
    NODE* header = create_header_node();
    m_vec_header[ m_id_header ] =  header;

    // レス番号
    char tmplink[ LNG_RES ], tmpstr[ LNG_RES ];
    snprintf( tmpstr, LNG_RES, "%d", header->id_header );
    snprintf( tmplink, LNG_RES,"%s%d", PROTO_RES, header->id_header );    

    header->headinfo->block[ BLOCK_NUMBER ] = create_block_node();
    create_linknode( tmpstr, strlen( tmpstr ) , tmplink, strlen( tmplink ), COLOR_CHAR_LINK, true );

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
        header->headinfo->block[ BLOCK_NAME ] = create_block_node();

        createTextNode( " 壊れています", COLOR_CHAR );

        header->headinfo->block[ BLOCK_MES ] = create_block_node();
        const char str_broken[] = "ここ";
        create_linknode( str_broken, strlen( str_broken ) , PROTO_BROKEN, strlen( PROTO_BROKEN ), COLOR_CHAR_LINK, false );
        createTextNode( "をクリックしてスレを再読み込みして下さい。", COLOR_CHAR );
        
        return pos;
    }
    
    // 名前
    parseName( header, section[ 0 ], section_lng[ 0 ] );
    
    // メール
    parseMail( header, section[ 1 ], section_lng[ 1 ] );

    // 日付とID
    parse_date_id( header, section[ 2 ], section_lng[ 2 ] );

    // 本文
    header->headinfo->block[ BLOCK_MES ] = create_block_node();
    parse_html( section[ 3 ], section_lng[ 3 ], COLOR_CHAR );

    // サブジェクト
    if( header->id_header == 1 ){
        m_subject = std::string( section[ 4 ] ).substr( 0, section_lng[ 4 ] );

#ifdef _DEBUG
        std::cout << "subject = " << m_subject << std::endl;
#endif
    }

    return pos;
}


//
// 名前
//
void NodeTreeBase::parseName( NODE* header, const char* str, int lng )
{
    bool digitlink = true;
    const bool bold = true;
    int pos_trip_begin = 0;
    int pos_trip_end = 0;
    int i;
    const bool defaultname = ( strncmp( m_default_noname.data(), str, lng ) == 0 );

    header->headinfo->block[ BLOCK_NAMELINK ] = create_block_node();

    // デフォルトの名前で無いときはリンクにする
    if( defaultname ) createTextNode( "名前", COLOR_CHAR );
    else{
        const char namestr[] = "名前";
        create_linknode( namestr, strlen( namestr ) , PROTO_NAME, strlen( PROTO_NAME ), COLOR_CHAR, false );
    }

    NODE* node = header->headinfo->block[ BLOCK_NAME ] = create_block_node();

    // トリップなど</b>〜<b>が含まれている場合。</b>〜<b>の中の数字はリンクにしない
    for( i = 0; i < lng; ++i ) if( str[ i ] == '<' && ( str[ i+2 ] == 'b' || str[ i+2 ] == 'B' ) ) break;
    if( i != lng ){

        pos_trip_begin = i;
        pos_trip_end = lng -1;
        for( i = pos_trip_begin + 4; i < lng; ++i ){
                if( str[ i ] == '<' && ( str[ i+1 ] == 'b' || str[ i+1 ] == 'B' ) ){
                pos_trip_end = i + 2;
                break;
            }
        }

#ifdef _DEBUG        
        char tmp_str[256];
        memset( tmp_str, 0, 256);
        memcpy(tmp_str, str, lng);

        std::cout << "NodeTreeBase::parseName trip = " << tmp_str
                  << " begin = " << pos_trip_begin << " end = " << pos_trip_end << std::endl;
#endif
        // トリップの前(名前部分)
        parse_html( str, pos_trip_begin, COLOR_CHAR_NAME, digitlink, bold );

        // あとは数字が入ってもリンクしない
        digitlink = false;

        // トリップ
        parse_html( str + pos_trip_begin, pos_trip_end - pos_trip_begin + 1, COLOR_CHAR_NAME_B, digitlink, bold );

        // トリップの後
        if( pos_trip_end < lng-1 )
            parse_html( str + pos_trip_end + 1, lng - pos_trip_end - 1, COLOR_CHAR_NAME, digitlink, bold );
    }

    // デフォルト名無しと同じときはアンカーを作らない
    else if( defaultname ){
        digitlink = false;
        parse_html( str, lng, COLOR_CHAR_NAME, digitlink, bold );
    }

    // 通常の場合は先頭に数字があったらアンカーにする
    else parse_html( str, lng, COLOR_CHAR_NAME, digitlink, bold );

    // plainな名前取得
    std::string str_tmp;
    node = node->next_node;
    while( node ){
        if( node->text ) str_tmp += node->text;
        node = node->next_node;
    }
    header->headinfo->name = ( char* )m_heap.heap_alloc( str_tmp.length() +2 );
    memcpy( header->headinfo->name, str_tmp.c_str(), str_tmp.length() );
}


//
// メール
//
void NodeTreeBase::parseMail( NODE* header, const char* str, int lng )
{
    // sage 以外の時は色を変える
    int color = COLOR_CHAR;
    int i = 0;
    while( str[ i ] != 's' && i < lng ) ++i;
    if( str[ i ] != 's' || str[ i+1 ] != 'a' || str[ i+2 ] != 'g' || str[ i+3 ] != 'e' ) color = COLOR_CHAR_AGE;

    header->headinfo->block[ BLOCK_MAIL ] = create_block_node();
    createTextNode( "[", color );
    parse_html( str, lng, color, true );

    if( color == COLOR_CHAR ) createTextNode( "]", color );
    else createTextNode( "]", color );
}


//
// 日付とID、及びBE、株、その他
//
void NodeTreeBase::parse_date_id( NODE* header, const char* str, int lng )
{
    int start = 0;
    int lng_text = 0;
    char tmplink[ LNG_LINK ];

    int lng_id_tmp;
    char tmpid[ LNG_ID ];

    header->headinfo->block[ BLOCK_DATE ] = create_block_node();

    for(;;){

        // 先頭の空白を飛ばす
        while( start + lng_text < lng && str[ start + lng_text ] == ' ' ) ++lng_text;

        // 空白ごとにブロック分けしてパースする
        int start_block = start + lng_text;
        int lng_block = 0; // ブロックの長さ
        while( start_block + lng_block < lng && str[ start_block + lng_block ] != ' ' ) ++lng_block;
        if( !lng_block ) break;

        // ID ( ??? の時は除く )
        if( str[ start_block ] == 'I' && str[ start_block + 1 ] == 'D' && str[ start_block + 3 ] != '?' ){

            // フラッシュ
            if( lng_text ){
                if( *( str + start + lng_text - 1 ) == ' ' ) --lng_text;
                createTextNodeN( str + start, lng_text, COLOR_CHAR );
            }

            // id 取得
            lng_id_tmp = lng_block -3;
            memcpy( tmpid, str + start_block + 3, lng_id_tmp );
            tmpid[ lng_id_tmp ] = '\0';
            
            // リンク文字作成
            memcpy( tmplink, PROTO_ID, sizeof( PROTO_ID ) );
            memcpy( tmplink + sizeof( PROTO_ID ) - 1, tmpid, lng_id_tmp + 1 );
            
            // リンク作成
            header->headinfo->block[ BLOCK_ID_NAME ] = create_block_node();
            create_linknode( "ID:", 3 , tmplink, strlen( tmplink ), COLOR_CHAR, false );
            createTextNodeN( tmpid, lng_id_tmp, COLOR_CHAR);

            // 発言回数ノード作成
            createIDNumNode();

            // 次のブロックへ移動
            start = start_block + lng_block;
            lng_text = 0;
        }

        // BE:
        else if( str[ start_block ] == 'B' && str[ start_block + 1 ] == 'E' ){

            // フラッシュ
            if( lng_text ) createTextNodeN( str + start, lng_text, COLOR_CHAR );

            // id 取得
            lng_id_tmp = 0;
            while( str[ start_block + lng_id_tmp ] != '-' && lng_id_tmp < lng_block ) ++lng_id_tmp;
            lng_id_tmp -= 3;
            memcpy( tmpid, str + start_block + 3, lng_id_tmp );
            tmpid[ lng_id_tmp ] = '\0';

            // リンク文字作成
            memcpy( tmplink, PROTO_BE, sizeof( PROTO_BE ) );
            memcpy( tmplink + sizeof( PROTO_BE ) -1, tmpid, lng_id_tmp + 1 );

            // リンク作成
            create_linknode( "BE:", 3 , tmplink, strlen( tmplink ), COLOR_CHAR, false );
            createTextNodeN( tmpid, lng_id_tmp, COLOR_CHAR);

            // 次のブロックへ移動
            start = start_block + lng_block;
            lng_text = 0;
        }

        // 株などの<a href～>
        else if( str[ start_block ] == '<'
                 && ( str[ start_block + 1 ] == 'a' || str[ start_block + 1 ] == 'A' )
                 && str[ start_block + 2 ] == ' ' ){

            // フラッシュ
            if( lng_text ) createTextNodeN( str + start, lng_text, COLOR_CHAR );

            // </a>までブロックの長さを伸ばす
            while( start_block + lng_block < lng
                   && ! ( ( str[ start_block + lng_block -1 ] == 'a' || str[ start_block + lng_block -1 ] == 'A' )
                          && str[ start_block + lng_block ] == '>' ) ) ++lng_block;
            ++lng_block;

            parse_html( str + start_block, lng_block, COLOR_CHAR, false, false, true );

            // 次のブロックへ移動
            start = start_block + lng_block;
            lng_text = 0;
        }

        // テキスト(日付含む)
        else lng_text += lng_block;
    }

    // フラッシュ
    if( lng_text ) createTextNodeN( str + start, lng_text, COLOR_CHAR );
}


//
// HTMLパーサ
//
// digitlink : true の時は先頭に数字が現れたらアンカーにする( parseName() などで使う )
//             false なら数字の前に >> がついてるときだけアンカーにする
//
// bold : ボールド標示
//
// ahref : <a href=～></a> からリンクノードを作成する
// (例) parse_html( "<a href=\"hoge.com\">hoge</a>", 27, COLOR_CHAR, false, false, true );
//
// (パッチ)
//
// 行頭の空白は全て除くパッチ
// Thanks to 「パッチ投稿スレ」の28氏
// http://jd4linux.sourceforge.jp/cgi-bin/bbs/test/read.cgi/support/1151836078/28
//
void NodeTreeBase::parse_html( const char* str, int lng, int color_text, bool digitlink, bool bold, bool ahref )
{
    const char* pos = str;
    const char* pos_end = str + lng;
    int lng_text = 0;
    
    // 行頭の空白は全て除く
    while( *pos == ' ' ) ++pos;
   
    for( ; pos < pos_end; ++pos, digitlink = false ){


        ///////////////////////
        // HTMLタグ
        if( *pos == '<' ){ 

            // 改行 <br>
            if( ( *( pos + 1 ) == 'b' || *( pos + 1 ) == 'B' )
                &&  ( *( pos + 2 ) == 'r' || *( pos + 2 ) == 'R' ) ){  

                // フラッシュ
                if( *( pos - 1 ) == ' ' ) --lng_text; // 改行前の空白を取り除く
                createTextNodeN( m_parsed_text, lng_text, color_text, bold ); lng_text = 0;

                pos += 4;
                
                // 改行ノード作成
                createBrNode();

                // 改行直後と行頭の空白は全て除く
                while( *pos == ' ' ) ++pos;
            }

            //  ahref == true かつ <a href=～></a>
            else if( ahref &&
                     ( *( pos + 1 ) == 'a' || *( pos + 1 ) == 'A' ) && *( pos + 2 ) == ' ' ){

                // フラッシュ
                createTextNodeN( m_parsed_text, lng_text, color_text, bold ); lng_text = 0;

                while( pos < pos_end && *pos != '"' ) ++pos;
                if( pos >= pos_end ) continue;
                ++pos;

                const char* pos_link_start = pos;
                int lng_link = 0;

                while( pos < pos_end && *pos != '"' ){ ++pos; ++lng_link; }
                if( pos >= pos_end ) continue;

                while( pos < pos_end && *pos != '>' ) ++pos;
                if( pos >= pos_end ) continue;
                ++pos;

                const char* pos_str_start = pos;
                int lng_str = 0;

                while( pos < pos_end && *pos != '<' ){ ++pos; ++lng_str; }
                if( pos >= pos_end ) continue;

                while( pos < pos_end && *pos != '>' ) ++pos;
                if( pos >= pos_end ) continue;
                ++pos;

                if( lng_link && lng_str ){
                    create_linknode( pos_str_start, lng_str , pos_link_start, lng_link, COLOR_CHAR, false );
                }
            }

            // </a>
            else if( *( pos + 1 ) == '/' && ( *( pos + 2 ) == 'a' || *( pos + 2 ) == 'A' ) && *( pos + 3 ) == '>' ) pos += 4;

            // その他のタグは無視。タグを取り除いて中身だけを見る
            else {

                // フラッシュ
                createTextNodeN( m_parsed_text, lng_text, color_text, bold ); lng_text = 0;
                
                while( pos < pos_end && *pos != '>' ) ++pos;
                ++pos;
            }

            // forのところで++されるので--しておく
            --pos;
            continue;
        }


        ///////////////////////
        // アンカーのチェック
        int n_in = 0;
        int n_out = 0;
        char tmpstr[ LNG_LINK ], tmplink[ LNG_LINK ];
        int lng_str = 0, lng_link = strlen( PROTO_ANCHORE );
        ANCINFO ancinfo[ MAX_ANCINFO ];
        int lng_anc = 0;

        int mode = 0;
        if( digitlink ) mode = 2;

        if( check_anchor( mode , pos, n_in, tmpstr + lng_str, tmplink + lng_link, LNG_LINK - lng_link, ancinfo + lng_anc ) ){

            // フラッシュしてからアンカーノードをつくる
            createTextNodeN( m_parsed_text, lng_text, color_text, bold ); lng_text = 0;

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

            create_ancnode( tmpstr, lng_str, tmplink, lng_link, COLOR_CHAR_LINK, bold, ancinfo, lng_anc );

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
        if( check_link( pos, (int)( pos_end - pos ), n_in, tmplink, LNG_LINK ) ){

            // フラッシュしてからリンクノードつくる
            createTextNodeN( m_parsed_text, lng_text, color_text, bold ); lng_text = 0;

            while( remove_imenu( tmplink ) );

            lng_link = strlen( tmplink );

            // 画像リンク
            if( DBIMG::is_loadable( tmplink, lng_link ) ){
                create_imgnode( pos, n_in, tmplink , lng_link, COLOR_IMG_NOCACHE, bold );
            }

            // 一般リンク
            else create_linknode( pos, n_in, tmplink , lng_link, COLOR_CHAR_LINK, bold );

            pos += n_in;

            // forのところで++されるので--しておく
            --pos;
            continue;
        }
        

        ///////////////////////
        // 特殊文字デコード
        if( *pos == '&' ){

            int ret_decode = DBTREE::decode_char( pos, n_in, m_parsed_text + lng_text, n_out );

            if( ret_decode != NODE_NONE ){

                // 文字以外の空白ノードならフラッシュして空白ノード追加
                if( ret_decode != NODE_TEXT ){
                    createTextNodeN( m_parsed_text, lng_text, color_text, bold );
                    lng_text = 0;
                    createSpNode( ret_decode );
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
            createTextNodeN( m_parsed_text, lng_text, color_text, bold ); lng_text = 0;
            createHTabNode();
            continue;
        }

        // 連続する空白は一個にする
        while( *pos == ' ' && *( pos + 1 ) == ' ' ) ++pos;

        m_parsed_text[ lng_text++ ] = *pos;
    }

    createTextNodeN( m_parsed_text, lng_text, color_text, bold );
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
bool NodeTreeBase::check_anchor( int mode, const char* str_in,
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
                tmp_out[ lng_out++ ] = 0xef;
                tmp_out[ lng_out++ ] = 0xbc;
                tmp_out[ lng_out++ ] = 0x9e;
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

            tmp_out[ lng_out++ ] = 0xe3;
            tmp_out[ lng_out++ ] = 0x80;
            tmp_out[ lng_out++ ] = 0x81;

            str_link[ 0 ] = ',';
            ++str_link;
            --lng_link;

            pos += 3;
        }

        else return false;
    }

    // 数字かチェック
    unsigned int n, dig;
    ancinfo->anc_from = ancinfo->anc_to = MISC::str_to_uint( pos, dig, n );
    if( dig == 0 || dig > MAX_LINK_DIGIT || ancinfo->anc_from == 0 ){

        // モード2で数字が長すぎるときは飛ばす
        if( mode == 2 && dig > MAX_LINK_DIGIT ) n_in = ( int )( pos - str_in ) + n; 

        return false;
    }

    // アンカーが現れたのでとりあえず作成する

    // 画面に表示する文字
    memcpy( str_out, tmp_out, lng_out );
    memcpy( str_out + lng_out, pos, n );
    str_out[ lng_out + n ] = '\0';

    // アンカー文字
    snprintf( str_link, lng_link, "%d", ancinfo->anc_from );
    pos += n;
    lng_out += n;    


    // dat形式では "&gt;&gt;数字"のパターンの場合には後に</a>がついてるのでのぞく
    if( *( pos ) == '<' && *( pos + 1 ) == '/' && ( *( pos + 2 ) == 'a' || *( pos + 2 ) == 'A' ) && *( pos + 3 ) == '>' ) pos += 4;


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
//
// 出力
// n_in : str_in から何バイト読み取ったか
// str_link : リンクの文字列
//
// 戻り値 : リンクが現れれば true
//

//
// Thanks to 「パッチ投稿」スレの8氏
//
// http://jd4linux.sourceforge.jp/cgi-bin/bbs/test/read.cgi/support/1151836078/8
//

enum
{
    LINKTYPE_NONE,

    LINKTYPE_HTTP,
    LINKTYPE_FTP,
    LINKTYPE_TTP,
    LINKTYPE_TP
};

bool NodeTreeBase::check_link( const char* str_in, int lng_in, int& n_in, char* str_link, int lng_link )
{
    // CONFIG::get_loose_url() == true の時はRFCで規定されていない文字も含める
    const bool loose_url = CONFIG::get_loose_url();

    // http://, https://, ftp://, ttp(s)://, tp(s):// のチェック
    int linktype = LINKTYPE_NONE;

    // http://, https://
    if( ( *( str_in ) == 'h' && *( str_in + 1 ) == 't' && *( str_in + 2 ) == 't' && *( str_in + 3 ) == 'p' ) &&
        ( ( *( str_in + 4 ) == ':' && *( str_in + 5 ) == '/' && *( str_in + 6 ) == '/' )
          || ( *( str_in + 4 ) == 's' && *( str_in + 5 ) == ':' && *( str_in + 6 ) == '/' && *( str_in + 7 ) == '/' ) ) ){
        linktype = LINKTYPE_HTTP;
    }
    // ftp://
    else if( *( str_in ) == 'f' && *( str_in + 1 ) == 't' && *( str_in + 2 ) == 'p' && 
             *( str_in + 3 ) == ':' && *( str_in + 4 ) == '/' && *( str_in + 5 ) == '/' ){
        linktype = LINKTYPE_FTP;
    }
    // ttp://
    else if( ( *( str_in ) == 't' && *( str_in + 1 ) == 't' && *( str_in + 2 ) == 'p'  ) && 
             ( ( *( str_in + 3 ) == ':' && *( str_in + 4 ) == '/' && *( str_in + 5 ) == '/' )
               || ( *( str_in + 3 ) == 's' && *( str_in + 4 ) == ':' && *( str_in + 5 ) == '/' && *( str_in + 6 ) == '/' ) ) ){
        linktype = LINKTYPE_TTP;
    }
    // tp://
    else if( ( *( str_in ) == 't' && *( str_in + 1 ) == 'p'  ) &&
             ( ( *( str_in + 2 ) == ':' && *( str_in + 3 ) == '/' && *( str_in + 4 ) == '/' )
               || ( *( str_in + 2 ) == 's' && *( str_in + 3 ) == ':' && *( str_in + 4 ) == '/' && *( str_in + 5 ) == '/' ) ) ){
        linktype = LINKTYPE_TP;
    }
    
    if( linktype == LINKTYPE_NONE ) return false;

    // リンクの長さを取得
    bool url_encode = false;
    char cchar = *( str_in + n_in );
    n_in = 0;
    while(
        // バッファサイズを越えない
        n_in < lng_in

        // < ではない
        && !( cchar == '&' && *( str_in + n_in +1 ) == 'l' && *( str_in + n_in +2 ) == 't'
              && *( str_in + n_in +3) == ';' ) 

        // > ではない
        && !( cchar == '&' && *( str_in + n_in +1 ) == 'g' && *( str_in + n_in +2 ) == 't'
              && *( str_in + n_in +3 ) == ';' ) 

        // " ではない
        && !( cchar == '&' && *( str_in + n_in +1 ) == 'q' && *( str_in + n_in +2 ) == 'u'
              && *( str_in + n_in +3 ) == 'o' && *( str_in + n_in +4 ) == 't' && *( str_in + n_in +5 ) == ';' ) 

        // [-a-zA-Z0-9!#$%&'()~=@;+:*,./?_] が続く限りwhileで回す
        && (
            ( cchar >= '0' && cchar <= '9' )
            || ( cchar >= 'a' && cchar <= 'z' )
            || ( cchar >= 'A' && cchar <= 'Z' )
            || cchar == '!'
            || cchar == '#'
            || cchar == '$'
            || cchar == '%'
            || cchar == '&'
            || cchar == '\''
            || cchar == '('
            || cchar == ')'
            || cchar == '~'
            || cchar == '-'
            || cchar == '='
            || cchar == '@'
            || cchar == ';'
            || cchar == '+'
            || cchar == ':'
            || cchar == '*'
            || cchar == ','
            || cchar == '.'
            || cchar == '/'
            || cchar == '?'
            || cchar == '_'

            || ( loose_url && cchar == '^' )

            )
        ){

        if( loose_url && cchar == '^' ) url_encode = true;

        ++n_in;
        cchar = *( str_in + n_in );
    }

    // 最後に ()が来たら除く
    if( *( str_in + n_in -1 ) == '('
        || *( str_in + n_in -1 ) == ')' ) --n_in;

    int offset = 0;
    if( linktype == LINKTYPE_TTP || linktype == LINKTYPE_TP ){ // ttp://, tp:// の場合、リンクの先頭にhを補完

        str_link[ 0 ] = 'h';
        offset = 1;

        if( linktype == LINKTYPE_TP ){ // tp:// の場合、さらにリンク先頭にtを補完
            str_link[ 1 ] = 't';
            offset = 2;
        }
    }

    if( n_in + offset > lng_link ) return false;

    if( ! url_encode ){
        memcpy( str_link + offset, str_in, n_in );
        str_link[ n_in + offset ] = '\0';
    }
    else{ // URLエンコードが必要な場合

        char *pos = str_link + offset;
        for( int i = 0; i < n_in; ++i, ++pos ){

            if( str_in[ i ] == '^' ){ // '^' -> %5e
                *( pos++ ) = '%';
                *( pos++ ) = '5';
                *pos = 'e';
            }
            else *pos = str_in[ i ];
        }
        *pos = '\0';
    }

    return true;
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
void NodeTreeBase::copy_abone_info( std::list< std::string >& list_abone_id,
                                    std::list< std::string >& list_abone_name,
                                    std::list< std::string >& list_abone_word,
                                    std::list< std::string >& list_abone_regex,
                                    std::vector< char >& vec_abone_res,
                                    bool& abone_transparent, bool& abone_chain )
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
}


//
// from_number番から to_number 番までのレスのあぼーん状態を更新
//
void NodeTreeBase::update_abone( int from_number, int to_number )
{
    if( empty() ) return;
    if( to_number < from_number ) return;

    for( int i = from_number ; i <= to_number; ++i ){
        if( check_abone_res( i ) )  continue;
        if( check_abone_id( i ) )  continue;
        if( check_abone_name( i ) ) continue;
        if( check_abone_word( i ) ) continue;
        if( check_abone_chain( i ) ) continue;
    }
}



//
// number番のあぼーん判定(レスあぼーん)
//
// あぼーんの時はtrueを返す
//
bool NodeTreeBase::check_abone_res( int number )
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
bool NodeTreeBase::check_abone_id( int number )
{
    bool check_id = ! m_list_abone_id.empty();
    bool check_id_board = ! m_list_abone_id_board.empty();

    if( !check_id && !check_id_board ) return false;

    NODE* head = res_header( number );
    if( ! head ) return false;
    if( ! head->headinfo ) return false;
    if( head->headinfo->abone ) return true;
    if( ! head->headinfo->block[ BLOCK_ID_NAME ] ) return false;

    int ln_protoid = strlen( PROTO_ID );

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
bool NodeTreeBase::check_abone_name( int number )
{
    bool check_name = ! m_list_abone_name.empty();
    bool check_name_board = ! m_list_abone_name_board.empty();
    bool check_name_global = ! CONFIG::get_list_abone_name().empty();

    if( !check_name && !check_name_board && !check_name_global ) return false;

    NODE* head = res_header( number );
    if( ! head ) return false;
    if( ! head->headinfo ) return false;
    if( head->headinfo->abone ) return true;
    if( ! head->headinfo->name ) return false;

    std::list< std::string >::iterator it;
    std::string name_str( head->headinfo->name );

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
// number番のあぼーん判定( word, regex  /  board word, board regex / global word, global regex )
//
// あぼーんの時はtrueを返す
//
bool NodeTreeBase::check_abone_word( int number )
{
    bool check_word = ! m_list_abone_word.empty();
    bool check_regex = ! m_list_abone_regex.empty();

    bool check_word_board = ! m_list_abone_word_board.empty();
    bool check_regex_board = ! m_list_abone_regex_board.empty();

    bool check_word_global = ! m_list_abone_word_global.empty();
    bool check_regex_global = ! m_list_abone_regex_global.empty();

    if( !check_word && !check_regex
        && !check_word_board && !check_regex_board
        && !check_word_global && !check_regex_global ) return false;

    NODE* head = res_header( number );
    if( ! head ) return false;
    if( ! head->headinfo ) return false;
    if( head->headinfo->abone ) return true;

    std::string res_str = get_res_str( number );
    JDLIB::Regex regex;

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
            if( regex.exec( *it, res_str ) ){
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
            if( regex.exec( *it, res_str ) ){
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
            if( regex.exec( *it, res_str ) ){
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
bool NodeTreeBase::check_abone_chain( int number )
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
            tmphead->headinfo->block[ BLOCK_NUMBER ]->next_node->color_text = COLOR_CHAR_LINK;
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
void NodeTreeBase::check_reference( int number )
{
    NODE* head = res_header( number );
    if( ! head ) return;

    // 既にあぼーんしているならチェックしない
    if( head->headinfo->abone ) return;

    // 2重チェック防止用
    bool checked[ number ];
    memset( checked, 0, sizeof( bool ) * number );

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

                        // number-1 番以下のレスだけを見る
                        if( anc_from >= number ) continue;
                        anc_to = MIN( anc_to, number -1 );

                        // >>1-1000 みたいなアンカーは弾く
                        if( anc_to - anc_from >= RANGE_REF ) continue;

                        for( int i = anc_from; i <= anc_to ; ++i ){

                            // 既にチェックしている
                            if( checked[ i ] ) continue;

                            NODE* tmphead = res_header( i );
                            if( tmphead
                                && ! tmphead->headinfo->abone // 対象スレがあぼーんしていたらカウントしない
                                && tmphead->headinfo->block[ BLOCK_NUMBER ]
                                ){

                                checked[ i ] = true;

                                // 参照数を更新して色を変更
                                ++( tmphead->headinfo->num_reference );
                                if( tmphead->headinfo->num_reference >= 3 )
                                    tmphead->headinfo->block[ BLOCK_NUMBER ]->next_node->color_text = COLOR_CHAR_LINK_HIGH;
                                else if( tmphead->headinfo->num_reference >= 1 )
                                    tmphead->headinfo->block[ BLOCK_NUMBER ]->next_node->color_text = COLOR_CHAR_LINK_LOW;
                                else tmphead->headinfo->block[ BLOCK_NUMBER ]->next_node->color_text = COLOR_CHAR_LINK;
                            }
                        }
                    }
                }

            }

            node = node->next_node;

        } // while( node )

    } // for( block )
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
void NodeTreeBase::update_id_name( int from_number, int to_number )
{
    if( empty() ) return;
    if( to_number < from_number ) return;
    for( int i = from_number ; i <= to_number; ++i ) check_id_name( i );
}



//
// number番のレスの発言数を更新
//
void NodeTreeBase::check_id_name( int number )
{
    NODE* header = res_header( number );

    // あぼーんしているならチェックしない
    if( header->headinfo->abone ) return;

    if( header && header->headinfo->block[ BLOCK_ID_NAME ] ){

        char* str_id = header->headinfo->block[ BLOCK_ID_NAME ]->next_node->linkinfo->link;

        // 以前に出た同じIDのレスの発言数を更新しつつ発言回数を調べる
        int num_id_name = 1;
        for( int i = 1 ; i < header->id_header; ++i ){

            NODE* tmphead = m_vec_header[ i ];

            if( tmphead
                && ! tmphead->headinfo->abone // 対象スレがあぼーんしていたらカウントしない
                && tmphead->headinfo->block[ BLOCK_ID_NAME ]
                && strcmp( str_id, tmphead->headinfo->block[ BLOCK_ID_NAME ]->next_node->linkinfo->link ) == 0 ){

                set_num_id_name( tmphead, tmphead->headinfo->num_id_name+1 );
                ++num_id_name;
            }
        }

        set_num_id_name( header, num_id_name );
    }
}



//
// 発言数( num_id_name )の更新
//
// IDノードの色も変更する
//
void NodeTreeBase::set_num_id_name( NODE* header, int num_id_name )
{
    if( header->headinfo->block[ BLOCK_ID_NAME ] ){

        header->headinfo->num_id_name = num_id_name;        
        if( num_id_name >= 4 ) header->headinfo->block[ BLOCK_ID_NAME ]->next_node->color_text = COLOR_CHAR_LINK_HIGH;
        else if( num_id_name >= 2 ) header->headinfo->block[ BLOCK_ID_NAME ]->next_node->color_text = COLOR_CHAR_LINK;
        else header->headinfo->block[ BLOCK_ID_NAME ]->next_node->color_text = COLOR_CHAR;
    }
}




//
// http://ime.nu/ などをリンクから削除
//
// 取り除いたらtrueを返す
//
// Thanks to 「ハッチ投稿」スレの24氏
//
// http://jd4linux.sourceforge.jp/cgi-bin/bbs/test/read.cgi/support/1151836078/24
//
bool NodeTreeBase::remove_imenu( char* str_link )
{
    const int lng_http = 7; // = strlen( "http://" );

    if( ( str_link[ lng_http ] == 'i' && str_link[ lng_http +1 ] == 'm' )
        || ( str_link[ lng_http ] == 'n' && str_link[ lng_http +1 ] == 'u' )
        || ( str_link[ lng_http ] == 'p' && str_link[ lng_http +1 ] == 'i' ) ){

        if( strstr( str_link, "http://ime.nu/" ) == str_link
               || strstr( str_link, "http://ime.st/" ) == str_link
               || strstr( str_link, "http://nun.nu/" ) == str_link
               || strstr( str_link, "http://pinktower.com/" ) == str_link ){

            int linklen = strlen( str_link ) +1;
            int cutsize = 0; 

            if( str_link[ lng_http ] == 'p' ) cutsize = 14; // = strlen( "pinktower.com/" )
            else cutsize =  7; // = strlen( "ime.nu/" )

            memmove( str_link + lng_http, str_link + lng_http + cutsize, linklen - ( lng_http + cutsize ) );

            return true;
        }
    }

    return false;
}
