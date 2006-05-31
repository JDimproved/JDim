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

        DrawAreaPopup( const std::string& url );

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


    /////////////////////////////

    // あぼーんされたレスも表示する
    class DrawAreaPopupShowAbone : public ARTICLE::DrawAreaPopup
    {

      public:
        DrawAreaPopupShowAbone( const std::string& url );
    };

}

#endif

