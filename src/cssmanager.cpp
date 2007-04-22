// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "dbtree/node.h"

#include "jdlib/miscutil.h"
#include "jdlib/miscgtk.h"
#include "jdlib/jdregex.h"

#include "cssmanager.h"
#include "colorid.h"
#include "cache.h"

#define SIZE_OF_HEAP 16 * 1024

CORE::Css_Manager* instance_css_manager = NULL;


CORE::Css_Manager* CORE::get_css_manager()
{
    if( ! instance_css_manager ) instance_css_manager = new Css_Manager();
    assert( instance_css_manager );

    return instance_css_manager;
}


void CORE::delete_css_manager()
{
    if( instance_css_manager ) delete instance_css_manager;
    instance_css_manager = NULL;
}


///////////////////////////////////////////////

using namespace CORE;


enum
{
    SIZETYPE_PX = 0,
    SIZETYPE_EM
};


Css_Manager::Css_Manager()
    : m_heap( SIZE_OF_HEAP ),
      m_last_dom( NULL )
{
#ifdef _DEBUG
    std::cout << "Css_Manager::Css_Manager\n";
#endif

    set_default_css();
    read_css();

    if( ! read_html() ) set_default_html();
}


void Css_Manager::clear_property( CSS_PROPERTY* css )
{
    memset( css, 0, sizeof( CSS_PROPERTY ) );

    css->color = -1;
    css->bg_color = -1;

    css->border_left_color = -1;
    css->border_right_color = -1;
    css->border_top_color = -1;
    css->border_bottom_color = -1;

    css->align = ALIGN_LEFT;
}


//
// クラス名からID取得
//
int Css_Manager::get_classid( const std::string& classname )
{
    int id = 0;

    std::list< std::string >::iterator it = m_css_class.begin();
    for( ; it != m_css_class.end(); ++it, ++id ) if( ( *it ) == classname ) return id;

    return -1;
}


//
// クラス名登録
//
int Css_Manager::register_class( const std::string& classname )
{
#ifdef _DEBUG
    std::cout << "Css_Manager::register_class name = " << classname << std::endl;
#endif

    m_css_class.push_back( classname );

    return get_classid( classname );
}



//
// デフォルトのcssをセット
//
void Css_Manager::set_default_css()
{
    CSS_PROPERTY css;

    ////////////////
    // body
    clear_property( &css );

    css.padding_bottom_em = 2;

    set_property( "body", css );


    /////////////////
    // res
    clear_property( &css );

    css.mrg_left_em = 0.8;
    css.mrg_right_em = 0.8;
    css.mrg_bottom_em = 1;

    set_property( "res", css );


    /////////////////
    // separator
    clear_property( &css );

    css.align = ALIGN_CENTER;

    css.color = COLOR_BACK;

    css.padding_top_px = 4;
    css.padding_bottom_px = 4;

    css.mrg_bottom_em = 1;

    set_property( "separator", css );


    /////////////////
    // comment
    clear_property( &css );

    css.mrg_left_em = 0.8;
    css.mrg_right_em = 0.8;
    css.mrg_bottom_em = 1;

    set_property( "comment", css );


    /////////////////
    // title
    clear_property( &css );

    set_property( "title", css );

    /////////////////
    // mes
    clear_property( &css );

    css.padding_left_em = 1.5;

    set_property( "mes", css );
}



//
// css 読み込み
//

#define GET_PROPVAL() {\
std::string sizestr = regex.str( 2 ); \
size = atof( sizestr.c_str() ); \
type = SIZETYPE_PX; \
if( sizestr.find( "em" ) != std::string::npos ) type = SIZETYPE_EM; \
}while(0)

