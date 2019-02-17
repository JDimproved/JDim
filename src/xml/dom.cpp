// License: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "dom.h"

#include "jdlib/miscutil.h"
#include "type.h"
#include "tools.h"

#include "skeleton/editcolumns.h"

#include <sstream>

enum
{
    SIZE_OF_RAWDATA = 2 * 1024 * 1024
};

using namespace XML;


// staticなHTMLの要素名リストを再定義
std::set< std::string > Dom::m_static_html_elements;


// コンストラクタ
Dom::Dom( const int type, const std::string& name, const bool html )
    : m_html( html ),
      m_nodeType( type ),
      m_nodeName( name ),
      m_parentNode( 0 )
{
    // HTMLの場合に空要素として扱う物の内で、使われていそうな物( 小文字で統一 )
    if( html && m_static_html_elements.empty() )
    {
        m_static_html_elements.insert( "base" );
        m_static_html_elements.insert( "br" );
        m_static_html_elements.insert( "hr" );
        m_static_html_elements.insert( "img" );
        m_static_html_elements.insert( "input" );
        m_static_html_elements.insert( "link" );
        m_static_html_elements.insert( "meta" );
    }
}

// デストラクタ
Dom::~Dom()
{
#ifdef _DEBUG
    std::cout << "~Dom() : " << m_nodeName << ", " << m_childNodes.size() << std::endl;
#endif

    clear();
}

// コピーコンストラクタ
Dom::Dom( const Dom& dom )
    : m_html( dom.m_html ),
      m_nodeType( dom.m_nodeType ),
      m_nodeName( dom.m_nodeName ),
      m_nodeValue( dom.m_nodeValue ),
      m_parentNode( 0 ),
      m_attributes( dom.m_attributes )
{
    copy_childNodes( dom );
}


//
// 子ノードのクリア
//
void Dom::clear()
{
    std::list< Dom* >::iterator it = m_childNodes.begin();
    while( it != m_childNodes.end() )
    {
        if( *it ) delete *it;
        ++it;
    }

    m_childNodes.clear();
}


