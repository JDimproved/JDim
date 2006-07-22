// ライセンス: 最新のGPL

// 実際の描画レイアウト時に使うノードのツリー構造クラス


#ifndef _LAYOUTTREE_H
#define _LAYOUTTREE_H

#include <string>

#include "jdlib/refptr_lock.h"
#include "jdlib/heap.h"


namespace DBTREE
{
    class ArticleBase;
    class NodeTreeBase;
    struct NODE;
}

        
namespace ARTICLE
{
    // 描画レイアウト用ノード
    struct LAYOUT
    {
        unsigned char type; // dbtree/node.hで定義されてるノードタイプ
        int id_header; // ヘッダ番号、コメントなどもあるので必ずしも id_header = res_number　とはならない
        int id; // ヘッダノードから順に 0,1,2,..
        int res_number; // スレ内のレス番号、0の時はコメント

        LAYOUT* header;      // 親ヘッダーへのポインタ
        LAYOUT* next_layout; // 次のノード、最終ノードではNULL
        LAYOUT* next_header; // 次のヘッダノード、ヘッダノード以外ではNULL

        // レイアウト用変数
        unsigned mrg_level;  // 字下げレベル
        int x;
        int y;
        int width;
        int height;

        // 文字情報( 実際にはノード情報(DBTREE::NODE)のtext情報へのポインタ)
        const char* text;
        const char* link;
        const unsigned char* color_text;
        bool bold;

        // ノード情報へのポインタ
        DBTREE::NODE* node;
    };


    // レイアウトツリー
    class LayoutTree
    {
        // 高速化のため直接アクセス
        JDLIB::RefPtr_Lock< DBTREE::ArticleBase > m_article; 

        JDLIB::HEAP m_heap;
        std::string m_url;

        // コメントノードやプレビュー表示時に使うローカルなノードツリー
        DBTREE::NodeTreeBase* m_local_nodetree; 

        // 改行レイアウトノードなどで使うダミーのテキストバッファ
        // 詳しくは create_layout_br() の説明を見よ
        char m_dummy_str;

        LAYOUT* m_root_header;
        LAYOUT* m_last_header;
        LAYOUT* m_last_layout;

        // 表示中の最大のレス番号
        int m_max_res_number; 
        
        int m_id_header;
        int m_id_layout;

        // true ならあぼーんしたレスも表示
        bool m_show_abone;
        
      public:
        
        // m_show_abone = true ならあぼーんしたレスも表示する
        LayoutTree( const std::string& url, bool show_abone );
        ~LayoutTree();

        void clear();

        LAYOUT* top_header() const { return m_root_header->next_header; }
        const LAYOUT* last_header() const { return m_last_header; }
        const int max_res_number() const { return m_max_res_number; }

        // nodetreeのノード構造をコピーし、ツリーの一番最後に加える
        // joint == true の時はヘッダを作らないで、本文を前のツリーの続きに連結する
        void append_node( DBTREE::NODE* node_header, bool joint );

        // html をパースして追加
        void append_html( const std::string& html );

        // dat をパースして追加
        void append_dat( const std::string& dat, int num );


        // 座標 y の下にあるヘッダ
        const LAYOUT* get_header_at_y( int y );

        // レス番号 number のヘッダを返す
        const LAYOUT* get_header_of_res( int number );

      private:
        
        LAYOUT* create_layout( const int& type );
        LAYOUT* create_layout_header();
        LAYOUT* create_layout_text( const char* text, const unsigned char* color_text, bool bold );
        LAYOUT* create_layout_link( const char* text, const char* link, const unsigned char* color_text, bool bold );
        LAYOUT* create_layout_br();
        LAYOUT* create_layout_sp( const int& type );
        LAYOUT* create_layout_downleft();

        void append_abone_node( DBTREE::NODE* node_header );
    };
}

#endif
