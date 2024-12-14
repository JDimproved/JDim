// ライセンス: GPL2

// ツールバーのクラス
//
// ARTICLE::ArticleView* 以外では使わない
//

#ifndef _ARTICLE_TOOLBAR_H
#define _ARTICLE_TOOLBAR_H

#include "skeleton/toolbar.h"
#include "skeleton/imgtoolbutton.h"


namespace ARTICLE
{
    class ArticleToolBar : public SKELETON::ToolBar
    {
        std::string m_url_article;

        bool m_enable_slot;

        SKELETON::ImgToolButton m_button_drawout_and;
        SKELETON::ImgToolButton m_button_drawout_or;

        // 実況
        Gtk::ToggleToolButton* m_button_live_play_stop{};

      public:

        ArticleToolBar(); 
        ~ArticleToolBar() noexcept override;

        // タブが切り替わった時に呼び出される( Viewの情報を取得する )
        void set_view( SKELETON::View * view ) override;

        void reload_ui_icon() override;

      protected:

        void pack_buttons() override;

        // ボタンを押したときのslot関数
        void slot_drawout_and();
        void slot_drawout_or();

        void slot_live_play_stop();
    };
}


#endif
