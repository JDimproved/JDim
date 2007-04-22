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
#include "colorid.h"
#include "cssmanager.h"

#define SIZE_OF_HEAP 256 * 1024


// 埋め込み画像用構造体
#define MAX_IMGITEM 512

namespace ARTICLE
{
    struct IMGITEM
    {
        char* link;
        DBTREE::NODE* node;
    };

    struct IMGDATA
    {
        IMGITEM item[ MAX_IMGITEM ];
        int num;
    };
}



using namespace ARTICLE;


// m_show_abone = true ならあぼーんしたレスも表示する
LayoutTree::LayoutTree( const std::string& url, bool show_abone )
    : m_heap( SIZE_OF_HEAP ),
      m_url( url ),
      m_local_nodetree( 0 ),
      m_separator_header( NULL ),
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
    
    m_last_header = NULL;
    m_max_res_number = 0;

    m_id_header = -1; // ルートヘッダのIDが 0 になるように -1 を入れておく
    m_root_header = create_layout_header();
    
    if( m_local_nodetree ) delete m_local_nodetree;
    m_local_nodetree = NULL;

    m_last_div = NULL;

    // 新着セパレータ作成
    m_separator_new = 0;
    m_separator_new_reserve = 0;
    m_separator_header = create_separator();
}


// RECTANGLE型のメモリ確保
RECTANGLE* LayoutTree::create_rect()
{
    RECTANGLE* rect = ( RECTANGLE* ) m_heap.heap_alloc( sizeof( RECTANGLE ) );
    rect->end = true;

    return rect;
}


//
// 基本レイアウトノード作成
//
LAYOUT* LayoutTree::create_layout( const int type )
{
    LAYOUT* tmplayout = ( LAYOUT* ) m_heap.heap_alloc( sizeof( LAYOUT ) );
    tmplayout->type = type;
    tmplayout->id_header = m_id_header; 
    tmplayout->id = m_id_layout++;
    tmplayout->header = m_last_header;
    tmplayout->div = m_last_div;
    
    return tmplayout;
}


