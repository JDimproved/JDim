// License GPL2

// Document( Dom派生 )クラス

#ifndef _DOCUMENT_H
#define _DOCUMENT_H

#include "dom.h"


namespace XML
{
    class Document : public Dom
    {
        // コメントなどを取り除く
        std::string remove_comments( const std::string& str );

        // コピーコンストラクタは使わない
        Document( const Document& );

      public:

        // 文字列を元にノードツリーを作る場合( html = HTMLモード )
        Document( const std::string& str, const bool html = false );

        // Gtk::TreeStore を元にノードツリーを作る場合
        // ただし列は SKELETON::EditColumns を継承したものであること
        Document( Glib::RefPtr< Gtk::TreeStore > treestore, SKELETON::EditColumns& columns, const std::string& root_name );

        // 何も無い状態からノードツリーを作る場合
        Document();

        ~Document() noexcept {}

        // このクラスは代入可能
        Document& operator=( const Document& document );

        // 初期化
        void init( const std::string& str );
        void init( Glib::RefPtr< Gtk::TreeStore > treestore,
                   SKELETON::EditColumns& columns,
                    const std::string& root_name );

        // Gtk::TreeStore をセットする
        // ただし列は SKELETON::EditColumns を継承したものであること
        void set_treestore( Glib::RefPtr< Gtk::TreeStore >& treestore, SKELETON::EditColumns& columns,
                            const std::string& root_name, std::list< Gtk::TreePath >& list_path_expand );

        // ルート要素を取得する
        Dom* get_root_element( const std::string& node_name = std::string() );
    };
}

#endif
