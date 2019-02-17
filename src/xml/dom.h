// License: GPL2

// DOM基本クラス

#ifndef _DOM_H
#define _DOM_H

#include <string>
#include <list>
#include <map>
#include <set>
#include <gtkmm.h>

#include "domlist.h"

namespace SKELETON
{
    class EditColumns;
}


namespace XML
{
    // ノードタイプ( m_nodeType )
    enum
    {
        NODE_TYPE_UNKNOWN = 0,
        NODE_TYPE_ELEMENT,
        NODE_TYPE_TEXT,
        NODE_TYPE_DOCUMENT
    };

    class DomList;

    class Dom
    {
        // HTMLの時に空要素として扱う要素を格納する
        static std::set< std::string > m_static_html_elements;

        // HTMLモード有/無
        bool m_html;

        // プロパティ
        int m_nodeType;
        std::string m_nodeName;
        std::string m_nodeValue;
        Dom* m_parentNode;

        // 属性ペアのリスト
        std::map< std::string, std::string > m_attributes;

        // 子要素リスト
        std::list< Dom* > m_childNodes;

      private:

        // このクラスの代入演算子は使わない
        Dom& operator=( const Dom& );

        // 属性ペアのリストを作成
        std::map< std::string, std::string > create_attribute( const std::string& str );

      protected:

        // Document をフレンドクラスとする( append_treestore() を public にしたくない )
        friend class Document;

        // コピーコンストラクタ
        Dom( const Dom& dom );

        // パースして子ノードを追加
        void parse( const std::string& str );
        void parse( const Gtk::TreeModel::Children& children, SKELETON::EditColumns& columns );

        // プロパティをセットするアクセッサ
        void parentNode( Dom* parent );
        void copy_childNodes( const Dom& dom ); // dom の子ノードをコピーする

        // ノードを分解して Gtk::TreeStore へ Gtk::TreeModel::Row を追加
        // ただし列は SKELETON::EditColumns を継承したものであること
        void append_treestore( Glib::RefPtr< Gtk::TreeStore >& treestore,
                               SKELETON::EditColumns& columns,
                                std::list< Gtk::TreePath >& list_path_expand,
                                const Gtk::TreeModel::Row& parnet = Gtk::TreeModel::Row() );

      public:

        // コンストラクタ、デストラクタ
        Dom( const int type, const std::string& name, const bool html = false );
        virtual ~Dom();

        // クリア
        void clear();

        // XMLタグ構造の文字列を生成
        std::string get_xml( const int n = 0 );

        // getElement(s)By*()
        Dom* getElementById( const std::string& id );
        DomList getElementsByTagName( const std::string& name );

        // プロパティを扱うアクセッサ
        int nodeType();
        std::string nodeName();
        std::string nodeValue();
        void nodeValue( const std::string& value );

        // ノード
        // 注意：appendChild(), replaceChild(), insertBefore() は
        // 戻り値と引数がJavascriptなどのDOMとは異なります。
        //
        // delete忘れを防ぐために外部でnewしない方が良いだろうという
        // 事で、メンバ関数の内部でnewして追加されたノードのポインタ
        // を返すようにしてあります。

        Dom* ownerDocument();
        Dom* parentNode();
        bool hasChildNodes();
        DomList childNodes();
        Dom* firstChild();
        Dom* lastChild();
        Dom* appendChild( const int node_type, const std::string& node_name );
        bool removeChild( Dom* node );
        Dom* replaceChild( const int node_type, const std::string& node_name, Dom* oldNode );
        Dom* insertBefore( const int node_type, const std::string& node_name, Dom* insNode );
        Dom* previousSibling();
        Dom* nextSibling();

        // 属性
        bool hasAttributes();
        std::map< std::string, std::string > attributes();
        void attributes( const std::map< std::string, std::string > attributes );
        bool hasAttribute( const std::string& name );
        std::string getAttribute( const std::string& name );
        bool setAttribute( const std::string& name, const std::string& value );
        bool setAttribute( const std::string& name, const int value );
        bool removeAttribute( const std::string& name );
    };
}

#endif