bool Css_Manager::read_css()
{
    const std::string cssfile = CACHE::path_css();

#ifdef _DEBUG
    std::cout << "Css_Manager::read_css file = " << cssfile << std::endl;
#endif    

    std::string cssdat;
    if( ! CACHE::load_rawdata( cssfile, cssdat ) ) return false;

#ifdef _DEBUG
    std::cout << "css : \n" << cssdat << "--------------\n\n";
#endif

    // '}' でcssを分ける
    std::list< std::string > blocks = MISC::StringTokenizer( cssdat, '}' );

    JDLIB::Regex regex;

    std::list< std::string >::iterator it_block = blocks.begin();
    for( ; it_block != blocks.end(); ++it_block ){

        std::string classname;
        std::string block = MISC::tolower_str( MISC::remove_str( *it_block, " " ) );

        // name取得
        if( regex.exec( "^(body)\\{", block, 0, true ) || regex.exec( "^\\.([^\\{]*)\\{", block, 0, true ) 
            ){

            classname = MISC::remove_space( regex.str( 1 ) );

#ifdef _DEBUG
            std::cout << std::endl << "name = " << classname << std::endl;
#endif
        }

        CSS_PROPERTY css;
        clear_property( &css );

#ifdef _DEBUG
        std::cout << block << "------------------\n" << std::endl;
#endif

        // 各プロパティ取得
        // 改行を消した後に ';' で文字列を分ける
        block = MISC::remove_str( block, "\n" );
        std::list< std::string > props = MISC::StringTokenizer( block, ';' );
        std::list< std::string >::iterator it_prop = props.begin();
        for( ; it_prop != props.end(); ++it_prop ){

            std::string& prop = *it_prop;

            /////////////////////
            // background-color
            if( regex.exec( "background-color:(.*)", prop, 0, true ) ){

                std::string color = regex.str( 1 );

#ifdef _DEBUG
                std::cout << "backgraound-color = " << color << std::endl;
#endif

                m_colors.push_back( MISC::htmlcolor_to_str( color ) );
                css.bg_color = USRCOLOR_BASE + m_colors.size()-1;
            }

            /////////////////////
            // border-color
            else if( regex.exec( "border-([^-]*)-color:(.*)", prop, 0, true ) ){

                std::string mode = regex.str( 1 );
                std::string color = regex.str( 2 );

#ifdef _DEBUG
                std::cout << "border-" << mode << "-color = " << color << std::endl;
#endif
                m_colors.push_back( MISC::htmlcolor_to_str( color ) );
                int colorid = USRCOLOR_BASE + m_colors.size()-1;

                if( mode == "left" ) css.border_left_color = colorid;
                if( mode == "right" ) css.border_right_color = colorid;
                if( mode == "top" ) css.border_top_color = colorid;
                if( mode == "bottom" ) css.border_bottom_color = colorid;
            }

            /////////////////////
            // color
            else if( regex.exec( "color:(.*)", prop, 0, true ) ){

                std::string color = regex.str( 1 );

#ifdef _DEBUG
                std::cout << "color = " << color << std::endl;
#endif

                m_colors.push_back( MISC::htmlcolor_to_str( color ) );
                css.color = USRCOLOR_BASE + m_colors.size()-1;
            }

            /////////////////////
            // text-align
            else if( regex.exec( "text-align:(.*)", prop, 0, true ) ){

                std::string align = regex.str( 1 );

#ifdef _DEBUG
                std::cout << "text-align = " << align << std::endl;
#endif

                if( align == "left" ) css.align = ALIGN_LEFT;
                if( align == "center" ) css.align = ALIGN_CENTER;
                if( align == "right" ) css.align = ALIGN_RIGHT;
            }

            /////////////////////
            // padding
            else if( regex.exec( "padding-([^:]*):(.*)", prop, 0, true ) ){

                std::string mode = regex.str( 1 );
                int type;
                double size;
                GET_PROPVAL();

#ifdef _DEBUG
                std::cout << "padding-" << mode << " size = " << size << " type = " << type << std::endl;
#endif
                if( mode == "left" ) if( type == SIZETYPE_EM ) css.padding_left_em = size; else css.padding_left_px = (int)size;
                if( mode == "right" ) if( type == SIZETYPE_EM ) css.padding_right_em = size; else css.padding_right_px = (int)size;
                if( mode == "top" ) if( type == SIZETYPE_EM ) css.padding_top_em = size; else css.padding_top_px = (int)size;
                if( mode == "bottom" ) if( type == SIZETYPE_EM ) css.padding_bottom_em = size; else css.padding_bottom_px = (int)size;
            }


            /////////////////////
            // mrg
            else if( regex.exec( "mrg-([^:]*):(.*)", prop, 0, true ) ){

                std::string mode = regex.str( 1 );
                int type;
                double size;
                GET_PROPVAL();

#ifdef _DEBUG
                std::cout << "mrg-" << mode << " size = " << size << " type = " << type << std::endl;
#endif
                if( mode == "left" ) if( type == SIZETYPE_EM ) css.mrg_left_em = size; else css.mrg_left_px = (int)size;
                if( mode == "right" ) if( type == SIZETYPE_EM ) css.mrg_right_em = size; else css.mrg_right_px = (int)size;
                if( mode == "top" ) if( type == SIZETYPE_EM ) css.mrg_top_em = size; else css.mrg_top_px = (int)size;
                if( mode == "bottom" ) if( type == SIZETYPE_EM ) css.mrg_bottom_em = size; else css.mrg_bottom_px = (int)size;
            }

            /////////////////////
            // border-width
            else if( regex.exec( "border-([^-]*)-width:(.*)", prop, 0, true ) ){

                std::string mode = regex.str( 1 );
                int type;
                double size;
                GET_PROPVAL();

#ifdef _DEBUG
                std::cout << "border-" << mode << "-width size = " << size << " type = " << type << std::endl;
#endif
                if( mode == "left" ) if( type == SIZETYPE_EM ) css.border_left_width_em = size; else css.border_left_width_px = (int)size;
                if( mode == "right" ) if( type == SIZETYPE_EM ) css.border_right_width_em = size; else css.border_right_width_px = (int)size;
                if( mode == "top" ) if( type == SIZETYPE_EM ) css.border_top_width_em = size; else css.border_top_width_px = (int)size;
                if( mode == "bottom" ) if( type == SIZETYPE_EM ) css.border_bottom_width_em = size; else css.border_bottom_width_px = (int)size;
            }

            /////////////////////
            // border-style
            else if( regex.exec( "border-style:(.*)", prop, 0, true ) ){

                std::string style = regex.str( 1 );

#ifdef _DEBUG
                std::cout << "border-style" << style << std::endl;
#endif
                if( style == "solid" ) css.border_style = BORDER_SOLID;
            }
        }

        set_property( classname, css );
    }

    return true;
}


