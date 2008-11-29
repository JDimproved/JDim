// ライセンス: GPL2

//
// 共有バッファ
//
// クラス間で情報をやり取りするときに使う
//

#ifndef _SHAREDBUFFER_H
#define _SHAREDBUFFER_H

#include <string>
#include <list>

namespace CORE
{
    // やりとりするデータ
    // リスト型で複数保存可能
    struct DATA_INFO
    {
        int type; // global.hで定義しているデータタイプ
        std::string url;
        std::string name;

        // ユーザー定義
        std::string user1;
        std::string user2;
    };

    int SBUF_size();
    void SBUF_clear_info();
    void SBUF_append( const DATA_INFO& info );
    std::list< DATA_INFO >& SBUF_infolist();
}

#endif
