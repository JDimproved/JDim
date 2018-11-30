// ライセンス: GPL2

// メインツールバークラス
//
// CORE::Core 以外では使わない
//

#ifndef _MAIN_TOOLBAR_H
#define _MAIN_TOOLBAR_H

#include <gtkmm.h>

#include "skeleton/toolbar.h"
#include "skeleton/imgtoolbutton.h"
#include "skeleton/imgtoggletoolbutton.h"

namespace CORE
{
    class MainToolBar : public SKELETON::ToolBar
    {
        friend class Core;

        Gtk::ToolItem m_tool_url;
        Gtk::Entry m_entry_url;
        SKELETON::ImgToolButton m_button_go;

        SKELETON::ImgToggleToolButton m_button_bbslist;
        SKELETON::ImgToggleToolButton m_button_favorite;
        SKELETON::ImgToggleToolButton m_button_hist;
        SKELETON::ImgToggleToolButton m_button_hist_board;
        SKELETON::ImgToggleToolButton m_button_hist_close;
        SKELETON::ImgToggleToolButton m_button_hist_closeboard;
        SKELETON::ImgToggleToolButton m_button_hist_closeimg;

        SKELETON::ImgToggleToolButton m_button_board;

        SKELETON::ImgToggleToolButton m_button_thread;

        SKELETON::ImgToggleToolButton m_button_image;

      public:

        MainToolBar(); 
        ~MainToolBar() noexcept {}

      protected:

        void pack_buttons() override;
    };
}


#endif