//
// プロパティ取得
//
CSS_PROPERTY Css_Manager::get_property( const int id )
{
    return m_css[ id ];
}


//
// プロパティをセット
//
void Css_Manager::set_property( const std::string& classname, const CSS_PROPERTY& css )
{
    if( classname.empty() ) return;

#ifdef _DEBUG
    std::cout << "Css_Manager::set_property class = " << classname << std::endl;
#endif

    int id = get_classid( classname );
    if( id < 0 ) id = register_class( classname );

    m_css.erase( id );
    m_css.insert( std::pair< int, CSS_PROPERTY >( id, css ) );
}



//
// 文字の高さを与えてemをセット
//
void Css_Manager::set_size( CSS_PROPERTY* css, double height )
{
    if( ! css ) return;
    if( ! height ) return;

    css->padding_left = 0;
    css->padding_right = 0;
    css->padding_top = 0;
    css->padding_bottom = 0;

    if( css->padding_left_px > 0 ) css->padding_left = css->padding_left_px;
    if( css->padding_right_px > 0 ) css->padding_right = css->padding_right_px;
    if( css->padding_top_px > 0 ) css->padding_top = css->padding_top_px;
    if( css->padding_bottom_px > 0 ) css->padding_bottom = css->padding_bottom_px;

    if( css->padding_left_em > 0 ) css->padding_left = (int)(height * css->padding_left_em);
    if( css->padding_right_em > 0 ) css->padding_right = (int)(height * css->padding_right_em);
    if( css->padding_top_em > 0 ) css->padding_top = (int)(height * css->padding_top_em);
    if( css->padding_bottom_em > 0 ) css->padding_bottom = (int)(height * css->padding_bottom_em);

    ///////////////////////////////////////

    css->mrg_left = 0;
    css->mrg_right = 0;
    css->mrg_top = 0;
    css->mrg_bottom = 0;

    if( css->mrg_left_px > 0 ) css->mrg_left = css->mrg_left_px;
    if( css->mrg_right_px > 0 ) css->mrg_right = css->mrg_right_px;
    if( css->mrg_top_px > 0 ) css->mrg_top = css->mrg_top_px;
    if( css->mrg_bottom_px > 0 ) css->mrg_bottom = css->mrg_bottom_px;

    if( css->mrg_left_em > 0 ) css->mrg_left = (int)(height * css->mrg_left_em);
    if( css->mrg_right_em > 0 ) css->mrg_right = (int)(height * css->mrg_right_em);
    if( css->mrg_top_em > 0 ) css->mrg_top = (int)(height * css->mrg_top_em);
    if( css->mrg_bottom_em > 0 ) css->mrg_bottom = (int)(height * css->mrg_bottom_em);

    ///////////////////////////////////////

    css->border_left_width = 0;
    css->border_right_width = 0;
    css->border_top_width = 0;
    css->border_bottom_width = 0;

    if( css->border_left_width_px > 0 ) css->border_left_width = css->border_left_width_px;
    if( css->border_right_width_px > 0 ) css->border_right_width = css->border_right_width_px;
    if( css->border_top_width_px > 0 ) css->border_top_width = css->border_top_width_px;
    if( css->border_bottom_width_px > 0 ) css->border_bottom_width = css->border_bottom_width_px;

    if( css->border_left_width_em > 0 ) css->border_left_width = (int)(height * css->border_left_width_em);
    if( css->border_right_width_em > 0 ) css->border_right_width = (int)(height * css->border_right_width_em);
    if( css->border_top_width_em > 0 ) css->border_top_width = (int)( height * css->border_top_width_em);
    if( css->border_bottom_width_em > 0 ) css->border_bottom_width = (int)(height * css->border_bottom_width_em);

    ///////////////////////////////////////

    css->padding_left += css->border_left_width;
    css->padding_right += css->border_right_width;
    css->padding_top += css->border_top_width;
    css->padding_bottom += css->border_bottom_width;
}


