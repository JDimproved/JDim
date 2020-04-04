// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "layouttree.h"

#include "dbtree/interface.h"
#include "dbtree/articlebase.h"
#include "dbtree/nodetreedummy.h"

#include "jdlib/miscutil.h"

#include "config/globalconf.h"

#include "global.h"
#include "colorid.h"
#include "fontid.h"
#include "cssmanager.h"

enum
{
    SIZE_OF_HEAP_DEFAULT = 256 * 1024,
    SIZE_OF_HEAP_REFER_POSTS_LAYOUTTREE = 4096,
    SIZE_OF_HEAP_REFER_POSTS_LOCAL_NODE = 512,

    STEP_ID = 10,
    STEP_SEPARATOR = 1,
    STEP_REFER_POSTS = 5,

    MAX_IMGITEM = 512 // struct IMGDATA.item[] のサイズ
};


// 埋め込み画像用構造体
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


// show_abone : true ならあぼーんしたレスも表示する
// show_multispace : true なら連続空白ノードも表示
LayoutTree::LayoutTree( const std::string& url, const bool show_abone, const bool show_multispace )
    : m_heap_default( SIZE_OF_HEAP_DEFAULT ),
      m_heap_refer_posts_from_newres( SIZE_OF_HEAP_REFER_POSTS_LAYOUTTREE ),
      m_url( url ),
      m_local_nodetree( nullptr ),
      m_separator_header( nullptr ),
      m_show_abone( show_abone ),
      m_show_multispace( show_multispace ),
      m_last_dom_attr( 0 )
{
#ifdef _DEBUG
    std::cout << "LayoutTree::LayoutTree : url = " << url << " show_abone = " << m_show_abone << std::endl;
#endif    

    m_heap = &m_heap_default;
    m_article = DBTREE::get_article( m_url );
    assert( m_article );

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
    m_heap_default.clear();

    m_map_header.clear();
    
    m_last_header = nullptr;
    m_max_res_number = 0;

    m_id_header = -STEP_ID; // ルートヘッダのIDが 0 になるように -STEP_ID を入れておく
    m_root_header = create_layout_header();
    
    if( m_local_nodetree ) delete m_local_nodetree;
    m_local_nodetree = nullptr;

    m_last_div = nullptr;

    // 新着セパレータ作成
    m_separator_new = 0;
    m_separator_new_reserve = 0;
    m_separator_header = create_separator();

    // 新着返信関連削除
    if ( m_local_nodetree_newres ) m_local_nodetree_newres->~NodeTreeBase();
    m_local_nodetree_newres = nullptr;
    m_heap_refer_posts_from_newres.clear();
    m_refer_posts_from_newres_number = 0;
    m_refer_posts_from_newres_header = nullptr;
}


// RECTANGLE型のメモリ確保
RECTANGLE* LayoutTree::create_rect()
{
    RECTANGLE* rect = m_heap->heap_alloc<RECTANGLE>();
    rect->end = true;

    return rect;
}


//
// 基本レイアウトノード作成
//
LAYOUT* LayoutTree::create_layout( const int type )
{
    LAYOUT* tmplayout = m_heap->heap_alloc<LAYOUT>();
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
    m_last_layout = nullptr;
    m_id_layout = 0;
    m_id_header += STEP_ID;

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
LAYOUT* LayoutTree::create_layout_br( const bool nobr )
{
    if( nobr ){
        return create_layout_text( " ", nullptr, false );
    }
    LAYOUT* tmplayout = create_layout( DBTREE::NODE_BR );
    m_last_layout->next_layout = tmplayout;
    m_last_layout = tmplayout;
    return tmplayout;
}


//
// 水平線ノード作成
//
LAYOUT* LayoutTree::create_layout_hr()
{
    LAYOUT* tmplayout = create_layout( DBTREE::NODE_HR );
    m_last_layout->next_layout = tmplayout;
    m_last_layout = tmplayout;
    return tmplayout;
}


//
// 水平スペースノード作成
//
LAYOUT* LayoutTree::create_layout_hspace( const int type )
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

    div->css = m_heap->heap_alloc<CORE::CSS_PROPERTY>();
    *div->css = CORE::get_css_manager()->get_property( id );

    return div;
}


