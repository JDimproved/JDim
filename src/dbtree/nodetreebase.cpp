// ライセンス: 最新のGPL

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
// URL を含むレス番号をリストにして取得
//
std::list< int > NodeTreeBase::get_res_with_url()
{
    std::list< int > list_res;
    for( int i = 1; i <= m_id_header; ++i ){

        NODE* node = res_header( i );
        while( node ){

            if( node->type == NODE_LINK
                && std::string( node->linkinfo->link ).find( "http" ) == 0 ){
                list_res.push_back( i );
                break;
            }
            node = node->next_node;
        }
    }
    
    return list_res;
}



//
// number番のレスを参照しているレスの番号全てをリストにして取得
//
std::list< int > NodeTreeBase::get_res_reference( int number )
{
    std::list< int > list_ref;
    for( int i = number + 1; i <= m_id_header; ++i ){

        NODE* node = res_header( i );
        while( node ){

            if( node->type == NODE_LINK ){

                // アンカーノードの時は node->linkinfo->anc_from != 0
                int anc_from = node->linkinfo->anc_from;
                int anc_to = node->linkinfo->anc_to;

                const int range = RANGE_REF; // >>1-1000 みたいなアンカーは弾く
                if( anc_from && anc_to - anc_from < range && anc_from <= number && number <= anc_to ) {
                    list_ref.push_back( i );
                    break;
                }
            }
            node = node->next_node;
        }
    }

#ifdef _DEBUG
    std::cout << "NodeTreeBase::get_reference\n";
    std::list < int >::iterator it;
    for( it = list_ref.begin(); it != list_ref.end(); ++it ) std::cout << *it << std::endl;
#endif
    
    return list_ref;
}



