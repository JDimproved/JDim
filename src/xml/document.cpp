// License GPL2

//#define _DEBUG
#include "jddebug.h"

#include "document.h"
#include "jdlib/miscutil.h"


namespace XML::dc {
constexpr std::size_t kSizeOfRawData = 2 * 1024 * 1024;
}


using namespace XML;


// 文字列を元にノードツリーを作る場合( htmlはHTMLモード。デフォルト = false )
Document::Document( const std::string& str, const bool html )
    : Dom( NODE_TYPE_DOCUMENT, "#document", html )
{
    init( str );
}


// Gtk::TreeStore を元にノードツリーを作る場合
Document::Document( Glib::RefPtr< Gtk::TreeStore > treestore, SKELETON::EditColumns& columns, const std::string& root_name )
    : Dom( NODE_TYPE_DOCUMENT, "#document" )
{
    init( treestore, columns, root_name );
}


// 何も無い状態からノードツリーを作る場合
Document::Document()
    : Dom( NODE_TYPE_DOCUMENT, "#document" )
{}


//
// このクラスは代入可能
//
Document& Document::operator=( const Document& document )
{
    if( this == &document ) return *this;
    copy_childNodes( document );

    return *this;
}

//
// 初期化
//
void Document::init( const std::string& str )
{
    clear();

    if( ! str.empty() ) parse( remove_comments( str ) );
}

void Document::init( Glib::RefPtr< Gtk::TreeStore > treestore, SKELETON::EditColumns& columns, const std::string& root_name )
{
    clear();

    // XMLとして必要なのでルート要素を作成
    Dom* root = appendChild( NODE_TYPE_ELEMENT, root_name );

    // ルート以下に追加
    root->parse( treestore->children(), columns );
}


//
// 予めコメントやXML宣言などを取り除いておく( 順番に注意 )
//
std::string Document::remove_comments( const std::string& str )
{
    if( str.size() > dc::kSizeOfRawData ) return std::string();

    std::string out_str = str;

    out_str = MISC::remove_str( out_str, "<!--", "-->" ); // <!-- <example /> -->
    out_str = MISC::remove_str( out_str, "<?", "?>" );    // <?xml version="1.0" encoding="UTF-8"?>
    out_str = MISC::remove_str( out_str, "<![", "]>" );   // <![CDATA[ <a href="exmple">*</a> ]]>
    out_str = MISC::remove_str( out_str, "<!", ">" );     // <!DOCTYPE example "*">

    return out_str;
}


//
// Gtk::TreeStore をセット
//
// list_path_expand = 後で Gtk::TreeView::expand_row() をするためのリスト
//
void Document::set_treestore( Glib::RefPtr< Gtk::TreeStore >& treestore, SKELETON::EditColumns& columns,
                              const std::string& root_name, std::list< Gtk::TreePath >& list_path_expand ) const
{
    treestore->clear();

    if( size() )
    {
        // ルートの子要素以下が対象
        const Dom* root = get_root_element( root_name );

        // ルート要素の有無で処理を分ける( 旧様式=無, 新様式=有 )
        if( root ) root->append_treestore( treestore, columns, list_path_expand );
        else append_treestore( treestore, columns, list_path_expand );
    }
}


//
// ルート要素を取得
//
// node_name : 要素名で限定、空の時は要素名問わず取得
//
static Dom* get_root_element_impl( const std::list<Dom*>& children, const std::string& node_name )
{
    auto it = std::find_if( children.cbegin(), children.cend(),
                            []( const Dom* c ) { return c->nodeType() == NODE_TYPE_ELEMENT; } );

    if( it != children.cend()
        && ( node_name.empty() || (*it)->nodeName() == node_name ) ) return *it;
    return nullptr;
}


Dom* Document::get_root_element( const std::string& node_name )
{
    // Domのfriend classなのでアクセス可能
    return get_root_element_impl( m_childNodes, node_name );
}


const Dom* Document::get_root_element( const std::string& node_name ) const
{
    return get_root_element_impl( m_childNodes, node_name );
}
