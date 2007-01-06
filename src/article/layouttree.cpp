// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "layouttree.h"

#include "dbtree/interface.h"
#include "dbtree/articlebase.h"
#include "dbtree/nodetreebase.h"

#include "jdlib/miscutil.h"

#include "config/globalconf.h"

#include "global.h"

using namespace ARTICLE;

#define SIZE_OF_HEAP 256 * 1024


// m_show_abone = true ならあぼーんしたレスも表示する
LayoutTree::LayoutTree( const std::string& url, bool show_abone )
    : m_heap( SIZE_OF_HEAP ),
      m_url( url ),
      m_local_nodetree( 0 ),
      m_show_abone( show_abone )
{
#ifdef _DEBUG
    std::cout << "LayoutTree::LayoutTree : url = " << url << " show_abone = " << m_show_abone << std::endl;
#endif    

    m_article = DBTREE::get_article( m_url );
    assert( m_article );

    m_dummy_str = '\0';

    clear();
}


LayoutTree::~LayoutTree()
{
#ifdef _DEBUG
    std::cout << "LayoutTree::~LayoutTree : url = " << m_url << std::endl;
#endif

    clear();
}



void LayoutTree::clear()
{
    m_heap.clear();
    
    m_last_header = 0;
    m_max_res_number = 0;

    m_id_header = -1; // ルートヘッダのIDが 0 になるように -1 を入れておく
    m_root_header = create_layout_header();
    
    if( m_local_nodetree ) delete m_local_nodetree;
    m_local_nodetree = NULL;
}



//
// 基本レイアウトノード作成
//
LAYOUT* LayoutTree::create_layout( const int& type )
{
    LAYOUT* tmplayout = ( LAYOUT* ) m_heap.heap_alloc( sizeof( LAYOUT ) );
    tmplayout->type = type;
    tmplayout->id_header = m_id_header; 
    tmplayout->id = m_id_layout++;
    tmplayout->header = m_last_header;
    tmplayout->x = -1;
    tmplayout->y = -1;
    
    return tmplayout;
}


//
// ヘッダノード作成
//
LAYOUT* LayoutTree::create_layout_header()
{
    m_id_layout = 0;
    ++m_id_header;
    
    LAYOUT* tmplayout = create_layout( DBTREE::NODE_HEADER );
    m_last_layout = tmplayout;

    if( m_last_header ) m_last_header->next_header = tmplayout;
    m_last_header = tmplayout;
    tmplayout->header = tmplayout;
    
    return tmplayout;
}


//
//　テキストノード作成
//
LAYOUT* LayoutTree::create_layout_text( const char* text, const unsigned char* color_text, bool bold )
{
    LAYOUT* tmplayout = create_layout( DBTREE::NODE_TEXT );
    m_last_layout->next_layout = tmplayout;
    m_last_layout = tmplayout;

    tmplayout->text = text;
    tmplayout->color_text = color_text;
    tmplayout->bold = bold;

    return tmplayout;
}



//
//　リンクノード作成
//
LAYOUT* LayoutTree::create_layout_link( const char* text, const char* link, const unsigned char* color_text, bool bold )
{
    LAYOUT* tmplayout = create_layout_text( text, color_text, bold );

    tmplayout->type = DBTREE::NODE_LINK;
    tmplayout->link = link;

    return tmplayout;
}


// 発言回数(IDの出現数)ノード
//
LAYOUT* LayoutTree::create_layout_idnum( const char* text, const unsigned char* color_text, bool bold )
{
    LAYOUT* tmplayout = create_layout_text( text, color_text, bold );
    tmplayout->type = DBTREE::NODE_IDNUM;

    return tmplayout;
}


//
// 改行ノード作成
//
LAYOUT* LayoutTree::create_layout_br()
{
    LAYOUT* tmplayout = create_layout( DBTREE::NODE_BR );
    m_last_layout->next_layout = tmplayout;
    m_last_layout = tmplayout;

    // DrawAreaBase::set_caret() において layout->text != NULL の場合
    // キャレット移動の計算がうまくいかないのでダミーのテキストバッファをセットする
    tmplayout->text = &m_dummy_str;

    return tmplayout;
}


//
// スペースノード作成
//
LAYOUT* LayoutTree::create_layout_sp( const int& type )
{
    LAYOUT* tmplayout = create_layout( type );
    m_last_layout->next_layout = tmplayout;
    m_last_layout = tmplayout;
    return tmplayout;
}


//
// マージンレベル下げノード作成
//
LAYOUT* LayoutTree::create_layout_downleft()
{
    LAYOUT* tmplayout = create_layout( DBTREE::NODE_DOWN_LEFT );
    m_last_layout->next_layout = tmplayout;
    m_last_layout = tmplayout;
    return tmplayout;
}


