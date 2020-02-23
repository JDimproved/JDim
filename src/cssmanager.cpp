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

#include <cstring>

enum
{
    SIZE_OF_HEAP = 16 * 1024
};

CORE::Css_Manager* instance_css_manager = nullptr;


CORE::Css_Manager* CORE::get_css_manager()
{
    if( ! instance_css_manager ) instance_css_manager = new Css_Manager();
    assert( instance_css_manager );

    return instance_css_manager;
}


void CORE::delete_css_manager()
{
    if( instance_css_manager ) delete instance_css_manager;
    instance_css_manager = nullptr;
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
      m_last_dom( nullptr )
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
// ユーザ設定の色取得
// 
std::string Css_Manager::get_color( int colorid ) const
{
    colorid -= USRCOLOR_BASE;
    return m_colors[ colorid ];
}


//
// クラス名からID取得
//
int Css_Manager::get_classid( const std::string& classname ) const
{
    int id = 0;

    std::list< std::string >::const_iterator it = m_css_class.begin();
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


    /////////////////
    // imgpopup
    clear_property( &css );

    css.border_left_width_px = 1;
    css.border_right_width_px = 1;
    css.border_top_width_px = 1;
    css.border_bottom_width_px = 1;

    set_property( "imgpopup", css );
}



//
// css 読み込み
//
bool Css_Manager::read_css()
{
    const std::string css_file = CACHE::path_css();

#ifdef _DEBUG
    std::cout << "Css_Manager::read_css file = " << css_file << std::endl;
#endif    

    std::string css_data;
    if( ! CACHE::load_rawdata( css_file, css_data ) ) return false;

#ifdef _DEBUG
    std::cout << "css : \n" << css_data << "--------------\n\n";
#endif

    // 改行を取り除く
    css_data = MISC::remove_str( css_data, "\n" );
    // コメントを取り除く
    css_data = MISC::remove_str( css_data, "/*", "*/" );

    size_t start_pos = 0, l_pos = 0, r_pos = 0;
    while( ( l_pos = css_data.find( "{", start_pos ) ) != std::string::npos &&
            ( r_pos = css_data.find( "}", l_pos + 1 ) ) != std::string::npos )
    {
        // セレクタ部分を取り出す
        std::string selector = MISC::remove_spaces( css_data.substr( start_pos, l_pos - start_pos ) );
        start_pos = r_pos + 1;
        if( selector.find( "." ) == 0 ) selector.erase( 0, 1 );
        if( selector.empty() ) break;

        // {中身}を取り出す
        std::string range = css_data.substr( l_pos + 1, r_pos - l_pos - 1 );

#ifdef _DEBUG
        std::cout << "selector = " << selector << std::endl;
#endif

        // 中身を";"で分ける
        std::list< std::string > properties = MISC::StringTokenizer( range, ';' );

        // プロパティペア(名前, 値)の作成
        std::map< std::string, std::string > css_pair;
        std::list< std::string >::iterator it = properties.begin();
        while( it != properties.end() )
        {
            size_t colon = (*it).find( ":" );
            std::string key = MISC::remove_spaces( (*it).substr( 0, colon ) );
            std::string value = MISC::remove_spaces( (*it).substr( colon + 1 ) );

#ifdef _DEBUG
            std::cout << "  key = " << key << " value = " << value << std::endl;
#endif

            css_pair.insert( make_pair( key, value ) );

            ++it;
        }

        // 各プロパティを作成
        if( css_pair.size() ) set_property( selector, create_property( css_pair ) );
    }

    return true;
}



// CSS_PROPERTY を作成
#define GET_PROPVAL() {\
std::string sizestr = value; \
size = atof( sizestr.c_str() ); \
type = SIZETYPE_PX; \
if( sizestr.find( "em" ) != std::string::npos ) type = SIZETYPE_EM; \
}while(0)
CSS_PROPERTY Css_Manager::create_property( std::map< std::string, std::string >& css_pair )
{
    CSS_PROPERTY css;
    clear_property( &css );

    JDLIB::Regex regex;
    const size_t offset = 0;
    const bool icase = true; // 大文字小文字区別しない
    const bool newline = true;
    const bool usemigemo = false;
    const bool wchar = false;

    std::map< std::string, std::string >::iterator it = css_pair.begin();
    for( ; it != css_pair.end(); ++it )
    {
        const std::string key = (*it).first;
        const std::string value = (*it).second;

        // background-color
        if( key == "background-color" )
        {

#ifdef _DEBUG
            std::cout << "background-color = " << value << std::endl;
#endif

            m_colors.push_back( MISC::htmlcolor_to_str( value ) );
            css.bg_color = USRCOLOR_BASE + m_colors.size()-1;
        }
        // border-color
        else if( key == "border-color" )
        {

#ifdef _DEBUG
            std::cout << "border-color = " << value << std::endl;
#endif

            m_colors.push_back( MISC::htmlcolor_to_str( value ) );
            int colorid = USRCOLOR_BASE + m_colors.size()-1;

            css.border_left_color = colorid;
            css.border_right_color = colorid;
            css.border_top_color = colorid;
            css.border_bottom_color = colorid;
        }
        // border-*-color
        else if( regex.exec( "border-([a-z]+)-color", key, offset, icase, newline, usemigemo, wchar ) )
        {
            std::string mode = regex.str( 1 );

#ifdef _DEBUG
            std::cout << "border-" << mode << "-color = " << value << std::endl;
#endif
            m_colors.push_back( MISC::htmlcolor_to_str( value ) );
            int colorid = USRCOLOR_BASE + m_colors.size()-1;

            if( mode == "left" ) css.border_left_color = colorid;
            else if( mode == "right" ) css.border_right_color = colorid;
            else if( mode == "top" ) css.border_top_color = colorid;
            else if( mode == "bottom" ) css.border_bottom_color = colorid;
        }
        // border-style
        else if( key == "border-style" )
        {

#ifdef _DEBUG
            std::cout << "border-style = " << value << std::endl;
#endif
            if( value == "solid" ) css.border_style = BORDER_SOLID;
        }
        // border-width
        else if( key == "border-width" )
        {
            int type;
            double size;
            GET_PROPVAL();

#ifdef _DEBUG
            std::cout << "border-width = " << size << std::endl;
#endif

            if( type == SIZETYPE_EM )
            {
                css.border_left_width_em = size;
                css.border_right_width_em = size;
                css.border_right_width_em = size;
                css.border_bottom_width_em = size;
            }
            else
            {
                css.border_left_width_px = (int)size;
                css.border_right_width_px = (int)size;
                css.border_top_width_px = (int)size;
                css.border_bottom_width_px = (int)size;
            }
        }
        // border-*-width
        else if( regex.exec( "border-([a-z]+)-width", key, offset, icase, newline, usemigemo, wchar ) )
        {
            std::string mode = regex.str( 1 );
            int type;
            double size;
            GET_PROPVAL();

#ifdef _DEBUG
            std::cout << "border-" << mode << "-width size = " << size << " type = " << type << std::endl;
#endif
            if( mode == "left" ){ if( type == SIZETYPE_EM ) css.border_left_width_em = size; else css.border_left_width_px = (int)size; }
            else if( mode == "right" ){ if( type == SIZETYPE_EM ) css.border_right_width_em = size; else css.border_right_width_px = (int)size; }
            else if( mode == "top" ){ if( type == SIZETYPE_EM ) css.border_top_width_em = size; else css.border_top_width_px = (int)size; }
            else if( mode == "bottom" ){ if( type == SIZETYPE_EM ) css.border_bottom_width_em = size; else css.border_bottom_width_px = (int)size; }
        }
        // color
        else if( key == "color" )
        {

#ifdef _DEBUG
            std::cout << "color = " << value << std::endl;
#endif

            m_colors.push_back( MISC::htmlcolor_to_str( value ) );
            css.color = USRCOLOR_BASE + m_colors.size()-1;
        }
        // margin
        else if( key == "margin" )
        {
            int type;
            double size;
            GET_PROPVAL();

#ifdef _DEBUG
            std::cout << "margin = " << size << std::endl;
#endif

            if( type == SIZETYPE_EM )
            {
                css.mrg_left_em = size;
                css.mrg_right_em = size;
                css.mrg_right_em = size;
                css.mrg_bottom_em = size;
            }
            else
            {
                css.mrg_left_px = (int)size;
                css.mrg_right_px = (int)size;
                css.mrg_top_px = (int)size;
                css.mrg_bottom_px = (int)size;
            }
        }
        // margin-*
        else if( regex.exec( "margin-([a-z]+)", key, offset, icase, newline, usemigemo, wchar ) )
        {
            std::string mode = regex.str( 1 );
            int type;
            double size;
            GET_PROPVAL();

#ifdef _DEBUG
            std::cout << "margin-" << mode << " size = " << size << " type = " << type << std::endl;
#endif

            if( mode == "left" ){ if( type == SIZETYPE_EM ) css.mrg_left_em = size; else css.mrg_left_px = (int)size; }
            else if( mode == "right" ){ if( type == SIZETYPE_EM ) css.mrg_right_em = size; else css.mrg_right_px = (int)size; }
            else if( mode == "top" ){ if( type == SIZETYPE_EM ) css.mrg_top_em = size; else css.mrg_top_px = (int)size; }
            else if( mode == "bottom" ){ if( type == SIZETYPE_EM ) css.mrg_bottom_em = size; else css.mrg_bottom_px = (int)size; }
        }            
        // padding
        else if( key == "padding" )
        {
            int type;
            double size;
            GET_PROPVAL();

#ifdef _DEBUG
            std::cout << "padding = " << size << std::endl;
#endif

            if( type == SIZETYPE_EM )
            {
                css.padding_left_em = size;
                css.padding_right_em = size;
                css.padding_top_em = size;
                css.padding_bottom_em = size;
            }
            else
            {
                css.padding_left_px = (int)size;
                css.padding_right_px = (int)size;
                css.padding_top_px = (int)size;
                css.padding_bottom_px = (int)size;
            }
        }
        // padding-*
        else if( regex.exec( "padding-([a-z]+)", key, offset, icase, newline, usemigemo, wchar ) )
        {
            std::string mode = regex.str( 1 );
            int type;
            double size;
            GET_PROPVAL();

#ifdef _DEBUG
            std::cout << "padding-" << mode << " size = " << size << " type = " << type << std::endl;
#endif
            if( mode == "left" ){ if( type == SIZETYPE_EM ) css.padding_left_em = size; else css.padding_left_px = (int)size; }
            else if( mode == "right" ){ if( type == SIZETYPE_EM ) css.padding_right_em = size; else css.padding_right_px = (int)size; }
            else if( mode == "top" ){ if( type == SIZETYPE_EM ) css.padding_top_em = size; else css.padding_top_px = (int)size; }
            else if( mode == "bottom" ){ if( type == SIZETYPE_EM ) css.padding_bottom_em = size; else css.padding_bottom_px = (int)size; }
        }
        // text-align
        else if( key == "text-align" )
        {

#ifdef _DEBUG
            std::cout << "text-align = " << value << std::endl;
#endif

            if( value == "left" ) css.align = ALIGN_LEFT;
            else if( value == "center" ) css.align = ALIGN_CENTER;
            else if( value == "right" ) css.align = ALIGN_RIGHT;
        }
    }

    return css;
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
void Css_Manager::set_size( CSS_PROPERTY* css, double height ) const
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
    DOM* tmpdom = m_heap.heap_alloc<DOM>();
    tmpdom->nodetype = type;
    tmpdom->attr = 0;

    if( m_last_dom ) m_last_dom->next_dom = tmpdom;
    else m_dom = tmpdom;

    m_last_dom = tmpdom;
    return tmpdom;
}