//
// XMLの文字列をパースして子ノードを追加する
//
void Dom::parse( const std::string& str )
{
    if( str.empty() || str.size() > SIZE_OF_RAWDATA ) return;

    size_t current_pos = 0, tag_lt_pos = 0, tag_gt_pos = 0;
    const size_t str_length = str.length();

    while( current_pos < str_length )
    {
        //プロパティなどに使う変数
        int type = NODE_TYPE_UNKNOWN;
        std::string name;
        std::string value;
        std::map< std::string, std::string > attributes_pair;
        std::string next_source;

        // "<"を探す
        tag_lt_pos = str.find( "<", current_pos );

        // タグの前のテキストノード
        if( current_pos < tag_lt_pos )
        {
            type = NODE_TYPE_TEXT;
            name = "#text";
            value = str.substr( current_pos, tag_lt_pos - current_pos );
            current_pos = tag_lt_pos;
        }
        // 全てがテキストノード
        else if( tag_lt_pos == std::string::npos )
        {
            type = NODE_TYPE_TEXT;
            name = "#text";
            value = str;
            current_pos = str.length();
        }
        // 要素ノード
        else if( current_pos == tag_lt_pos )
        {
            tag_gt_pos = str.find( ">", tag_lt_pos + 1 );

            current_pos = tag_gt_pos + 1;

            // 開始タグの中身( <element attr="value"> )を取り出す
            std::string open_tag = str.substr( tag_lt_pos + 1, tag_gt_pos - tag_lt_pos - 1 );

            // タグの中身がアルファベットで始まっているか
            bool is_alpha = ( ( open_tag[0] >= 'A' && open_tag[0] <= 'Z' ) || ( open_tag[0] >= 'a' && open_tag[0] <= 'z' ) );

            // タグ構造が壊れてる場合
            size_t broken_pos = 0;
            if( open_tag.empty() || ! is_alpha ) continue;
            else if( ( broken_pos = open_tag.find( "<" ) ) != std::string::npos )
            {
                 current_pos += broken_pos;
                 continue;
            }

            // XMLの場合に空要素として扱う要素( <element /> )かどうか
            bool empty_element = false;
            if( ! m_html && open_tag.compare( open_tag.length() - 1, 1, "/" ) == 0 )
            {
                empty_element = true;
                open_tag.erase( open_tag.length() - 1 );
            }

            // 要素名を取り出す
            std::string element_name;
            size_t i = 0;
            const size_t open_tag_length = open_tag.length();
            while( i < open_tag_length
                 && open_tag[i] != '\n'
                 && open_tag[i] != '\t'
                 && open_tag[i] != ' ' )
            {
                element_name += open_tag[i];
                ++i;
            }
            if( element_name.empty() ) continue;

            // 要素ノードのタイプを設定
            type = NODE_TYPE_ELEMENT;

            // 要素名は小文字に統一
            name = MISC::tolower_str( element_name );

            // 属性ペアのリストを作成
            attributes_pair = create_attribute( open_tag.substr( i ) );

            // HTMLの場合に空要素として扱う要素かどうか
            if( m_html && m_static_html_elements.find( name ) != m_static_html_elements.end() ) empty_element = true;

            // 終了タグがある要素からの中身の取り出し( <element>中身</element> )
            if( ! empty_element )
            {
                // count は見つける必要がある終了タグの数
                size_t close_tag_lt_pos = 0, close_tag_gt_pos = 0, count = 1;
                while( ( close_tag_lt_pos = str.find( "<", current_pos ) ) != std::string::npos
                     && ( close_tag_gt_pos = str.find( ">", close_tag_lt_pos + 1 ) ) != std::string::npos )
                {
                    current_pos = close_tag_gt_pos + 1;

                    // タグの中身を取り出す
                    const std::string close_tag = MISC::tolower_str( str.substr( close_tag_lt_pos + 1, close_tag_gt_pos - close_tag_lt_pos - 1 ) );

                    // タグ構造が壊れてる場合
                    if( close_tag.empty() ) continue;
                    else if( ( broken_pos = close_tag.find( "<" ) ) != std::string::npos )
                    {
                         current_pos += broken_pos;
                         continue;
                    }

                    // 空要素でない同名の開始タグを見つけたらカウントを増やす
                    if( close_tag.compare( 0, name.length(), name ) == 0
                     && close_tag.compare( close_tag.length() - 1, 1, "/" ) != 0 ) ++count;
                    // 終了タグを見つけたらカウントを減らす
                    else if( close_tag.compare( 0, name.length() + 1, "/" + name ) == 0 ) --count;

                    // 終了タグを見つける必要数が 0 になったらループを抜ける
                    if( count <= 0 ) break;
                }

                // 必要な終了タグが見付からないまま上記のループを抜けた場合は、全体のループを抜ける
                if( count > 0 ) break;

                // 次の再帰で使う"子ノード"の素材になる文字列
                next_source = str.substr( tag_gt_pos + 1, close_tag_lt_pos - tag_gt_pos - 1 );
             }
        }

#ifdef _DEBUG
        std::cout << "Dom:parse():---------------------------------------\n";
        std::cout << "nodeName : " << name << "\n";
        std::cout << "nodeType : " << type << "\n";
        std::map< std::string, std::string >::iterator it = attributes_pair.begin();
        while( it != attributes_pair.end() )
        {
            std::cout << "Attribute: " << "name=" << (*it).first << ", value=" << (*it).second << std::endl;
            ++it;
        }
        std::cout << "nodeValue: " << value << std::endl;
#endif

        // ノードを追加
        Dom* node = appendChild( type, name );
        node->attributes( attributes_pair );
        node->nodeValue( value );

        // 再帰
        if( ! next_source.empty() ) node->parse( next_source );
    }
}


