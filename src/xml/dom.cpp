// License: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "dom.h"

#include "jdlib/miscutil.h"
#include "type.h"
#include "tools.h"

#include "skeleton/editcolumns.h"

#include <sstream>


namespace XML::dm {
constexpr std::size_t kSizeOfRawData = 2 * 1024 * 1024;
}


using namespace XML;


// staticなHTMLの要素名リストを再定義
std::set< std::string > Dom::m_static_html_elements;


// コンストラクタ
Dom::Dom( const int type, const std::string& name, const bool html )
    : m_html( html )
    , m_nodeType( type )
    , m_nodeName( name )
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
Dom::~Dom() noexcept
{
#ifdef _DEBUG
    std::cout << "~Dom() : " << m_nodeName << ", " << m_childNodes.size() << std::endl;
#endif

    clear();
}

// コピーコンストラクタ
Dom::Dom( const Dom& dom )
    : m_html( dom.m_html )
    , m_nodeType( dom.m_nodeType )
    , m_nodeName( dom.m_nodeName )
    , m_nodeValue( dom.m_nodeValue )
    , m_attributes( dom.m_attributes )
{
    copy_childNodes( dom );
}


//
// 子ノードのクリア
//
void Dom::clear() noexcept
{
    for( Dom* child : m_childNodes )
    {
        if( child ) delete child;
    }
    m_childNodes.clear();
}


//
// XMLの文字列をパースして子ノードを追加する
//
void Dom::parse( const std::string& str )
{
    if( str.empty() || str.size() > dm::kSizeOfRawData ) return;

    size_t current_pos = 0;
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
        const std::size_t tag_lt_pos = str.find( '<', current_pos );

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
            const std::size_t tag_gt_pos = str.find( '>', tag_lt_pos + 1 );

            current_pos = tag_gt_pos + 1;

            // 開始タグの中身( <element attr="value"> )を取り出す
            std::string open_tag = str.substr( tag_lt_pos + 1, tag_gt_pos - tag_lt_pos - 1 );
            if( open_tag.empty() ) continue;

            // タグの中身がアルファベットで始まっているかチェック
            if( !( open_tag[0] >= 'A' && open_tag[0] <= 'Z' ) && !( open_tag[0] >= 'a' && open_tag[0] <= 'z' ) ) {
                continue;
            }

            // タグ構造が壊れてる場合
            size_t broken_pos = 0;
            if( ( broken_pos = open_tag.find( '<' ) ) != std::string::npos )
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
                while( ( close_tag_lt_pos = str.find( '<', current_pos ) ) != std::string::npos
                     && ( close_tag_gt_pos = str.find( '>', close_tag_lt_pos + 1 ) ) != std::string::npos )
                {
                    current_pos = close_tag_gt_pos + 1;

                    // タグの中身を取り出す
                    std::string close_tag = str.substr( close_tag_lt_pos + 1, close_tag_gt_pos - close_tag_lt_pos - 1 );
                    if( m_html ) close_tag = MISC::tolower_str( close_tag );

                    // タグ構造が壊れてる場合
                    if( close_tag.empty() ) continue;
                    else if( ( broken_pos = close_tag.find( '<' ) ) != std::string::npos )
                    {
                         current_pos += broken_pos;
                         continue;
                    }

                    const std::string& tag_name{ m_html ? name : element_name };
                    // 空要素でない同名の開始タグを見つけたらカウントを増やす
                    if( close_tag.back() != '/'
                        && close_tag.rfind( tag_name, 0 ) == 0 ) ++count;
                    // 終了タグを見つけたらカウントを減らす
                    else if( close_tag.front() == '/'
                             && close_tag.compare( 1, tag_name.size(), tag_name ) == 0 ) --count;

                    // 終了タグを見つける必要数が 0 になったらループを抜ける
                    if( count == 0 ) break;
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
        for( const auto& attr : attributes_pair )
        {
            std::cout << "Attribute: " << "name=" << attr.first << ", value=" << attr.second << std::endl;
        }
        std::cout << "nodeValue: " << value << std::endl;
#endif

        // ノードを追加
        Dom* node = appendChild( type, name );
        node->m_attributes = std::move( attributes_pair );
        node->m_nodeValue = std::move( value );

        // 再帰
        if( ! next_source.empty() ) node->parse( next_source );
    }
}


