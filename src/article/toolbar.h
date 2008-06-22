// ライセンス: GPL2

// ツールバーのクラス
//
// ARTICLE::ArticleView* 以外では使わない
//

#ifndef _ARTICLE_TOOLBAR_H
#define _ARTICLE_TOOLBAR_H

#include "skeleton/toolbar.h"
#include "skeleton/imgtoolbutton.h"

namespace SKELETON
{
    class ImgToggleToolButton;
}

namespace ARTICLE
{
    class ArticleToolBar : public SKELETON::ToolBar
    {
        std::string m_url_article;

        bool m_enable_slot;

        SKELETON::ImgToolButton m_button_drawout_and;
        SKELETON::ImgToolButton m_button_drawout_or;
        SKELETON::ImgToolButton m_button_clear_hl;

        // 実況
        SKELETON::ImgToggleToolButton* m_button_live_play_stop;

      public:

        ArticleToolBar(); 
        virtual ~ArticleToolBar(){}

        // タブが切り替わった時に呼び出される( Viewの情報を取得する )
        virtual void set_view( SKELETON::View * view );

      protected:

        virtual void pack_buttons();

        // ボタンを押したときのslot関数
        void slot_open_board();

        void slot_drawout_and();
        void slot_drawout_or();
        void slot_clear_highlight();

        void slot_live_play_stop();
    };

    ///////////////////////////

    // 簡易版
    class ArticleToolBarSimple : public SKELETON::ToolBar
    {
      public:

        ArticleToolBarSimple();
        virtual ~ArticleToolBarSimple(){}

      protected:

        virtual void pack_buttons();
    };
}


#endif
