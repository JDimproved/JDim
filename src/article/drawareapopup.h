// ライセンス: 最新のGPL

// ポップアップの表示部

#ifndef _DRAWAREAPOPUP_H
#define _DRAWAREAPOPUP_H

#include "drawareabase.h"

namespace ARTICLE
{
    class DrawAreaPopup : public ARTICLE::DrawAreaBase
    {

      public:

        // show_abone == true ならあぼーんされたスレも表示
        DrawAreaPopup( const std::string& url, bool show_abone );

      protected:

        // レイアウト実行
        virtual void layout();

        // バックスクリーンをDrawAreaにコピー
        virtual bool draw_drawarea();

      private:

        // 背景色
        virtual const int* rgb_color_back();

        // フォント
        virtual const std::string& fontname(); 

        // フォントモード
        virtual const int fontmode();
    };
}

#endif