//
// 属性ペアのリストを作成
//
std::map< std::string, std::string > Dom::create_attribute( const std::string& str )
{
    std::map< std::string, std::string > attributes_pair;

    if( str.empty() ) return attributes_pair;

    size_t i = 0, str_length = str.length();
    while( i < str_length )
    {
        std::string attr_key;
        std::string attr_value;

        // 余分なスペース等を詰める
        while( i < str_length
             && ( str[i] == '\n'
               || str[i] == '\t'
               || str[i] == ' ' ) ) ++i;

        // "="かスペース等でなければ属性名とする
        while( i < str_length
             && str[i] != '\n'
             && str[i] != '\t'
             && str[i] != ' '
             && str[i] != '=' )
        {
            attr_key += str[i];
            ++i;
        }

        // 余分なスペース等を詰める
        while( i < str_length
             && ( str[i] == '\n'
               || str[i] == '\t'
               || str[i] == ' ' ) ) ++i;

        // "="の区切りがある場合のみ属性の値をセットする
        if( str[i] == '=' )
        {
            // "="の分進める
            ++i;

            // 余分なスペース等を詰める
            while( i < str_length
                 && ( str[i] == '\n'
                   || str[i] == '\t'
                   || str[i] == ' ' ) ) ++i;

            // 属性の値 "〜"
            if( str[i] == '"' )
            {
                ++i;
                while( i < str_length && str[i] != '"' )
                {
                    attr_value += str[i];
                    ++i;
                }
                if( str[i] == '"' ) ++i;
            }
            // 属性の値 '〜'
            else if( str[i] == '\'' )
            {
                ++i;
                while( i < str_length && str[i] != '\'' )
                {
                    attr_value += str[i];
                    ++i;
                }
                if( str[i] == '\'' ) ++i;
            }
            // 属性の値 その他( 本来ならXMLとしてエラー )
            else
            {
                while( i < str_length
                     && str[i] != '\n'
                     && str[i] != '\t'
                     && str[i] != ' ' )
                {
                    attr_value += str[i];
                    ++i;
                }
            }
        }

        // 属性を追加
        if( ! attr_key.empty() && ! attr_value.empty() )
        {
            // 属性名は小文字に統一する
            attributes_pair.insert( make_pair( MISC::tolower_str( attr_key ), attr_value ) );
        }
    }

    return attributes_pair;
}


//
// XMLタグ構造の文字列を生成
//
std::string Dom::get_xml( const int n )
{
    std::stringstream xml;

    // インデント
    std::string indent;

    // テキストノードの文字列
    std::string text;

    // 属性
    std::map< std::string, std::string >::iterator attr_it = m_attributes.begin();

    // 子要素
    std::list< Dom* >::iterator child_it = m_childNodes.begin();

    // ノードの種類別に処理
    switch( m_nodeType )
    {
        // 要素
        case NODE_TYPE_ELEMENT:

            // インデントを追加
            for( int i = 0; i < n; ++i ) indent += " ";

            // タグの始まり
            xml << indent << "<" << m_nodeName;

            // 属性を追加
            while( attr_it != m_attributes.end() )
            {
                xml << " " << (*attr_it).first << "=\"" << (*attr_it).second << "\"";
                ++attr_it;
            }

            // 子要素がある場合
            if( ! m_childNodes.empty() )
            {
                xml << ">\n";

                while( child_it != m_childNodes.end() )
                {
                    // 子要素をたどる
                    xml << (*child_it)->get_xml( n + 2 );
                    ++child_it;
                }

                xml << indent << "</" << m_nodeName << ">\n";
            }
            // 空要素の場合
            else xml << " />\n";

            break;

        // テキストノード
        case NODE_TYPE_TEXT:

            text = MISC::remove_spaces( m_nodeValue );
            if( ! text.empty() )
            {
                // インデントを追加
                for( int i = 0; i < n; ++i ) indent += " ";
                xml << indent << text << "\n";
            }

            break;

        // ドキュメントノード
        case NODE_TYPE_DOCUMENT:

            if( ! m_childNodes.empty() ) xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";

            while( child_it != m_childNodes.end() )
            {
                // 子要素をたどる
                xml << (*child_it)->get_xml();
                ++child_it;
            }

            break;
    }

    return xml.str();
}


