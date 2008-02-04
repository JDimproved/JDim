// ライセンス: GPL2

// ツールバーのクラス
//
// ARTICLE::ArticleView* 以外では使わない
//

#ifndef _ARTICLE_TOOLBAR_H
#define _ARTICLE_TOOLBAR_H

#include "skeleton/toolbar.h"
#include "skeleton/imgbutton.h"

namespace ARTICLE
{
    class ArticleToolBar : public SKELETON::ToolBar
    {
        std::string m_url_article;

        Gtk::Button* m_button_board;

        SKELETON::ImgButton m_button_drawout_and;
        SKELETON::ImgButton m_button_drawout_or;
        SKELETON::ImgButton m_button_clear_hl;

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
    };
}


#endif