//
// DOM作成
//
DOM* Css_Manager::create_domnode( int type )
{
    DOM* tmpdom = ( DOM* ) m_heap.heap_alloc( sizeof( DOM ) );
    tmpdom->nodetype = type;

    if( m_last_dom ) m_last_dom->next_dom = tmpdom;
    else m_dom = tmpdom;

    m_last_dom = tmpdom;
    return tmpdom;
}


DOM* Css_Manager::create_divnode( const std::string& classname )
{
    int classid = get_classid( classname );
    if( classid < 0 ) return NULL;

#ifdef _DEBUG
    std::cout << "Css_Manager::create_divnode name = " << classname << std::endl;
#endif

    DOM* tmpdom = create_domnode( DOMNODE_DIV );
    tmpdom->dat = classid;

    return tmpdom;
}


DOM* Css_Manager::create_blocknode( int blockid )
{
    DOM* tmpdom = create_domnode( DOMNODE_BLOCK );
    tmpdom->dat = blockid;

#ifdef _DEBUG
    std::cout << "Css_Manager::create_blocknode id = " << blockid << std::endl;
#endif

    return tmpdom;
}


DOM* Css_Manager::create_textnode( const char* text )
{
    int lng = strlen( text );
    if( ! lng ) return NULL;

#ifdef _DEBUG
    std::cout << "Css_Manager::create_textnode text = " << text << std::endl;
#endif

    DOM* tmpdom = create_domnode( DOMNODE_TEXT );
    tmpdom->chardat = ( char* ) m_heap.heap_alloc( lng );
    strncpy( tmpdom->chardat, text, lng );

    return tmpdom;
}