//
// Gtk::TreeModel::Children からノードツリーを生成
//
// ただし列は SKELETON::EditColumns を継承したものであること
//
void Dom::parse( const Gtk::TreeModel::Children& children, SKELETON::EditColumns& columns )
{
    if( children.empty() ) return;

    // Gtk::TreeModel::Children を走査
    Gtk::TreeModel::iterator it = children.begin();
    while( it != children.end() )
    {
        Gtk::TreeModel::Row row = *it;

        // 各値を取得( skeleton/editcolumns.h を参照 )
        const int type = row[ columns.m_type ];
        const Glib::ustring url = row[ columns.m_url ];
        const Glib::ustring data = row[ columns.m_data ];
        const Glib::ustring name = row[ columns.m_name ];
        const size_t dirid = row[ columns.m_dirid ];
        const bool expand = row[ columns.m_expand ];

        if( type != TYPE_UNKNOWN )
        {
            // タイプにより要素名を決定( board や link など)
            const std::string node_name = XML::get_name( type );

            if( ! node_name.empty() )
            {
                Dom* node = appendChild( NODE_TYPE_ELEMENT, node_name );
                if( type == TYPE_DIR && expand ) node->setAttribute( "open", "y" );
                if( ! name.empty() ) node->setAttribute( "name", name );
                if( ! url.empty() ) node->setAttribute( "url", url );
                if( ! data.empty() ) node->setAttribute( "data", data );
                if( dirid ) node->setAttribute( "dirid", dirid );

                // 再帰
                if( ! row.children().empty() ) node->parse( row.children(), columns );
            }
        }

        ++it;
    }
}


//
// ノードを分解して Gtk::TreeStore へ Gtk::TreeModel::Row を追加
//
// ただし列は SKELETON::EditColumns を継承したものであること
//
// list_path_expand は開いてるツリーを格納するための参照渡し
//
void Dom::append_treestore( Glib::RefPtr< Gtk::TreeStore >& treestore,
                            SKELETON::EditColumns& columns,
                             std::list< Gtk::TreePath >& list_path_expand,
                             const Gtk::TreeModel::Row& parent )
{
    // ノードの子要素を走査
    std::list< Dom* >::iterator it = m_childNodes.begin();
    while( it != m_childNodes.end() )
    {
        const int node_type = (*it)->nodeType();

        if( node_type == NODE_TYPE_ELEMENT )
        {
            const int type = XML::get_type( (*it)->nodeName() );

            if( type != TYPE_UNKNOWN )
            {
                Gtk::TreeModel::Row row;

                // Gtk::TreeStore::append() の追加ポイント
                if( parent ) row = *( treestore->append( parent.children() ) );
                else row = *( treestore->append() );

                // 各値をセット
                columns.setup_row( row, (*it)->getAttribute( "url" ), (*it)->getAttribute( "name" ), (*it)->getAttribute( "data" ), type, 0 );

                if( type == TYPE_DIR ){

                    row[ columns.m_dirid ] = atoi( (*it)->getAttribute( "dirid" ).c_str() );

                    // 開いているツリーを追加
                    if( (*it)->getAttribute( "open" ) == "y" ) list_path_expand.push_back( treestore->get_path( row ) );
                }

                // 再帰
                if( (*it)->hasChildNodes() ) (*it)->append_treestore( treestore, columns, list_path_expand, row );
            }
        }

        ++it;
    }
}



//
// プロパティを扱うアクセッサ
//
int Dom::nodeType()
{
    return m_nodeType;
}