//
// ヘッダノード作成
//
// ヘッダ自体もdiv要素
//
LAYOUT* LayoutTree::create_layout_header()
{
    m_last_layout = NULL;
    m_id_layout = 0;
    ++m_id_header;

    int classid = CORE::get_css_manager()->get_classid( "res" );
    LAYOUT* header = create_layout_div( classid );
    header->type = DBTREE::NODE_HEADER;
    m_last_layout = header;

    if( m_last_header ) m_last_header->next_header = header;
    m_last_header = header;
    header->header = header;

    return header;
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
LAYOUT* LayoutTree::create_layout_sp( const int type )
{
    LAYOUT* tmplayout = create_layout( type );
    m_last_layout->next_layout = tmplayout;
    m_last_layout = tmplayout;
    return tmplayout;
}


//
// divノード作成
//
LAYOUT* LayoutTree::create_layout_div( const int id )
{
    LAYOUT* div = create_layout( DBTREE::NODE_DIV );
    if( m_last_layout ) m_last_layout->next_layout = div;
    m_last_layout = div;

    m_last_div = div;

    div->css = ( CORE::CSS_PROPERTY* ) m_heap.heap_alloc( sizeof( CORE::CSS_PROPERTY ) );
    *div->css = CORE::get_css_manager()->get_property( id );

    return div;
}


//
// img ノード作成
//
LAYOUT* LayoutTree::create_layout_img( const char* link )
{
    LAYOUT* img = create_layout( DBTREE::NODE_IMG );
    m_last_layout->next_layout = img;
    m_last_layout = img;
    img->link = link;

    return img;
}


//
// nodetreeのノード構造をコピーし、レイアウトツリーの一番最後に加える
//
// joint == true の時はヘッダを作らないで、本文を前のツリーの続きに連結する
//
void LayoutTree::append_node( DBTREE::NODE* node_header, bool joint )
{
    if( ! node_header ) return;

    DBTREE::HEADERINFO* headinfo = node_header->headinfo;
    if( ! headinfo ) return;

    int res_number = node_header->id_header;

#ifdef _DEBUG
    std::cout << "LayoutTree::append_node num = " << res_number << " show_abone = " << m_show_abone << std::endl;
#endif

    // あぼーん
    if( ! m_article->empty() && ! m_show_abone && m_article->get_abone( res_number ) ){
        append_abone_node( node_header );
        return;
    }

    // 連結モード
    // 本文ブロックだけ追加
    if( joint ){
        create_layout_br();
        append_block( headinfo->block[ DBTREE::BLOCK_MES ], res_number, NULL );
    }
    else{

        const CORE::DOM* dom = CORE::get_css_manager()->get_dom();

        IMGDATA imgdata;
        imgdata.num = 0;

        LAYOUT* header = create_layout_header();
        header->res_number = res_number;
        header->node = node_header;
        if( res_number > m_max_res_number ) m_max_res_number = res_number;

        while( dom ){

            switch( dom->nodetype ){

                case CORE::DOMNODE_DIV:
                    create_layout_div( dom->dat );
                    break;

                case CORE::DOMNODE_BLOCK:
                    append_block( headinfo->block[ dom->dat ], res_number, &imgdata );
                    break;

                case CORE::DOMNODE_TEXT:
                    create_layout_text( dom->chardat, NULL, false );
                    break;

                case CORE::DOMNODE_IMAGE: // インライン画像

                    if( imgdata.num && CONFIG::get_use_inline_image() ){

#ifdef _DEBUG
                        std::cout << "LayoutTree::append_node : create image\n";
#endif

                        create_layout_br();
                        create_layout_br();

                        for( int i = 0; i < imgdata.num; ++i ){
#ifdef _DEBUG
                            std::cout << imgdata[ i ].link << std::endl;
#endif
                            LAYOUT* tmplayout = create_layout_img( imgdata.item[ i ].link );
                            tmplayout->res_number = res_number;
                            tmplayout->link = imgdata.item[ i ].link;
                            tmplayout->node = imgdata.item[ i ].node;
                        }
                    }

                    break;

            }

            dom = dom->next_dom;
        }

    }
}


//
// 名前やメールなどのブロックのコピー
//
void LayoutTree::append_block( DBTREE::NODE* block, const int res_number, IMGDATA* imgdata )
{
    if( ! block ) return;

    DBTREE::NODE* tmpnode = block->next_node;

    while( tmpnode ){

        LAYOUT* tmplayout = NULL;

        switch( tmpnode->type ){
        
            case DBTREE::NODE_TEXT:
                tmplayout = create_layout_text( tmpnode->text, &tmpnode->color_text, tmpnode->bold );
                break;

            case DBTREE::NODE_LINK:
                tmplayout = create_layout_link( tmpnode->text, tmpnode->linkinfo->link,
                                                &tmpnode->color_text, tmpnode->bold );
                // 画像リンクのurlを集める
                if( imgdata && tmpnode->linkinfo->image && imgdata->num < MAX_IMGITEM ){
                    imgdata->item[ imgdata->num ].link = tmpnode->linkinfo->link;
                    imgdata->item[ imgdata->num++ ].node = tmpnode;
                }

                break;


            case DBTREE::NODE_IDNUM:
                tmplayout = create_layout_idnum( tmpnode->text, &tmpnode->color_text, tmpnode->bold );
                break;

            case DBTREE::NODE_BR:
                tmplayout = create_layout_br();
                break;
            
            case DBTREE::NODE_ZWSP: // 幅0スペース
                tmplayout = create_layout_sp( tmpnode->type );
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

    LAYOUT* head = create_layout_header();
    head->res_number = res_number;

    int classid = CORE::get_css_manager()->get_classid( "title" );
    create_layout_div( classid );

    DBTREE::NODE* node = node_header->headinfo->block[ DBTREE::BLOCK_NUMBER ]->next_node;
    create_layout_link( node->text, node->linkinfo->link, &node->color_text, node->bold );
    create_layout_text( " ", NULL, false );
    create_layout_link( "あぼ〜ん", PROTO_ABONE, NULL, false );

    classid = CORE::get_css_manager()->get_classid( "mes" );
    create_layout_div( classid );
    create_layout_link( "あぼ〜ん", PROTO_ABONE, NULL, false );
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
    DBTREE::NODE* node_header = m_local_nodetree->append_html( html );

    LAYOUT* header = create_layout_header();
    header->node = node_header;

    int classid = CORE::get_css_manager()->get_classid( "comment" );
    *header->css = CORE::get_css_manager()->get_property( classid );

    append_block( node_header->headinfo->block[ DBTREE::BLOCK_MES ], 0, NULL );
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
// レス番号 number のヘッダを返す
//
const LAYOUT* LayoutTree::get_header_of_res_const( int number ){ return get_header_of_res( number ); }

LAYOUT* LayoutTree::get_header_of_res( int number )
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


//
// 新着セパレータ作成
//
LAYOUT* LayoutTree::create_separator()
{
    int classid = CORE::get_css_manager()->get_classid( "separator" );
    LAYOUT* header = create_layout_div( classid );
    header->type = DBTREE::NODE_HEADER;

    if( header->css->bg_color < 0 ) header->css->bg_color = COLOR_SEPARATOR_NEW;

    create_layout_text( "ここまで読んだ", NULL, false );

    return header;
}


//
// 新着セパレータ移動
//
void LayoutTree::move_separator()
{
    int num = m_separator_new_reserve;

#ifdef _DEBUG
    std::cout << "LayoutTree::set_separator num = " << num << " max = " << m_max_res_number << std::endl;
#endif    

    if( num == m_separator_new ) return;

    LAYOUT* header_before;
    LAYOUT* header_after;

    // 表示中なら取り除く
    if( m_separator_new ){
        header_before = get_header_of_res( m_separator_new -1 );
        if( header_before ) header_before->next_header = m_separator_header->next_header;

#ifdef _DEBUG
        std::cout << "removed num = " << m_separator_new << std::endl;
#endif    
    }

    m_separator_new = 0;

    if( ! num ) return;

    // あぼーんしているレスは飛ばす
    int num_tmp = num;
    while( ! ( header_after = get_header_of_res( num_tmp ) ) && num_tmp++ < m_max_res_number );
    if( ! header_after ) return;

    num_tmp = num-1;
    while( ! ( header_before = get_header_of_res( num_tmp ) ) && num_tmp-- > 1 );
    if( ! header_before ) return;

    header_before->next_header = m_separator_header;
    m_separator_header->next_header = header_after;
    m_separator_new = num;

#ifdef _DEBUG
    std::cout << "set before = " << num_tmp << " after = " << m_separator_new << std::endl;
#endif    
}
