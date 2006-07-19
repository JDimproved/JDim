// ライセンス: 最新のGPL

//
// ノードツリーのノード
//

#ifndef _NODE_H
#define _NODE_H


namespace DBIMG
{
    class Img;
}


namespace DBTREE
{
    // NODE::type
    enum
    {
        NODE_HEADER = 0, // ヘッダ
        NODE_TEXT,  // テキスト
        NODE_LINK,  // リンク
        NODE_BR,     // 改行
        NODE_DOWN_LEFT, // 左マージンのレベルをひとつ下げる。ヘッダの時点でレベル1
        // スペース
        NODE_ZWSP,
        NODE_HAIRSP,
        NODE_THINSP, 
        NODE_ENSP,
        NODE_EMSP,

        NODE_NONE
    };

    struct NODE;

    // ヘッダ拡張情報
    struct HEADERINFO
    {
        NODE* next_header; // 次のヘッダのポインタ

        NODE* node_res;  // レス番号ノードのポインタ(リンクの色を変えるときに必要)
        int num_reference; // 他のレスから参照されている数
        bool abone; // あぼーん

        char* name; // 名前
        
        NODE* node_id_name;  // 名前IDリンクノードのポインタ(IDを取得したり、リンクの色を変えるときに必要)
        int num_id_name; // 同じIDのレスの個数( = 発言数 )
    };


    // リンク情報
    struct LINKINFO
    {
        char* link; // リンクURL

        // アンカー情報
        // anc_from番 から anc_to番までのアンカーになっている
        // anc_from = 0 ならアンカーでは無い
        int anc_from;
        int anc_to;

        // 画像関係の情報
        //
        // 画像リンクの場合、実際にリンクが画面に表示される段階でノードに DBIMG::Img
        // のポインタと色をセットする。
        // image == true かつ img == NULL ならまだ img は未取得
        // 実際にノードが画面に表示された際に img のポインタを取得して画像の状態を取得する
        // 詳しくは  DrawAreaBase::draw_one_node() を参照

        bool image; // 画像かどうか
        DBIMG::Img* img;  // 画像データクラスへのポインタ(危険だが高速化のため直接アクセス、deleteしないこと)
    };

    
    // ノード構造体
    struct NODE
    {
        unsigned char type;

        short id_header; // ヘッダID ( つまりレス番号、ルートヘッダは0 )
        short id; // ヘッダノードから順に 0,1,2,....
        NODE* next_node; // 最終ノードはNULL
        
        char* text;
        unsigned char color_text; // 色
        bool bold;
        
        // ヘッダ拡張情報
        HEADERINFO* headinfo;

        // リンク情報
        LINKINFO* linkinfo;
    };
}


#endif