std::string Dom::nodeName()
{
    return m_nodeName;
}

std::string Dom::nodeValue()
{
    return m_nodeValue;
}

void Dom::nodeValue( const std::string& value )
{
    m_nodeValue = value;
}


//
// getElementById()
//
Dom* Dom::getElementById( const std::string& id )
{
    Dom* node = 0;

    std::list< Dom* >::iterator it = m_childNodes.begin();
    while( it != m_childNodes.end() )
    {
        if( (*it)->nodeType() == NODE_TYPE_ELEMENT )
        {
            if( (*it)->getAttribute( "id" ) == id ) node = *it;
            // 再帰
            else if( (*it)->hasChildNodes() ) node = (*it)->getElementById( id );

            if( node ) break;
        }
        ++it;
    }

    return node;
}


//
// getElementsByTagName()
//
DomList Dom::getElementsByTagName( const std::string& name )
{
    DomList domlist;

    std::list< Dom* >::iterator it = m_childNodes.begin();
    while( it != m_childNodes.end() )
    {
        if( (*it)->nodeType() == NODE_TYPE_ELEMENT )
        {
            if( (*it)->nodeName() == name ) domlist.push_back( *it );

            // 再帰
            DomList sub_nodes = (*it)->getElementsByTagName( name );
            domlist.splice( domlist.end(), sub_nodes );
        }
        ++it;
    }

    return domlist;
}


//
// ノード：ownerDocument
//
Dom* Dom::ownerDocument()
{
    Dom* parent = m_parentNode;

    while( parent )
    {
        if( parent->nodeType() == NODE_TYPE_DOCUMENT ) break;
        else parent = parent->parentNode();
    }

    return parent;    
}


//
// ノード：parentNode
//
Dom* Dom::parentNode()
{
    return m_parentNode;
}

void Dom::parentNode( Dom* parent )
{
    m_parentNode = parent;
}


//
// ノード：hasChildNodes
//
bool Dom::hasChildNodes()
{
    return ! m_childNodes.empty();
}


//
// ノード：childNodes
//
DomList Dom::childNodes()
{
    DomList result;

    // DomList に std::list< Dom* > を代入している
    result = m_childNodes;

    return result;
}


//
// dom の子ノードをコピーする
//
void Dom::copy_childNodes( const Dom& dom )
{
    clear();

    DomList children;
    children = dom.m_childNodes;

    if( children.size() ){

        std::list< Dom* >::iterator it = children.begin();
        while( it != children.end() )
        {
            Dom* node = new Dom( (*it)->nodeType(), (*it)->nodeName() );
            node->nodeValue( (*it)->nodeValue() );
            node->parentNode( this );
            node->attributes( (*it)->attributes() );
            node->copy_childNodes( *(*it) );

            m_childNodes.push_back( node );

            ++it;
        }
    }
}


//
// ノード：firstChild
//
Dom* Dom::firstChild()
{
    if( m_childNodes.empty() ) return 0;

    return m_childNodes.front();
}


//
// ノード：lastChild
//
Dom* Dom::lastChild()
{
    if( m_childNodes.empty() ) return 0;

    return m_childNodes.back();
}


//
// ノード：appendChild()
//
Dom* Dom::appendChild( const int node_type, const std::string& node_name )
{
    Dom* node = 0;
    
    {
        node = new Dom( node_type, node_name, m_html );

        node->parentNode( this );

        m_childNodes.push_back( node );
    }

    return node;
}


//
// ノード：removeChild()
//
bool Dom::removeChild( Dom* node )
{
    if( ! node ) return false;

    m_childNodes.remove( node );

    if( node ) delete node;

    return true;
}