//
// 属性ペアのリストを作成
//
std::map< std::string, std::string > Dom::create_attribute( const std::string& str ) const
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
std::string Dom::get_xml( const int n ) const
{
    std::stringstream xml;

    // インデント
    std::string indent;

    // テキストノードの文字列
    std::string text;

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
            for( const auto& attr : m_attributes )
            {
                xml << " " << attr.first << "=\"" << attr.second << "\"";
            }

            // 子要素がある場合
            if( ! m_childNodes.empty() )
            {
                xml << ">\n";

                for( const Dom* child : m_childNodes )
                {
                    // 子要素をたどる
                    xml << child->get_xml( n + 2 );
                }

                xml << indent << "</" << m_nodeName << ">\n";
            }
            // 空要素の場合
            else xml << " />\n";

            break;

        // テキストノード
        case NODE_TYPE_TEXT:

            text = MISC::ascii_trim( m_nodeValue );
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

            for( const Dom* child : m_childNodes )
            {
                // 子要素をたどる
                xml << child->get_xml();
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
    for( const Gtk::TreeModel::Row& row : children )
    {
        // 各値を取得( skeleton/editcolumns.h を参照 )
        const int type = row[ columns.m_type ];
        const Glib::ustring& url = row[ columns.m_url ];
        const Glib::ustring& data = row[ columns.m_data ];
        const Glib::ustring& name = row[ columns.m_name ];
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
                const Gtk::TreeModel::Children sub_children = row.children();
                if( ! sub_children.empty() ) node->parse( sub_children, columns );
            }
        }
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
                            const Gtk::TreeModel::Row& parent ) const
{
    // ノードの子要素を走査
    for( const Dom* child : m_childNodes )
    {
        const int node_type = child->nodeType();

        if( node_type == NODE_TYPE_ELEMENT )
        {
            const int type = XML::get_type( child->nodeName() );

            if( type != TYPE_UNKNOWN )
            {
                Gtk::TreeModel::Row row;

                // Gtk::TreeStore::append() の追加ポイント
                if( parent ) row = *( treestore->append( parent.children() ) );
                else row = *( treestore->append() );

                // 各値をセット
                columns.setup_row( row, child->getAttribute( "url" ), child->getAttribute( "name" ),
                                   child->getAttribute( "data" ), type, 0 );

                if( type == TYPE_DIR ){

                    row[ columns.m_dirid ] = atoi( child->getAttribute( "dirid" ).c_str() );

                    // 開いているツリーを追加
                    if( child->getAttribute( "open" ) == "y" ) list_path_expand.push_back( treestore->get_path( row ) );
                }

                // 再帰
                if( child->hasChildNodes() ) child->append_treestore( treestore, columns, list_path_expand, row );
            }
        }
    }
}


//
// getElementById()
//
Dom* Dom::getElementById( const std::string& id ) const
{
    Dom* node = nullptr;

    for( Dom* child : m_childNodes )
    {
        if( child->nodeType() == NODE_TYPE_ELEMENT )
        {
            if( child->getAttribute( "id" ) == id ) node = child;
            // 再帰
            else if( child->hasChildNodes() ) node = child->getElementById( id );

            if( node ) break;
        }
    }

    return node;
}


//
// getElementsByTagName()
//
std::list<Dom*> Dom::getElementsByTagName( const std::string& name ) const
{
    std::list<Dom*> domlist;

    for( Dom* child : m_childNodes )
    {
        if( child->nodeType() == NODE_TYPE_ELEMENT )
        {
            if( child->nodeName() == name ) domlist.push_back( child );

            // 再帰
            std::list<Dom*> sub_nodes = child->getElementsByTagName( name );
            domlist.splice( domlist.end(), std::move( sub_nodes ) );
        }
    }

    return domlist;
}


//
// ノード：hasChildNodes
//
bool Dom::hasChildNodes() const noexcept
{
    return ! m_childNodes.empty();
}


//
// dom の子ノードをコピーする
//
void Dom::copy_childNodes( const Dom& dom )
{
    clear();

    for( const Dom* child : dom.m_childNodes )
    {
        Dom* node = new Dom( child->m_nodeType, child->m_nodeName );
        node->m_nodeValue = child->m_nodeValue;
        node->m_parentNode = this;
        node->m_attributes = child->m_attributes;
        node->copy_childNodes( *child );

        m_childNodes.push_back( node );
    }
}


//
// ノード：firstChild
//
Dom* Dom::firstChild() const
{
    if( m_childNodes.empty() ) return nullptr;

    return m_childNodes.front();
}


//
// ノード：appendChild()
//
Dom* Dom::appendChild( const int node_type, const std::string& node_name )
{
    Dom* node = new Dom( node_type, node_name, m_html );
    node->m_parentNode = this;
    m_childNodes.push_back( node );

    return node;
}


//
// ノード：removeChild()
//
bool Dom::removeChild( Dom* node )
{
    if( ! node ) return false;

    m_childNodes.remove( node );
    delete node;

    return true;
}


//
// insertBefore() の機能をフルに使っていなかったためシンプルにした関数を導入する
//
Dom* Dom::emplace_front( const int node_type, const std::string& node_name )
{
    Dom* node = new Dom( node_type, node_name );
    node->m_parentNode = this;
    m_childNodes.push_front( node );
    return node;
}


//
// 属性：getAttribute()
//
std::string Dom::getAttribute( const std::string& name ) const
{
    std::string value;

    if( name.empty() ) return value;

    std::map< std::string, std::string >::const_iterator it = m_attributes.find( name );

    if( it != m_attributes.cend() ) value = MISC::html_unescape( (*it).second );

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

    m_attributes.insert( make_pair( attr_name, std::to_string( value ) ) );

    return true;
}
