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
        SKELETON::ImgToggleToolButton m_button_board;
        SKELETON::ImgToggleToolButton m_button_thread;
        SKELETON::ImgToggleToolButton m_button_image;

      public:

        MainToolBar(); 
        virtual ~MainToolBar(){}

      protected:

        virtual void pack_buttons();
    };
}


#endif