//
// ノード：replaceChild()
//
Dom* Dom::replaceChild( const int node_type, const std::string& node_name, Dom* oldNode )
{
    Dom* newNode = 0;

    newNode = new Dom( node_type, node_name );

    std::list< Dom* >::iterator it = m_childNodes.begin();
    while( it != m_childNodes.end() )
    {
        if( *it == oldNode )
        {
            newNode->parentNode( oldNode->parentNode() );
            m_childNodes.erase( it );
            m_childNodes.insert( it, newNode );
            break;
        }
        ++it;
    }

    if( oldNode ) delete oldNode;

    return newNode;
}


//
// ノード：insertBefore()
//
Dom* Dom::insertBefore( const int node_type, const std::string& node_name, Dom* insNode )
{
    Dom* newNode = 0;

    newNode = new Dom( node_type, node_name );

    std::list< Dom* >::iterator it = m_childNodes.begin();
    while( it != m_childNodes.end() )
    {
        if( *it == insNode )
        {
            newNode->parentNode( insNode->parentNode() );
            m_childNodes.insert( it, newNode );
            break;
        }
        ++it;
    }

    return newNode;
}


//
// ノード：previousSibling
//
Dom* Dom::previousSibling()
{
    Dom* previous = 0;

    DomList brothers = m_parentNode->childNodes();

    std::list< Dom* >::iterator it = brothers.begin();
    while( it != brothers.end() )
    {
        if( it != brothers.begin() && *it == this )
        {
            previous = *( --it );
            break;
        }
        ++it;
    }

    return previous;
}


//
// ノード：nextSibling
//
Dom* Dom::nextSibling()
{
    Dom* next = 0;

    DomList brothers = m_parentNode->childNodes();

    std::list< Dom* >::iterator it = brothers.begin();
    while( it != brothers.end() )
    {
        if( *it == this )
        {
            ++it;
            if( it != brothers.end() ) next = *it;
            break;
        }
        ++it;
    }

    return next;
}


//
// 属性：attributes
//
std::map< std::string, std::string > Dom::attributes()
{
    return m_attributes;
}

void Dom::attributes( const std::map< std::string, std::string > attributes )
{
    if( ! attributes.empty() ) m_attributes = attributes;
}


//
// 属性：hasAttributes()
//
bool Dom::hasAttributes()
{
    return ! m_attributes.empty();
}


//
// 属性：hasAttribute()
//
bool Dom::hasAttribute( const std::string& name )
{
    if( name.empty() ) return false;

    return m_attributes.find( name ) != m_attributes.end();
}


//
// 属性：getAttribute()
//
std::string Dom::getAttribute( const std::string& name )
{
    std::string value;

    if( name.empty() ) return value;

    std::map< std::string, std::string >::iterator it = m_attributes.find( name );

    if( it != m_attributes.end() ) value = MISC::html_unescape( (*it).second );

    return value;
}


//
// 属性：setAttribute( std::string )
//
bool Dom::setAttribute( const std::string& name, const std::string& value )
{
    if( name.empty()
     || value.empty()
     || m_nodeType != NODE_TYPE_ELEMENT ) return false;

    // 属性名は小文字に統一
    const std::string attr_name = MISC::tolower_str( name );

    m_attributes.insert( make_pair( attr_name, MISC::html_escape( value ) ) );

    return true;
}


//
// 属性：setAttribute( int )
//
bool Dom::setAttribute( const std::string& name, const int value )
{
    if( name.empty()
     || m_nodeType != NODE_TYPE_ELEMENT ) return false;

    // 属性名は小文字に統一
    const std::string attr_name = MISC::tolower_str( name );

    m_attributes.insert( make_pair( attr_name, MISC::itostr( value ) ) );

    return true;
}


//
// 属性：removeAttribute()
//
bool Dom::removeAttribute( const std::string& name )
{
    bool result = false;

    if( name.empty() ) return result;

    std::map< std::string, std::string >::iterator it = m_attributes.find( name );

    if( it != m_attributes.end() )
    {
        m_attributes.erase( it );
        result = true;
    }

    return result;
}

