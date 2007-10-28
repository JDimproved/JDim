// ライセンス: GPL2

// ツールバーのクラス

#ifndef _BOARD_TOOLBAR_H
#define _BOARD_TOOLBAR_H

#include <gtkmm.h>

#include "skeleton/toolbar.h"
#include "skeleton/compentry.h"

namespace BOARD
{
    class BoardToolBar : public SKELETON::ToolBar
    {
        friend class BoardView;

        SKELETON::SearchEntry m_entry_search;
        SKELETON::ImgButton m_button_reload;
        SKELETON::ImgButton m_button_delete;
        SKELETON::ImgButton m_button_stop;
        SKELETON::ImgButton m_button_favorite;
        SKELETON::ImgButton m_button_up_search;
        SKELETON::ImgButton m_button_down_search;
        SKELETON::ImgButton m_button_new_article;

      public:

        BoardToolBar();
        virtual ~BoardToolBar(){}

      protected:

        virtual void pack_buttons();
    };
}


#endif