//
// img ノード作成
//
LAYOUT* LayoutTree::create_layout_img( const char* link )
{
    LAYOUT* tmplayout = create_layout( DBTREE::NODE_IMG );
    m_last_layout->next_layout = tmplayout;
    m_last_layout = tmplayout;
    tmplayout->link = link;

    return tmplayout;
}


//
// sssp ノード作成
//
LAYOUT* LayoutTree::create_layout_sssp( const char* link )
{
    LAYOUT* tmplayout = create_layout_img( link );
    tmplayout->type = DBTREE::NODE_SSSP;

    return tmplayout;
}


//
// nodetreeのノード構造をコピーし、レイアウトツリーの一番最後に加える
//
// joint == true の時はヘッダを作らないで、本文を前のツリーの続きに連結する
//
void LayoutTree::append_node( DBTREE::NODE* node_header, const bool joint )
{
    if( ! node_header ) return;

    DBTREE::HEADERINFO* headinfo = node_header->headinfo;
    if( ! headinfo ) return;

    const int res_number = node_header->id_header;

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
        create_layout_br( m_last_dom_attr & CORE::DOMATTR_NOBR );
        append_block( headinfo->block[ DBTREE::BLOCK_MES ], res_number, nullptr, m_last_dom_attr );
    }
    else{

        const CORE::DOM* dom = CORE::get_css_manager()->get_dom();
        m_last_dom_attr = 0;

        IMGDATA imgdata;
        imgdata.num = 0;

        LAYOUT* header = create_layout_header();
        header->res_number = res_number;
        header->node = node_header;
        if( res_number > m_max_res_number ) m_max_res_number = res_number;
        m_map_header[ res_number ] = header;

        while( dom ){
            m_last_dom_attr = dom->attr;

            switch( dom->nodetype ){

                case CORE::DOMNODE_DIV:
                    create_layout_div( dom->dat );
                    break;

                case CORE::DOMNODE_BLOCK:
                    append_block( headinfo->block[ dom->dat ], res_number, &imgdata, dom->attr );
                    break;

                case CORE::DOMNODE_TEXT:
                    create_layout_text( dom->chardat, nullptr, false );
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
                            std::cout << imgdata.item[ i ].link << std::endl;
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
void LayoutTree::append_block( DBTREE::NODE* block, const int res_number, IMGDATA* imgdata, const int dom_attr )
{
    if( ! block ) return;

    DBTREE::NODE* tmpnode = block->next_node;

    while( tmpnode ){

        LAYOUT* tmplayout = nullptr;

        switch( tmpnode->type ){
        
            case DBTREE::NODE_TEXT:
                tmplayout = create_layout_text( tmpnode->text, &tmpnode->color_text, tmpnode->bold );
                break;

            case DBTREE::NODE_LINK:
                tmplayout = create_layout_link( tmpnode->text, tmpnode->linkinfo->link,
                                                &tmpnode->color_text, tmpnode->bold );
                // 画像リンクのurlを集める
                if( imgdata && tmpnode->linkinfo->imglink && imgdata->num < MAX_IMGITEM ){

                    imgdata->item[ imgdata->num ].link = tmpnode->linkinfo->imglink;
                    imgdata->item[ imgdata->num++ ].node = tmpnode;
                }

                break;

            case DBTREE::NODE_SSSP:

                if( CONFIG::get_show_ssspicon() ){
                    tmplayout = create_layout_sssp( tmpnode->linkinfo->link );
                }
                else{
                    // 次が改行ノードの時は飛ばす
                    if( tmpnode->next_node && tmpnode->next_node->type == DBTREE::NODE_BR )
                        tmpnode = tmpnode->next_node;
                }

                break;

            case DBTREE::NODE_IDNUM:
                tmplayout = create_layout_idnum( tmpnode->text, &tmpnode->color_text, tmpnode->bold );
                break;

            case DBTREE::NODE_BR:
                tmplayout = create_layout_br( dom_attr & CORE::DOMATTR_NOBR );
                break;

            case DBTREE::NODE_HR:
                tmplayout = create_layout_hr();
                break;
            
            case DBTREE::NODE_ZWSP: // 幅0スペース
                tmplayout = create_layout_hspace( tmpnode->type );
                break;

            case DBTREE::NODE_MULTISP: // 連続半角スペース
                if( m_show_multispace ) tmplayout = create_layout_text( tmpnode->text, &tmpnode->color_text, tmpnode->bold );
                break;

            case DBTREE::NODE_HTAB: // 水平タブ
                tmplayout = create_layout_hspace( tmpnode->type );
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
    const int res_number = node_header->id_header;
    if( res_number > m_max_res_number ) m_max_res_number = res_number;

#ifdef _DEBUG
    std::cout << "LayoutTree::append_abone_node num = " << res_number << std::endl;
#endif

    // 透明あぼーん
    if( ! m_show_abone && m_article->get_abone_transparent() ) return;

    LAYOUT* head = create_layout_header();
    m_map_header[ res_number ] = head;

    head->res_number = res_number;

    int classid = CORE::get_css_manager()->get_classid( "title" );
    create_layout_div( classid );

    DBTREE::NODE* node = node_header->headinfo->block[ DBTREE::BLOCK_NUMBER ]->next_node;
    create_layout_link( node->text, node->linkinfo->link, &node->color_text, node->bold );
    create_layout_text( " ", nullptr, false );
    create_layout_link( "あぼ〜ん", PROTO_ABONE, nullptr, false );

    classid = CORE::get_css_manager()->get_classid( "mes" );
    create_layout_div( classid );
    create_layout_link( "あぼ〜ん", PROTO_ABONE, nullptr, false );
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

    if( ! m_local_nodetree ) m_local_nodetree = new DBTREE::NodeTreeDummy( m_url );
    DBTREE::NODE* node_header = m_local_nodetree->append_html( html );
    if( !node_header ) {
        return;
    }

    LAYOUT* header = create_layout_header();
    header->node = node_header;

    int classid = CORE::get_css_manager()->get_classid( "comment" );
    *header->css = CORE::get_css_manager()->get_property( classid );

    append_block( node_header->headinfo->block[ DBTREE::BLOCK_MES ], 0 );
}


//
//　dat追加
//
// 一時的にローカルなノードツリーを作ってdatをパースして append_node() で作ったノードをコピー
// num: レス番号、0なら通し番号で
//
void LayoutTree::append_dat( const std::string& dat, int num )
{
    if( dat.empty() ) return;
    if( ! m_local_nodetree ) m_local_nodetree = new DBTREE::NodeTreeBase( m_url, std::string() );

    // ダミーのノードを作って番号を調整する
    int res_num = m_local_nodetree->get_res_number();
    if( num && res_num < num  ){
        for(int i = res_num +1 ; i <= num -1; ++i )  m_local_nodetree->append_dat( "<>broken<>\n" );
    }

    // 改行毎に dat を分割して追加
    std::list< std::string > lines = MISC::get_lines( dat );
    std::list< std::string >::iterator it = lines.begin();
    for( ; it != lines.end(); ++it ){
        if( ! ( *it ).empty() ){
            DBTREE::NODE* node = m_local_nodetree->append_dat( (*it) + "\n" );
            append_node( node, false );
        }
    }
}



//
// レス番号 number のヘッダを返す
//
const LAYOUT* LayoutTree::get_header_of_res_const( const int number ){ return get_header_of_res( number ); }

LAYOUT* LayoutTree::get_header_of_res( const int number )
{
    if( m_map_header.empty() ) return nullptr;
    if( number > m_max_res_number || number <= 0 ) return nullptr;

    return m_map_header[ number ];
}


//
// 新着セパレータ作成
//
LAYOUT* LayoutTree::create_separator()
{
    m_last_layout = nullptr;
    m_id_layout = 0;

    int classid = CORE::get_css_manager()->get_classid( "separator" );
    LAYOUT* header = create_layout_div( classid );
    header->type = DBTREE::NODE_HEADER;

    DBTREE::NODE* node = m_heap->heap_alloc<DBTREE::NODE>();
    node->fontid = FONT_DEFAULT; // デフォルトフォントを設定
    header->node = node;

    if( header->css->bg_color < 0 ) header->css->bg_color = COLOR_SEPARATOR_NEW;

    LAYOUT* layout = create_layout_text( "ここまで読んだ", nullptr, false );
    layout->header = header;

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

    // header_before　と　header_after　の間に挿入する
    LAYOUT* header_before;
    LAYOUT* header_after;

    hide_separator();

    if( ! num ) return;

    // 透明あぼーんしているレスは飛ばす
    int num_tmp = num;
    while( ! ( header_after = get_header_of_res( num_tmp ) ) && num_tmp++ < m_max_res_number );
    if( ! header_after ) return;

    num_tmp = num-1;
    while( ! ( header_before = get_header_of_res( num_tmp ) ) && num_tmp-- > 1 );
    if( ! header_before ) return;

    m_separator_new = num;
    header_before->next_header = m_separator_header;
    m_separator_header->next_header = header_after;

    // セパレータのヘッダID変更
    int id_header = header_before->id_header + STEP_SEPARATOR;
    LAYOUT* layout = m_separator_header;
    while( layout ){
        layout->id_header = id_header;
        layout = layout->next_layout;
    }

#ifdef _DEBUG
    std::cout << "set before = " << num_tmp << " after = " << m_separator_new << std::endl;
#endif    
}


//
// 新着セパレータ消去
//
void LayoutTree::hide_separator()
{
    // 表示中なら取り除く
    if( m_separator_new ){

#ifdef _DEBUG
        std::cout << "LayoutTree::hide_separator num = " << m_separator_new
                  << " next = " << m_separator_header->next_header->res_number << std::endl;
#endif    

        // あぼーんしているレスは飛ばす
        LAYOUT* header_before;
        int num_tmp = m_separator_new -1;;
        while( ! ( header_before = get_header_of_res( num_tmp ) ) && num_tmp-- > 1 );
        if( header_before ) header_before->next_header = m_separator_header->next_header;
    }

    m_separator_new = 0;
}


//
// 新着返信を表示するlayoutを作成
//
void LayoutTree::create_layout_refer_posts_from_newres()
{
    if ( m_refer_posts_from_newres_header ) return;

    // 新着返信のレス取得
    const std::set<int>& refer_posts_from_newres = m_article->get_refer_posts_from_newres();
#ifdef _DEBUG
    std::cout << "LayoutTree::create_layout_refer_posts_from_newres m_separator_new_reserve = " 
        << m_separator_new_reserve << std::endl;
#endif
    if ( m_separator_new_reserve <= 0 ) return;

    std::vector<int> v_newres;
    for ( const int& res_number : refer_posts_from_newres )
    {
        if ( res_number >= m_separator_new_reserve ) v_newres.push_back( res_number );
    }
    if ( v_newres.empty() ) return;

    // 内部 heap切り替え
    m_heap = &m_heap_refer_posts_from_newres;

    // layoutの履歴保存
    LAYOUT* last_layout_preserve = m_last_layout;
    LAYOUT* last_div_preserve = m_last_div;
    int id_layout_preserve = m_id_layout;
    m_last_layout = nullptr;
    m_id_layout = 0;

    // CSSはセパレータのものを流用
    int classid = CORE::get_css_manager()->get_classid( "separator" );
    LAYOUT* header = create_layout_div( classid );
    header->type = DBTREE::NODE_HEADER;

    DBTREE::NODE* node = m_heap->heap_alloc<DBTREE::NODE>();
    node->fontid = FONT_DEFAULT; // デフォルトフォントを設定
    header->node = node;

    if( header->css->bg_color < 0 ) header->css->bg_color = COLOR_SEPARATOR_NEW;

    std::string html;
    html += "新着返信があります";
    for ( const int& res_number : v_newres )
    {
        html += " &gt;&gt;";
        html += std::to_string( res_number );
    }


    m_local_nodetree_newres = m_heap->heap_alloc<DBTREE::NodeTreeBase>();
    m_local_nodetree_newres = new( m_local_nodetree_newres ) DBTREE::NodeTreeBase( m_url, std::string(), SIZE_OF_HEAP_REFER_POSTS_LOCAL_NODE );
    DBTREE::NODE* node_header = m_local_nodetree_newres->append_html( html );

    append_block( node_header->headinfo->block[ DBTREE::BLOCK_MES ], 0 );

    // RECTANGLEのメモリを予め確保
    LAYOUT* layout = header;
    while( layout ){
        layout->rect = create_rect();
        layout = layout->next_layout;
    }

    // heap, layout履歴を元に戻す
    m_heap = &m_heap_default;
    m_last_layout = last_layout_preserve;
    m_last_div = last_div_preserve;
    m_id_layout = id_layout_preserve;

    m_refer_posts_from_newres_header = header;
}


//
// 新着返信表示の削除
//
void LayoutTree::hide_layout_refer_posts_from_newres()
{
    if ( ! m_refer_posts_from_newres_header ) return;
    // 表示中なら取り除く
    if ( ! m_refer_posts_from_newres_number ) return;
#ifdef _DEBUG
    std::cout << "LayoutTree::hide_layout_refer_posts_from_newres num = " << m_refer_posts_from_newres_number << std::endl;
#endif

    // あぼーんしているレスは飛ばす
    LAYOUT* header_before;
    LAYOUT* header_after;
    int num_tmp;

    // 次のレスが読み込まれていて、headerの木構造が変わっているかもしれないので、
    // 後のレスもここで確認する
    num_tmp =  m_refer_posts_from_newres_number;
    while( ! ( header_before = get_header_of_res( num_tmp ) ) && num_tmp-- > 1 );
    num_tmp =  m_refer_posts_from_newres_number + 1;
    while( ! ( header_after = get_header_of_res( num_tmp ) ) && num_tmp++ < m_max_res_number );

    if( header_before )
    {
        if ( header_after ) header_before->next_header = header_after;
        else header_before->next_header = m_refer_posts_from_newres_header->next_header;
    }

    m_refer_posts_from_newres_number = 0;
    m_refer_posts_from_newres_header = nullptr;

    // 内部 heap 清掃
    if ( m_local_nodetree_newres ) m_local_nodetree_newres->~NodeTreeBase();
    m_local_nodetree_newres = nullptr;
    m_heap_refer_posts_from_newres.clear();
}

//
//  新着返信表示の挿入
//
void LayoutTree::insert_layout_refer_posts_from_newres()
{
    if ( ! m_refer_posts_from_newres_header ) return;

#ifdef _DEBUG
    std::cout << "LayoutTree::insert_layout_refer_posts_from_newres max = " << m_max_res_number << std::endl;
#endif

    // header_before の直後につなげる。header_beforeは一番最後のレス。
    LAYOUT* header_before;

    // 透明あぼーんしているレスは飛ばす
    int num_tmp = m_max_res_number;
    while ( ! ( header_before = get_header_of_res( num_tmp ) ) && num_tmp-- > 1 );
    if ( ! header_before ) return;

    m_refer_posts_from_newres_number = num_tmp;

    // ヘッダの木構造をつなぎかえ
    m_refer_posts_from_newres_header->next_header = header_before->next_header;
    header_before->next_header = m_refer_posts_from_newres_header;

    // 新着返信ヘッダID変更
    int id_header = header_before->id_header + STEP_REFER_POSTS;
    LAYOUT* layout = m_refer_posts_from_newres_header;
    while( layout ){
        layout->id_header = id_header;
        layout = layout->next_layout;
    }

#ifdef _DEBUG
    std::cout << "inserted before = " << num_tmp << std::endl;
#endif
}

