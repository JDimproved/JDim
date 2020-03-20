// License: GPL2

// DOM基本クラス

#ifndef _DOM_H
#define _DOM_H

#include <string>
#include <list>
#include <map>
#include <set>
#include <gtkmm.h>


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
        Dom& operator=( const Dom& ) = delete;

        // 属性ペアのリストを作成
        std::map< std::string, std::string > create_attribute( const std::string& str ) const;

      protected:

        // Document をフレンドクラスとする( append_treestore() を public にしたくない )
        friend class Document;

        // コピーコンストラクタ
        Dom( const Dom& dom );

        // パースして子ノードを追加
        void parse( const std::string& str );
        void parse( const Gtk::TreeModel::Children& children, SKELETON::EditColumns& columns );

        // プロパティをセットするアクセッサ
        void parentNode( Dom* parent ) { m_parentNode = parent; }
        void copy_childNodes( const Dom& dom ); // dom の子ノードをコピーする

        // ノードを分解して Gtk::TreeStore へ Gtk::TreeModel::Row を追加
        // ただし列は SKELETON::EditColumns を継承したものであること
        void append_treestore( Glib::RefPtr< Gtk::TreeStore >& treestore,
                               SKELETON::EditColumns& columns,
                                std::list< Gtk::TreePath >& list_path_expand,
                                const Gtk::TreeModel::Row& parnet = Gtk::TreeModel::Row() ) const;

      public:

        // コンストラクタ、デストラクタ
        Dom( const int type, const std::string& name, const bool html = false );
        virtual ~Dom() noexcept;

        // クリア
        void clear() noexcept;

        // XMLタグ構造の文字列を生成
        std::string get_xml( const int n = 0 ) const;

        // getElement(s)By*()
        Dom* getElementById( const std::string& id ) const;
        std::list<Dom*> getElementsByTagName( const std::string& name ) const;

        // プロパティを扱うアクセッサ
        int nodeType() const noexcept { return m_nodeType; }
        std::string nodeName() const { return m_nodeName; }
        std::string nodeValue() const { return m_nodeValue; }
        void nodeValue( const std::string& value ) { m_nodeValue = value; }

        // ノード
        // 注意：appendChild(), replaceChild(), insertBefore() は
        // 戻り値と引数がJavascriptなどのDOMとは異なります。
        //
        // delete忘れを防ぐために外部でnewしない方が良いだろうという
        // 事で、メンバ関数の内部でnewして追加されたノードのポインタ
        // を返すようにしてあります。
        //
        // クラス外で使用していないメンバ関数は削除してあります。

        Dom* ownerDocument() const noexcept = delete;
        Dom* parentNode() const noexcept { return m_parentNode; }
        bool hasChildNodes() const noexcept;
        std::list<Dom*> childNodes() const { return m_childNodes; }
        Dom* firstChild() const;
        Dom* lastChild() const = delete;
        Dom* appendChild( const int node_type, const std::string& node_name );
        bool removeChild( Dom* node );
        Dom* replaceChild( const int node_type, const std::string& node_name, Dom* oldNode ) = delete;
        Dom* insertBefore( const int node_type, const std::string& node_name, Dom* insNode );
        Dom* previousSibling() const = delete;
        Dom* nextSibling() const = delete;

        // 属性
        bool hasAttributes() const noexcept = delete;
        std::map< std::string, std::string > attributes() const = delete;
        void attributes( std::map< std::string, std::string > attributes ) = delete;
        bool hasAttribute( const std::string& name ) const = delete;
        std::string getAttribute( const std::string& name ) const;
        bool setAttribute( const std::string& name, const std::string& value );
        bool setAttribute( const std::string& name, const int value );
        bool removeAttribute( const std::string& name ) = delete;
    };
}

#endif