DOM* Css_Manager::create_divnode( const std::string& classname )
{
    int classid = get_classid( classname );
    if( classid < 0 ) return nullptr;

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
    if( ! lng ) return nullptr;

#ifdef _DEBUG
    std::cout << "Css_Manager::create_textnode text = " << text << std::endl;
#endif

    DOM* tmpdom = create_domnode( DOMNODE_TEXT );
    tmpdom->chardat = m_heap.heap_alloc<char>( lng + 1 );
    strncpy( tmpdom->chardat, text, lng + 1 );

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
    create_blocknode( DBTREE::BLOCK_NAMELINK );
    create_textnode( "：" );
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
    const size_t offset = 0;
    const bool icase = true; // 大文字小文字区別しない
    const bool newline = true;
    const bool usemigemo = false;
    const bool wchar = false;

    std::list< std::string >::iterator it_block = blocks.begin();
    for( ; it_block != blocks.end(); ++it_block ){

        std::string block = ( *it_block );

#ifdef _DEBUG
        std::cout << "block = " << block << std::endl;
#endif

        std::string::size_type pos = block.find( "<" );
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

        if( regex.exec( "div +class=\"([^\"]*)\"", block, offset, icase, newline, usemigemo, wchar ) ){
#ifdef _DEBUG
            std::cout << "name = " << regex.str( 1 ) << std::endl;
#endif
            create_divnode( regex.str( 1 ) );
        }
        else if( block.find( "number" ) == 0 ) create_blocknode( DBTREE::BLOCK_NUMBER );
        else if( block.find( "namelink" ) == 0 ) create_blocknode( DBTREE::BLOCK_NAMELINK );
        else if( block.find( "name" ) == 0 ) create_blocknode( DBTREE::BLOCK_NAME );
        else if( block.find( "mail" ) == 0 ) create_blocknode( DBTREE::BLOCK_MAIL );
        else if( block.find( "date" ) == 0 ) create_blocknode( DBTREE::BLOCK_DATE );
        else if( block.find( "id" ) == 0 ) create_blocknode( DBTREE::BLOCK_ID_NAME );
        else if( block.find( "message" ) == 0 ){
            DOM* dom = create_blocknode( DBTREE::BLOCK_MES );
            if( block.find( " br=\"no\"" ) != std::string::npos ) dom->attr |= DOMATTR_NOBR;
        }
        else if( block.find( "image" ) == 0 ) create_imagenode();
    }

    return true;
}