//
// number　番のレスの文字列を返す
//
// ref == true なら先頭に ">" を付ける
//
const std::string NodeTreeBase::get_res_str( int number, bool ref )
{
    std::string str_res;

#ifdef _DEBUG
    std::cout << "NodeTreeBase::get_res_str : num = " << number << std::endl;
#endif

    NODE* node = res_header( number );
    if( ! node ) return std::string();

    if( ref ) str_res += "> ";
    
    while( node ){
    
        if( node->type == DBTREE::NODE_BR ){
            str_res += "\n";
            if( ref ) str_res += "> ";
        }
        else if( node->text ){
            str_res += node->text;
        }

        node = node->next_node;
    }

    str_res += "\n";

#ifdef _DEBUG
//    std::cout << str_res << std::endl;
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
// number番の ID 取得
//
const std::string NodeTreeBase::get_id_name( int number )
{
    NODE* head = res_header( number );
    if( ! head ) return std::string();
    if( ! head->headinfo->node_id_name ) return std::string();

    return head->headinfo->node_id_name->linkinfo->link;
}



//
// 基本ノード作成
//
NODE* NodeTreeBase::createNode()
{
    NODE* tmpnode = ( NODE* ) m_heap.heap_alloc( sizeof( NODE ) );

    tmpnode->id_header = m_id_header;
    tmpnode->id = m_id_node++;
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
    m_id_node = 0;
    m_node_previous = NULL;

    NODE* tmpnode = createNode();
    tmpnode->type =  NODE_HEADER;

    // ヘッダ情報
    tmpnode->headinfo = ( HEADERINFO* )m_heap.heap_alloc( sizeof( HEADERINFO ) );
    if( m_id_header >= 2 ) m_vec_header[ m_id_header -1 ]->headinfo->next_header = tmpnode;
    
    return tmpnode;
}


//
// 改行ノード作成
//
NODE* NodeTreeBase::createBrNode()
{
    NODE* tmpnode = createNode();
    tmpnode->type = NODE_BR;

    // 便宜的に'\0'をセットする
    tmpnode->text = ( char* )m_heap.heap_alloc( 2 );
    tmpnode->text[ 0 ] = '\0';
    
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
// マージンレベル下げノード作成
//
NODE* NodeTreeBase::create_node_downleft()
{
    NODE* tmpnode = createNode();
    tmpnode->type = NODE_DOWN_LEFT;

    return tmpnode;
}


//
// リンクノード作成
//
// bold : 太字か
// img : 画像か
// anc_from, anc_to : アンカーの番号( anc_from == 0 なら一般のリンク )

NODE* NodeTreeBase::create_linknode( const char* text, int n, const char* link, int n_link, int color_text,
                                     bool bold, bool img, int anc_from, int anc_to )
{
    NODE* tmpnode = createTextNodeN( text, n, color_text, bold );
    if( tmpnode ){
        tmpnode->type = NODE_LINK;
        
        // リンク情報セット
        tmpnode->linkinfo = ( LINKINFO* )m_heap.heap_alloc( sizeof( LINKINFO ) );

        tmpnode->linkinfo->link = ( char* )m_heap.heap_alloc( n_link +4 );
        memcpy( tmpnode->linkinfo->link, link, n_link ); tmpnode->linkinfo->link[ n_link ] = '\0';

        tmpnode->linkinfo->anc_from = MAX( 0, anc_from );
        tmpnode->linkinfo->anc_to = MAX( anc_from, anc_to );

        tmpnode->linkinfo->image = img;
    }
    
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

    NODE* tmpnode = create_header_node();
    m_vec_header[ m_id_header ] = tmpnode;

    init_loading();
    parse_html( html.c_str(), html.length(), COLOR_CHAR );
    clear();

    return tmpnode;
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
    if( CACHE::is_file_exists( path_cache ) == CACHE::EXIST_FILE ){

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
        add_raw_lines( m_buffer_lines );
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
        err << m_url << " load failed. : " << get_str_code();
        MISC::ERRMSG( err.str() );
    }

    // Requested Range Not Satisfiable
    if( get_code() == HTTP_RANGE_ERR ) m_broken = true;

    // データがロードされなかったらキャッシュを消す
    if( get_res_number() == 0 ){

        std::string path = CACHE::path_dat( m_url );
        if( CACHE::is_file_exists( path ) == CACHE::EXIST_FILE ) unlink( path.c_str() );
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
void NodeTreeBase::add_raw_lines( char* rawlines )
{
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

    // データベース更新
    // Article::slot_node_updated()が呼び出される
    if( num_before != m_id_header ) m_sig_updated.emit();
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
    header->headinfo->node_res = create_linknode( tmpstr, strlen( tmpstr ) , tmplink, strlen( tmplink ), COLOR_CHAR_LINK, true );

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

        createTextNode( "broken", COLOR_CHAR );
        createBrNode();
        createBrNode();
        
        return pos;
    }
    
    // 名前
    parseName( header, section[ 0 ], section_lng[ 0 ] );
    
    // メール
    parseMail( section[ 1 ], section_lng[ 1 ] );

    // 日付とID
    parse_date_id( header, section[ 2 ], section_lng[ 2 ] );

    // 改行してレベル下げ
    createBrNode();
    create_node_downleft();
    create_node_downleft();
    
    // 本文
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
    const bool digitlink = true;
    const bool bold = true;
    int pos_trip_begin = 0;
    int pos_trip_end = 0;
    int i;

    NODE* node = createTextNode( " 名前：", COLOR_CHAR );

    // トリップ付きの時は中の数字をリンクにしない
    for( i = 0; i < lng; ++i ) if( str[ i ] == '<' && str[ i+2 ] == 'b' ) break;
    if( i != lng ){

        pos_trip_begin = i;
        pos_trip_end = lng -1;
        for( i = pos_trip_begin + 4; i < lng; ++i ){
            if( str[ i ] == '<' && str[ i+1 ] == 'b' ){
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

        // トリップ
        parse_html( str + pos_trip_begin, pos_trip_end - pos_trip_begin + 1, COLOR_CHAR_NAME, false, bold );

        // トリップの後
        if( pos_trip_end < lng-1 )
            parse_html( str + pos_trip_end + 1, lng - pos_trip_end - 1, COLOR_CHAR_NAME, digitlink, bold );
    }

    // デフォルト名無しと同じときはアンカーを作らない
    else if( strncmp( m_default_noname.data(), str, lng ) == 0 ) parse_html( str, lng, COLOR_CHAR_NAME, false, bold );

    // 通常の場合は中に数字があったらアンカーにする
    else parse_html( str, lng, COLOR_CHAR_NAME, digitlink, bold );

    // プレインな名前取得
    node = node->next_node;
    std::string str_tmp;
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
void NodeTreeBase::parseMail( const char* str, int lng )
{
    createTextNode( " [", COLOR_CHAR );
    parse_html( str, lng, COLOR_CHAR, true );
    createTextNode( "]：", COLOR_CHAR );
}


//
// 日付とIDとBE
//
void NodeTreeBase::parse_date_id( NODE* header, const char* str, int lng )
{
    int start = 0;
    int lng_text = 0;
    char tmplink[ LNG_LINK ];

    int lng_id_tmp;
    char tmpid[ LNG_ID ];

    for(;;){

        // 空白を飛ばす
        while( start + lng_text < lng && str[ start + lng_text ] == ' ' ) ++lng_text;

        int start_block = start + lng_text;
        int lng_block = 0;
        while( start_block + lng_block < lng && str[ start_block + lng_block ] != ' ' ) ++lng_block;
        if( !lng_block ) break;

        // ID ( ??? の時は除く )
        if( str[ start_block ] == 'I' && str[ start_block + 1 ] == 'D' && str[ start_block + 3 ] != '?' ){

            // フラッシュ
            if( lng_text ) createTextNodeN( str + start, lng_text, COLOR_CHAR );
            start = start_block + lng_block;
            lng_text = 0;

            // id 取得
            lng_id_tmp = lng_block -3;
            memcpy( tmpid, str + start_block + 3, lng_id_tmp );
            tmpid[ lng_id_tmp ] = '\0';
            
            // リンク文字作成
            memcpy( tmplink, PROTO_ID, sizeof( PROTO_ID ) );
            memcpy( tmplink + sizeof( PROTO_ID ) - 1, tmpid, lng_id_tmp + 1 );
            
            // リンク作成
            NODE* node_id_name = create_linknode( "ID:", 3 , tmplink, strlen( tmplink ), COLOR_CHAR );
            createTextNodeN( tmpid, lng_id_tmp, COLOR_CHAR);

            // ヘッダにリンクノードへのポインタを登録
            header->headinfo->node_id_name = node_id_name;
        }

        // BE:
        else if( str[ start_block ] == 'B' && str[ start_block + 1 ] == 'E' ){

            // フラッシュ
            if( lng_text ) createTextNodeN( str + start, lng_text, COLOR_CHAR );
            start = start_block + lng_block;
            lng_text = 0;

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
            create_linknode( "BE:", 3 , tmplink, strlen( tmplink ), COLOR_CHAR );
            createTextNodeN( tmpid, lng_id_tmp, COLOR_CHAR);
        }

        // テキスト
        else lng_text += lng_block;
    }

    // フラッシュ
    if( lng_text ) createTextNodeN( str + start, lng_text, COLOR_CHAR );
}


//
// HTMLパーサ
//
// digitlink : true の時は数字が現れたら手当たり次第にアンカーにする( parseName() などで使う )
//             false なら数字の前に >> がついてるときだけアンカーにする
//
// bold : ボールド標示
//
void NodeTreeBase::parse_html( const char* str, int lng, int color_text, bool digitlink, bool bold )
{
    const char* pos = str;
    const char* pos_end = str + lng;
    int lng_text = 0;
    
    // 行頭の1個以上の空白は除く
    if( ! digitlink ) while( *pos == ' ' && *( pos + 1 ) != ' ' ) ++pos;
   
    for( ; pos < pos_end; ++pos ){


        ///////////////////////
        // HTMLタグ
        if( *pos == '<' ){ 

            // 改行 <br>
            if( *( pos + 1 ) == 'b' && *( pos + 2 ) == 'r' ){  

                // フラッシュ
                if( *( pos - 1 ) == ' ' ) --lng_text; // 改行前の空白を取り除く
                createTextNodeN( m_parsed_text, lng_text, color_text, bold ); lng_text = 0;

                pos += 4;
                
                // 改行ノード作成
                createBrNode();

                //改行直後の空白を取り除く
                if( *pos == ' ' ) ++pos;
                
                // 行頭の1個以上の空白を除く
                while( *pos == ' ' && *( pos + 1 ) != ' ' ) ++pos;
            }

            // </a>
            else if( *( pos + 1 ) == '/' && *( pos + 2 ) == 'a' && *( pos + 3 ) == '>' ) pos += 4;

            // その他のタグは無視。タグを取り除いて中身だけを見る
            else {

                // フラッシュ
                createTextNodeN( m_parsed_text, lng_text, color_text, bold ); lng_text = 0;
                
                while( pos < pos_end && *pos != '>' ) ++pos;
                ++pos;
            }

            // forのところで++されるので--しとく
            --pos;
            continue;
        }


        ///////////////////////
        // アンカーのチェック
        int n_in = 0;
        int n_out = 0;
        char tmpstr[ LNG_LINK ], tmplink[ LNG_LINK ];
        int anc_from, anc_to;
        if( check_anchor( 2 * digitlink, pos, n_in, tmpstr, tmplink, LNG_LINK, anc_from, anc_to ) ){

            // フラッシュしてからリンクノードつくる
            createTextNodeN( m_parsed_text, lng_text, color_text, bold ); lng_text = 0;

            create_linknode( tmpstr, strlen( tmpstr), tmplink, strlen( tmplink ), COLOR_CHAR_LINK, bold, false, anc_from, anc_to );
            pos += n_in; 

            // , や = が続くとき
            while( check_anchor( 1, pos, n_in, tmpstr, tmplink, LNG_LINK, anc_from, anc_to ) ){
                create_linknode( tmpstr, strlen( tmpstr), tmplink, strlen( tmplink ), COLOR_CHAR_LINK, bold,
                     false, anc_from, anc_to );
                pos += n_in; 
            }

            // forのところで++されるので--しとく
            --pos;
            continue;
        }

        // digitlink = true の時は数字が長すぎるときは飛ばす( 例えば　名前: 12345678 みたいなとき )
        if( digitlink ){
            --n_in;
            while( n_in-- > 0 ) m_parsed_text[ lng_text++ ] = *(pos++);
        }

        ///////////////////////
        // http(s)://, ttp(s)://, tp(s):// のチェック
        int create_link = 0;
        if( ( *( pos ) == 'h' && *( pos + 1 ) == 't' && *( pos + 2 ) == 't' && *( pos + 3 ) == 'p' ) &&
            ( ( *( pos + 4 ) == ':' && *( pos + 5 ) == '/' && *( pos + 6 ) == '/' )
              || ( *( pos + 4 ) == 's' && *( pos + 5 ) == ':' && *( pos + 6 ) == '/' && *( pos + 7 ) == '/' ) ) ) create_link = 1;
        else if( ( *( pos ) == 't' && *( pos + 1 ) == 't' && *( pos + 2 ) == 'p'  ) && 
            ( ( *( pos + 3 ) == ':' && *( pos + 4 ) == '/' && *( pos + 5 ) == '/' )
              || ( *( pos + 3 ) == 's' && *( pos + 4 ) == ':' && *( pos + 5 ) == '/' && *( pos + 6 ) == '/' ) ) ) create_link = 2;
        else if( ( *( pos ) == 't' && *( pos + 1 ) == 'p'  ) &&
            ( ( *( pos + 2 ) == ':' && *( pos + 3 ) == '/' && *( pos + 4 ) == '/' )
              || ( *( pos + 2 ) == 's' && *( pos + 3 ) == ':' && *( pos + 4 ) == '/' && *( pos + 5 ) == '/' ) ) ) create_link = 3;
        if( create_link ){

            // フラッシュしてからリンクノードつくる
            createTextNodeN( m_parsed_text, lng_text, color_text, bold ); lng_text = 0;

            int n = 0;
            while( *( pos + n ) != '<' && *( pos + n ) != ' ' && pos + n < pos_end
                   && *( pos + n ) > 0 // 半角文字が続く限り
                ) ++n;

            // m_parsed_text に http://〜をコピー
            int offset = 0;
            if( create_link >= 2 ){ // ttp:// の場合
                m_parsed_text[ 0 ] = 'h';
                offset = 1;
            }
            if( create_link >= 3 ){ // tp:// の場合
                m_parsed_text[ 1 ] = 't';
                offset = 2;
            }
            memcpy( m_parsed_text + offset, pos, n );

            // リンクノード作成
            bool img = false;
            int color_tmp = COLOR_CHAR_LINK;

            // 画像かどうか
            if( DBIMG::is_loadable( m_parsed_text, n + offset ) ){
                img = true;
                color_tmp = COLOR_IMG_NOCACHE;
            }

            create_linknode( m_parsed_text + offset, n, m_parsed_text , n + offset, color_tmp, bold, img ); 
            pos += n;

            // forのところで++されるので--しとく
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

                // forのところで++されるので--しとく
                --pos;
                continue;
            }
        }

        m_parsed_text[ lng_text++ ] = *pos;
    }

    createTextNodeN( m_parsed_text, lng_text, color_text, bold );
}



//
// アンカーが現れたかチェックして文字列を取得する関数
//
// 入力
// mode : 0 なら >> が先頭に無ければアンカーにしない、1 なら,か=があればアンカーにする、2 なら数字が先頭に来たらアンカーにする
// str_in : 入力文字列の先頭アドレス
// lng_link : str_linkのバッファサイズ
//
// 出力
// n_in : str_in から何バイト読み取ったか
// str_out : (画面に表示される)文字列
// str_link : リンクの文字列
// anc_from, anc_to : num番からanc_to番までのアンカーが現れた
//
// 戻り値 : アンカーが現れれば true
//
bool NodeTreeBase::check_anchor( int mode, const char* str_in,
                                 int& n_in, char* str_out, char* str_link, int lng_link, int& anc_from, int& anc_to )
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

    // カンマかイコールをチェック
    else if( mode == 1 ){  

        if( *( pos ) == '=' || *( pos ) == ',' ){
            tmp_out[ lng_out ] = *( pos );
            ++lng_out;
            ++pos;
        }

        // utf-8で"、"
        else if( ( unsigned char )( *pos ) == 0xe3 && ( unsigned char ) ( *( pos + 1 ) ) == 0x80
                     && ( unsigned char ) ( *( pos + 2 ) ) == 0x81 ){
                tmp_out[ lng_out++ ] = 0xe3;
                tmp_out[ lng_out++ ] = 0x80;
                tmp_out[ lng_out++ ] = 0x81;
                pos += 3;
        }

        else return false;
    }

    // 数字かチェック
    unsigned int n, dig;
    anc_from = anc_to = MISC::str_to_uint( pos, dig, n );
    if( dig == 0 || dig > MAX_LINK_DIGIT || anc_from == 0 ){

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
    snprintf( str_link, lng_link, "%s%d",  PROTO_ANCHORE, anc_from );
    pos += n;
    lng_out += n;    


    // dat形式では "&gt;&gt;数字"のパターンの場合には後に</a>がついてるのでのぞく
    if( *( pos ) == '<' && *( pos + 1 ) == '/' && *( pos + 2 ) == 'a' && *( pos + 3 ) == '>' ) pos += 4;


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
        
        anc_to = MISC::str_to_uint( pos + offset, dig, n );
        if( dig && dig <= MAX_LINK_DIGIT && anc_from ){

            // 画面に表示する文字            
            memcpy( str_out + lng_out, pos, offset + n );
            str_out[ lng_out + offset + n ] = '\0';

            // アンカー文字をもう一度作成
            snprintf( str_link, lng_link, "%s%d-%d",  PROTO_ANCHORE, anc_from, anc_to );
            pos += offset + n;
            lng_out += offset + n;
        }
    }

    //"&gt;&gt;数字-数字</a>"のパターンの時に</a>をのぞく
    if( *( pos ) == '<' && *( pos + 1 ) == '/' && *( pos + 2 ) == 'a' && *( pos + 3 ) == '>' ) pos += 4;
    
    n_in = ( int )( pos - str_in );

    return true;
}




// あぼーんのクリア
void NodeTreeBase::clear_abone()
{
    for( int i = 1; i <= m_id_header; ++i ){
        NODE* tmphead = m_vec_header[ i ];
        if( tmphead && tmphead->headinfo ) tmphead->headinfo->abone = false;
    }
}


//
// number番のあぼーん判定(ID)
//
// あぼーんの時はtrueを返す
//
bool NodeTreeBase::check_abone_id( int number, std::list< std::string >& list_id )
{
    if( list_id.empty() ) return false;

    NODE* head = res_header( number );
    if( ! head ) return false;
    if( head->headinfo->abone ) return true;

    int ln_protoid = strlen( PROTO_ID );

    std::list< std::string >::iterator it = list_id.begin();
    for( ; it != list_id.end(); ++it ){

        // std::string の find は遅いのでstrcmp使う
        if( strcmp( head->headinfo->node_id_name->linkinfo->link + ln_protoid, ( *it ).c_str() ) == 0 ){
            head->headinfo->abone = true;
            return true;
        }
    }

    return false;
}


//
// number番のあぼーん判定(name)
//
// あぼーんの時はtrueを返す
//
bool NodeTreeBase::check_abone_name( int number, std::list< std::string >& list_name )
{
    if( list_name.empty() ) return false;

    NODE* head = res_header( number );
    if( ! head ) return false;
    if( head->headinfo->abone ) return true;

    std::list< std::string >::iterator it = list_name.begin();
    for( ; it != list_name.end(); ++it ){

        // std::string の find は遅いのでstrcmp使う
        const char* s1 = head->headinfo->name;
        const char* s2 = ( *it ).c_str();
        while( *s1 != '\0' && *s1 != *s2 ) s1++;

        if( *s1 != '\0'  && strncmp( s1, s2, strlen( s2 ) ) == 0 ){
            head->headinfo->abone = true;
            return true;
        }
    }

    return false;
}



//
// number番のあぼーん判定(word)
//
// あぼーんの時はtrueを返す
//
bool NodeTreeBase::check_abone_word( int number, std::list< std::string >& list_word )
{
    if( list_word.empty() ) return false;

    NODE* head = res_header( number );
    if( ! head ) return false;
    if( head->headinfo->abone ) return true;

    std::string res_str = get_res_str( number );

    std::list< std::string >::iterator it = list_word.begin();
    for( ; it != list_word.end(); ++it ){
        if( res_str.find( *it ) != std::string::npos ){
            head->headinfo->abone = true;
            return true;
        }
    }

    return false;
}


//
// number番のあぼーん判定(regex)
//
// あぼーんの時はtrueを返す
//
bool NodeTreeBase::check_abone_regex( int number, std::list< std::string >& list_regex )
{
    if( list_regex.empty() ) return false;

    NODE* head = res_header( number );
    if( ! head ) return false;
    if( head->headinfo->abone ) return true;

    JDLIB::Regex regex;
    std::string res_str = get_res_str( number );

    std::list< std::string >::iterator it = list_regex.begin();
    for( ; it != list_regex.end(); ++it ){
        if( regex.exec( *it, res_str ) ){
            head->headinfo->abone = true;
            return true;
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
    NODE* head = res_header( number );
    if( ! head ) return false;
    if( head->headinfo->abone ) return true;

    NODE* node = head;
    while( node ){

        if( node->type == NODE_LINK ){

            // アンカーノードの時は node->linkinfo->anc_from != 0
            int anc_from = node->linkinfo->anc_from;
            int anc_to = node->linkinfo->anc_to;

            // >>20-30 の様に範囲でアンカーを張っている場合は除く
            if( anc_from && anc_from == anc_to ){

                NODE* tmphead = res_header( anc_from );

                if( tmphead && tmphead->headinfo->abone ){
                    head->headinfo->abone = true;
                    return true;
                }
            }
        }
        node = node->next_node;
    }

    return false;
}



// 参照数(num_reference)と色のクリア
void NodeTreeBase::clear_reference()
{
    for( int i = 1; i <= m_id_header; ++i ){
        NODE* tmphead = m_vec_header[ i ];
        if( tmphead && tmphead->headinfo && tmphead->headinfo->node_res ){
            tmphead->headinfo->num_reference = 0;
            tmphead->headinfo->node_res->color_text = COLOR_CHAR_LINK;
        }
    }
}


//
// number番のレスが参照しているレスのレス番号の参照数(num_reference)と色を更新する
//
void NodeTreeBase::update_reference( int number )
{
    NODE* node = res_header( number );
    while( node ){

        if( node->type == NODE_LINK ){

            // アンカーノードの時は node->linkinfo->anc_from != 0
            int anc_from = node->linkinfo->anc_from;
            int anc_to = node->linkinfo->anc_to;

            const int range = RANGE_REF; // >>1-1000 みたいなアンカーは弾く
            if( anc_from && anc_to - anc_from < range ){

                for( int i = anc_from; i <= MIN( m_id_header -1, anc_to ) ; ++i ){
        
                    NODE* tmphead = res_header( i );
                    if( tmphead && ! tmphead->headinfo->abone && tmphead->headinfo->node_res ){

                        // 参照数を更新して色を変更
                        ++( tmphead->headinfo->num_reference );
                        if( tmphead->headinfo->num_reference >= 3 ) tmphead->headinfo->node_res->color_text = COLOR_CHAR_LINK_RED;
                        else if( tmphead->headinfo->num_reference >= 1 ) tmphead->headinfo->node_res->color_text = COLOR_CHAR_LINK_PUR;
                    }
                }
            }
        }

        node = node->next_node;
    }
}


// 発言数(( num_id_name ))とIDの色のクリア
void NodeTreeBase::clear_id_name()
{
    for( int i = 1; i <= m_id_header; ++i ){
        NODE* tmphead = m_vec_header[ i ];
        if( tmphead && tmphead->headinfo && tmphead->headinfo->node_id_name ){
            tmphead->headinfo->num_id_name = 0;
            tmphead->headinfo->node_id_name->color_text = COLOR_CHAR;
        }
    }
}


//
// number番のレスの発言数を更新
//
void NodeTreeBase::update_id_name( int number )
{
    NODE* header = res_header( number );

    if( header && header->headinfo->node_id_name ){

        char* str_id = header->headinfo->node_id_name->linkinfo->link;

        // 以前に出た同じIDのレスの発言数を更新しつつ発言回数を調べる
        int num_id_name = 1;
        for( int i = 1 ; i < header->id_header; ++i ){

            NODE* tmphead = m_vec_header[ i ];

            if( tmphead
                && ! tmphead->headinfo->abone
                && tmphead->headinfo->node_id_name
                && str_id[ 0 ] == tmphead->headinfo->node_id_name->linkinfo->link[ 0 ] // とりあえず先頭だけで比較
                && strcmp( str_id, tmphead->headinfo->node_id_name->linkinfo->link ) == 0 ){

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
    header->headinfo->num_id_name = num_id_name;        
    if( num_id_name >= 4 ) header->headinfo->node_id_name->color_text = COLOR_CHAR_LINK_RED;        
    else if( num_id_name >= 2 ) header->headinfo->node_id_name->color_text = COLOR_CHAR_LINK;
    else header->headinfo->node_id_name->color_text = COLOR_CHAR;
}
