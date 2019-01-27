// ライセンス: GPL2

// 実際の描画レイアウト時に使うノードのツリー構造クラス


#ifndef _LAYOUTTREE_H
#define _LAYOUTTREE_H

#include <string>

#include "jdlib/refptr_lock.h"
#include "jdlib/heap.h"

#include "cssmanager.h"

namespace DBTREE
{
    class ArticleBase;
    class NodeTreeBase;
    struct NODE;
}

namespace ARTICLE
{
    class EmbeddedImage;
    struct IMGDATA;

    // 描画時のノードの座標情報
    struct RECTANGLE
    {
        bool end;
        RECTANGLE* next_rect; // テキストノードでwrapが起きたらリストで繋ぐ

        int x;
        int y;
        int width;
        int height;
        int align;

        // テキストノードで使用する情報
        int pos_start;
        int n_byte;
        int n_ustr;
    };

    // 描画レイアウト用ノード
    struct LAYOUT
    {
        unsigned char type; // dbtree/node.hで定義されているノードタイプ
        int id_header; // ヘッダ番号、コメントなどもあるので必ずしも id_header = res_number　とはならない
        int id; // ヘッダノードから順に 0,1,2,..
        int res_number; // スレ内のレス番号、0の時はコメント

        LAYOUT* header;      // 親ヘッダーへのポインタ
        LAYOUT* div;         // 所属するdivへのポインタ
        LAYOUT* next_layout; // 次のノード、最終ノードではNULL
        LAYOUT* next_header; // 次のヘッダノード、ヘッダノード以外ではNULL

        RECTANGLE* rect; // 描画時のノード座標、幅、高さ情報
        CORE::CSS_PROPERTY* css; // cssプロパティ

        DBTREE::NODE* node;

        // 文字情報( 実際にはノード情報(DBTREE::NODE)のtext情報へのポインタ)
        const char* text;
        int lng_text; // テキストの長さ
        const char* link;
        const unsigned char* color_text;
        bool bold;

        // 埋め込み画像のポインタ
        // deleteは DrawAreaBase::clear()でおこなう
        EmbeddedImage* eimg;
    };


    // レイアウトツリー
    class LayoutTree
    {
        // 高速化のため直接アクセス
        JDLIB::RefPtr_Lock< DBTREE::ArticleBase > m_article; 

        JDLIB::HEAP m_heap;
        std::string m_url;

        LAYOUT** m_vec_header;  // ヘッダのポインタの配列

        // コメントノードやプレビュー表示時に使うローカルなノードツリー
        DBTREE::NodeTreeBase* m_local_nodetree; 

        LAYOUT* m_root_header;
        LAYOUT* m_last_header;
        LAYOUT* m_last_layout;
        LAYOUT* m_last_div;

        // 新着セパレータ
        LAYOUT* m_separator_header;

        // 新着セパレータの位置(レス番号), 0の時は表示していない
        int m_separator_new;
        int m_separator_new_reserve; // これにレス番号をセットしてから move_separator()を呼ぶ

        // 表示中の最大のレス番号
        int m_max_res_number; 
        
        int m_id_header;
        int m_id_layout;

        // true ならあぼーんしたレスも表示
        bool m_show_abone;

        // true なら連続空白ノードも表示
        bool m_show_multispace;

        // 前回のブロックにあった、DOMノードのattribute
        int m_last_dom_attr;

      public:
        
        // show_abone : true ならあぼーんしたレスも表示する
        // show_multispace : true なら連続空白ノードも表示
        LayoutTree( const std::string& url, const bool show_abone, const bool show_multispace );
        ~LayoutTree();

        void clear();

        // RECTANGLE型のメモリ確保
        RECTANGLE* create_rect();

        LAYOUT* top_header() const { return m_root_header->next_header; }
        const LAYOUT* last_header() const { return m_last_header; }
        int max_res_number() const { return m_max_res_number; }
        const LAYOUT* get_separator() const{ return m_separator_header; }

        // nodetreeのノード構造をコピーし、ツリーの一番最後に加える
        // joint == true の時はヘッダを作らないで、本文を前のツリーの続きに連結する
        void append_node( DBTREE::NODE* node_header, const bool joint );

        void append_block( DBTREE::NODE* block, const int res_number, IMGDATA* imgdata = NULL, const int dom_attr = 0 );

        // html をパースして追加
        void append_html( const std::string& html );

        // dat をパースして追加
        void append_dat( const std::string& dat, int num );

        // レス番号 number のヘッダを返す
        const LAYOUT* get_header_of_res_const( const int number );
        LAYOUT* get_header_of_res( const int number );

        // 新着セパレータ移動
        // set_separator_new()にレス番号をセットしてからmove_separator()を呼ぶ
        void set_separator_new( int num ){ m_separator_new_reserve = num; }
        void move_separator();
        void hide_separator();
        int get_separator_new() const { return m_separator_new; }

      private:
        
        LAYOUT* create_layout( const int type );
        LAYOUT* create_layout_header();
        LAYOUT* create_layout_text( const char* text, const unsigned char* color_text, bool bold );
        LAYOUT* create_layout_link( const char* text, const char* link, const unsigned char* color_text, bool bold );
        LAYOUT* create_layout_idnum( const char* text, const unsigned char* color_text, bool bold );
        LAYOUT* create_layout_br( const bool nobr = false );
        LAYOUT* create_layout_hr();
        LAYOUT* create_layout_hspace( const int type );
        LAYOUT* create_layout_div( const int id );
        LAYOUT* create_layout_img( const char* link );
        LAYOUT* create_layout_sssp( const char* link );

        void append_abone_node( DBTREE::NODE* node_header );
        LAYOUT* create_separator();
    };
}

#endif