//
// nodetreeのノード構造をコピーし、レイアウトツリーの一番最後に加える
//
// joint == true の時はヘッダを作らないで、本文を前のツリーの続きに連結する
//
void LayoutTree::append_node( DBTREE::NODE* node_header, bool joint )
{
    if( ! node_header ) return;
    int res_number = node_header->id_header;

#ifdef _DEBUG
    std::cout << "LayoutTree::append_node num = " << res_number << " show_abone = " << m_show_abone << std::endl;
#endif

    // あぼーん
    if( ! m_article->empty() && ! m_show_abone && m_article->get_abone( res_number ) ){
        append_abone_node( node_header );
        return;
    }

    DBTREE::NODE* tmpnode = node_header;

    // 連結モード
    if( joint ){
        tmpnode = node_header->headinfo->node_body;
        create_layout_br();
    }

    while( tmpnode ){

        LAYOUT* tmplayout = NULL;

        switch( tmpnode->type ){
        
            case DBTREE::NODE_HEADER:
                tmplayout = create_layout_header();
                if( res_number > m_max_res_number ) m_max_res_number = res_number;
                break;
        
            case DBTREE::NODE_TEXT:
                tmplayout = create_layout_text( tmpnode->text, &tmpnode->color_text, tmpnode->bold );
                break;

            case DBTREE::NODE_LINK:
                tmplayout = create_layout_link( tmpnode->text, tmpnode->linkinfo->link,
                                                &tmpnode->color_text, tmpnode->bold );
                break;

            case DBTREE::NODE_IDNUM:
                tmplayout = create_layout_idnum( tmpnode->text, &tmpnode->color_text, tmpnode->bold );
                break;


            case DBTREE::NODE_BR:
                tmplayout = create_layout_br();
                break;
            
            case DBTREE::NODE_ZWSP:
                tmplayout = create_layout_sp( tmpnode->type );
                break;

            case DBTREE::NODE_DOWN_LEFT:
                tmplayout = create_layout_downleft();
                break;
        }

        if( tmplayout ){
            tmplayout->res_number = res_number;
            tmplayout->node = tmpnode;
        }

        tmpnode = tmpnode->next_node;
    }
}



//
// レイアウトツリーの一番最後にあぼーんノード追加
//
void LayoutTree::append_abone_node( DBTREE::NODE* node_header )
{
    int res_number = node_header->id_header;
    if( res_number > m_max_res_number ) m_max_res_number = res_number;

#ifdef _DEBUG
    std::cout << "LayoutTree::append_abone_node num = " << res_number << std::endl;
#endif

    // 透明あぼーん
    if( ! m_show_abone && m_article->get_abone_transparent() ) return;

    LAYOUT* tmplayout;
    DBTREE::NODE* tmpnode;

    tmplayout = create_layout_header();
    tmplayout->res_number = res_number;

    // node_header->next_node == レス番号のリンクヘッダ
    tmpnode = node_header->next_node;
    tmplayout = create_layout_link( tmpnode->text, tmpnode->linkinfo->link, &tmpnode->color_text, tmpnode->bold );

    tmplayout = create_layout_text( " ", NULL, false );
    tmplayout = create_layout_link( "あぼ〜ん", PROTO_ABONE, NULL, false );

    tmplayout = create_layout_br();

    tmplayout = create_layout_downleft();
    tmplayout = create_layout_downleft();

    tmplayout = create_layout_link( "あぼ〜ん", PROTO_ABONE, NULL, false );
}



//
//　HTML追加
//
// 一時的にローカルなノードツリーを作ってHTMLをパースして append_node() で作ったノードをコピー
//
void LayoutTree::append_html( const std::string& html )
{
#ifdef _DEBUG
    std::cout << "LayoutTree::append_html url = " << m_url << " html = " << html << std::endl;
#endif    

    if( ! m_local_nodetree ) m_local_nodetree = new DBTREE::NodeTreeBase( m_url, std::string() );
    DBTREE::NODE* node = m_local_nodetree->append_html( html );
    node->id_header = 0;
    append_node( node, false );
}


//
//　dat追加
//
// 一時的にローカルなノードツリーを作ってdatをパースして append_node() で作ったノードをコピー
// num: レス番号、0なら通し番号で
//
void LayoutTree::append_dat( const std::string& dat, int num )
{
    if( ! m_local_nodetree ) m_local_nodetree = new DBTREE::NodeTreeBase( m_url, std::string() );

    // ダミーのノードを作って番号を調整する
    int res_num = m_local_nodetree->get_res_number();
    if( num && res_num < num  ){
        for(int i = res_num +1 ; i <= num -1; ++i )  m_local_nodetree->append_dat( "<>broken<>\n" );
    }

    DBTREE::NODE* node = m_local_nodetree->append_dat( dat );
    append_node( node, false );
}



//
// 座標 y の下にあるヘッダを返す
//
const LAYOUT* LayoutTree::get_header_at_y( int y )
{
#ifdef _DEBUG
    std::cout << "LayoutTree::get_header_at_y : y = " << y << std::endl;
#endif    
    
    LAYOUT* tmpheader = top_header();
    while( tmpheader ){

        int height_block = tmpheader->next_header ? tmpheader->next_header->y - tmpheader->y : 10000;
        if( tmpheader->y <= y && tmpheader->y + height_block >= y ) return tmpheader;

        tmpheader = tmpheader->next_header;
    }

    return NULL;
}



//
// レス番号 number のヘッダを返す
//
const LAYOUT* LayoutTree::get_header_of_res( int number )
{
#ifdef _DEBUG
    std::cout << "LayoutTree::get_header_of_res num = " << number << std::endl;
#endif  

    LAYOUT* tmpheader = top_header();
    while( tmpheader ){

        if( tmpheader->res_number == number ) return tmpheader;

        tmpheader = tmpheader->next_header;
    }

#ifdef _DEBUG
    std::cout << "not found.\n";
#endif  

    return NULL;
}
