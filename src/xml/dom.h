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
        Dom* m_parentNode{};

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
        void copy_childNodes( const Dom& dom ); // dom の子ノードをコピーする

        // ノードを分解して Gtk::TreeStore へ Gtk::TreeModel::Row を追加
        // ただし列は SKELETON::EditColumns を継承したものであること
        void append_treestore( Glib::RefPtr< Gtk::TreeStore >& treestore,
                               SKELETON::EditColumns& columns,
                               std::list< Gtk::TreePath >& list_path_expand,
                               const Gtk::TreeModel::Row& parent = Gtk::TreeModel::Row() ) const;

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
        const std::string& nodeName() const noexcept { return m_nodeName; }
        const std::string& nodeValue() const noexcept { return m_nodeValue; }
        void nodeValue( const std::string& value ) { m_nodeValue = value; }

        // ノード
        // 注意：appendChild() は
        // 戻り値と引数がJavascriptなどのDOMとは異なります。
        //
        // delete忘れを防ぐために外部でnewしない方が良いだろうという
        // 事で、メンバ関数の内部でnewして追加されたノードのポインタ
        // を返すようにしてあります。
        //
        // クラス外で使用していないメンバ関数は削除してあります。

        bool hasChildNodes() const noexcept;
        std::list<Dom*> childNodes() const = delete;
        Dom* firstChild() const;
        Dom* appendChild( const int node_type, const std::string& node_name );
        bool removeChild( Dom* node );
        Dom* emplace_front( int node_type, const std::string& node_name );
        template<class Predicate>
        std::size_t remove_if( Predicate pred );

        // 属性
        std::string getAttribute( const std::string& name ) const;
        bool setAttribute( const std::string& name, const std::string& value );
        bool setAttribute( const std::string& name, const int value );

        std::size_t size() const noexcept { return m_childNodes.size(); }

        auto begin() const noexcept { return m_childNodes.cbegin(); }
        auto end() const noexcept { return m_childNodes.cend(); }
    };

    /** @brief 子ノードのうちpred(child)がtrueを返すものをすべて削除する
     *
     * @details このメンバー関数はchild自体もdeleteして開放する
     * @tparam Predicate bool(const Dom*), trueなら削除する
     * @param[in] pred 削除するかどうかチェックする関数呼び出し可能なオブジェクト
     * @return 削除したノード数
     */
    template<typename Predicate>
    std::size_t Dom::remove_if( Predicate pred )
    {
        std::size_t count = 0;
        for( auto it = m_childNodes.begin(), end = m_childNodes.end(); it != end; ) {
            it = std::find_if( it, end, pred );
            if( it == end ) break;

            Dom* child = *it;
            it = m_childNodes.erase( it );
            delete child;
            ++count;
        }
        return count;
    }
}

#endif
