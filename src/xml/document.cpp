// License GPL2

//#define _DEBUG
#include "jddebug.h"

#include "document.h"
#include "jdlib/miscutil.h"

#define SIZE_OF_RAWDATA ( 2 * 1024 * 1024 )


using namespace XML;


// 文字列を元にノードツリーを作る場合( htmlはHTMLモード。デフォルト = false )
Document::Document( const std::string& str, const bool html )
    : Dom( NODE_TYPE_DOCUMENT, "#document", html )
{
    if( ! str.empty() ) parse( remove_comments( str ) );
}

// Gtk::TreeStore を元にノードツリーを作る場合
Document::Document( Glib::RefPtr< Gtk::TreeStore > treestore, const std::string& root_name )
    : Dom( NODE_TYPE_DOCUMENT, "#document" )
{
    // XMLとして必要なのでルート要素を作成
    Dom* root = appendChild( NODE_TYPE_ELEMENT, root_name );

    // ルート以下に追加
    root->parse( treestore->children() );
}

// 何も無い状態からノードツリーを作る場合
Document::Document()
    : Dom( NODE_TYPE_DOCUMENT, "#document" )
{
    
}

Document::~Document()
{

}

// このクラスは代入可能
Document& Document::operator=( const Document& document )
{
    if( this == &document ) return *this;

    m_childNodes.clear();

    std::list< Dom* > children = document.m_childNodes;

    std::list< Dom* >::iterator it = children.begin();
    while( it != children.end() )
    {
        Dom* node = new Dom( (*it)->nodeType(), (*it)->nodeName() );
        node->nodeValue( (*it)->nodeValue() );
        node->parentNode( this );
        node->attributes( (*it)->attributes() );
        node->childNodes( (*it)->childNodes() );

        m_childNodes.push_back( node );

        ++it;
    }

    return *this;
}


//
// クリア
//
void Document::clear()
{
    m_childNodes.clear();
}


//
// 初期化
//
void Document::init( const std::string& str, const bool html )
{
    clear();

    if( ! str.empty() ) parse( remove_comments( str ) );
}

void Document::init( Glib::RefPtr< Gtk::TreeStore > treestore,
                      const std::string& root_name )
{
    clear();

    Dom* root = appendChild( NODE_TYPE_ELEMENT, root_name );
    root->parse( treestore->children() );
}


//
// 予めコメントやXML宣言などを取り除いておく( 順番に注意 )
//
std::string Document::remove_comments( const std::string& str )
{
    if( str.size() > SIZE_OF_RAWDATA ) return std::string();

    std::string out_str = str;

    out_str = MISC::remove_str( out_str, "<!--", "-->" ); // <!-- <example /> -->
    out_str = MISC::remove_str( out_str, "<?", "?>" );    // <?xml version="1.0" encoding="UTF-8"?>
    out_str = MISC::remove_str( out_str, "<![", "]>" );   // <![CDATA[ <a href="exmple">*</a> ]]>
    out_str = MISC::remove_str( out_str, "<!", ">" );     // <!DOCTYPE example "*">

    return out_str;
}


//
// Gtk::TreeStore を生成する
//
// list_path_expand = 後で Gtk::TreeView::expand_row() をするためのリスト
//
Glib::RefPtr< Gtk::TreeStore > Document::get_treestore( const std::string& root_name, std::list< Gtk::TreePath >& list_path_expand )
{
    Glib::RefPtr< Gtk::TreeStore > treestore = Gtk::TreeStore::create( m_columns );

    if( ! m_childNodes.empty() )
    {
        // ルートの子要素以下が対象
        Dom* root = get_root_element( root_name );

        // ルート要素の有無で処理を分ける( 旧様式=無, 新様式=有 )
        if( root ) root->append_treestore( treestore, list_path_expand );
        else append_treestore( treestore, list_path_expand );
    }

    return treestore;
}


//
// ルート要素を取得
//
Dom* Document::get_root_element( const std::string& node_name )
{
    Dom* node = 0;

    if( ! this ) return node;

    std::list< Dom* >::iterator it = m_childNodes.begin();
    while( it != m_childNodes.end() )
    {
        if( (*it)->nodeType() == NODE_TYPE_ELEMENT )
        {
            // 要素名問わず
            if( node_name.empty() ) node = *it;
            // 要素名限定
            else if( (*it)->nodeName() == node_name ) node = *it;

            break;
        }
        ++it;
    }

    return node;
}