DOM* Css_Manager::create_imagenode()
{
#ifdef _DEBUG
    std::cout << "Css_Manager::create_imagenode\n";
#endif

    DOM* tmpdom = create_domnode( DOMNODE_IMAGE );

    return tmpdom;
}


//
// デフォルトのhtmlをセット
//
void Css_Manager::set_default_html()
{
    create_divnode( "title" );
    create_blocknode( DBTREE::BLOCK_NUMBER );
    create_textnode( " " );
    create_blocknode( DBTREE::BLOCK_NAME );
    create_textnode( " " );
    create_blocknode( DBTREE::BLOCK_MAIL );
    create_textnode( "： " );
    create_blocknode( DBTREE::BLOCK_DATE );
    create_textnode( " " );
    create_blocknode( DBTREE::BLOCK_ID_NAME );
    create_divnode( "mes" );
    create_blocknode( DBTREE::BLOCK_MES );
    create_imagenode();
}


//
// html読み込み
//
bool Css_Manager::read_html()
{
    const std::string htmlfile = CACHE::path_reshtml();

#ifdef _DEBUG
    std::cout << "Css_Manager::read_html file = " << htmlfile << std::endl;
#endif    

    std::string html;
    if( ! CACHE::load_rawdata( htmlfile, html ) ) return false;

#ifdef _DEBUG
    std::cout << "html : \n" << html << "--------------\n\n";
#endif

    // 改行を消した後に  '>' でhtmlを分ける
    std::list< std::string > blocks = MISC::StringTokenizer( MISC::remove_str( html, "\n" ), '>' );

    JDLIB::Regex regex;

    std::list< std::string >::iterator it_block = blocks.begin();
    for( ; it_block != blocks.end(); ++it_block ){

        std::string block = ( *it_block );

#ifdef _DEBUG
        std::cout << "block = " << block << std::endl;
#endif

        unsigned int pos = block.find( "<" );
        if( pos == std::string::npos ){
            create_textnode( block.c_str() );
            continue;
        }
        else if( pos > 0 ){
            create_textnode( block.substr( 0, pos ).c_str() );
            block = block.substr( pos );
        }

        block = MISC::remove_space( MISC::tolower_str( block.substr( 1 ) ) );

#ifdef _DEBUG
        std::cout << "tag = " << block << std::endl;
#endif

        if( regex.exec( "div +class=\"([^\"]*)\"", block, 0, true ) ){
#ifdef _DEBUG
            std::cout << "name = " << regex.str( 1 ) << std::endl;
#endif
            create_divnode( regex.str( 1 ) );
        }
        else if( block.find( "number" ) == 0 ) create_blocknode( DBTREE::BLOCK_NUMBER );
        else if( block.find( "name" ) == 0 ) create_blocknode( DBTREE::BLOCK_NAME );
        else if( block.find( "mail" ) == 0 ) create_blocknode( DBTREE::BLOCK_MAIL );
        else if( block.find( "date" ) == 0 ) create_blocknode( DBTREE::BLOCK_DATE );
        else if( block.find( "id" ) == 0 ) create_blocknode( DBTREE::BLOCK_ID_NAME );
        else if( block.find( "message" ) == 0 ) create_blocknode( DBTREE::BLOCK_MES );
        else if( block.find( "image" ) == 0 ) create_imagenode();
    }

    return true;
}
