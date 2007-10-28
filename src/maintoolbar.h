// ライセンス: GPL2

// メインツールバークラス
//
// CORE::Core 以外では使わない
//

#ifndef _MAIN_TOOLBAR_H
#define _MAIN_TOOLBAR_H

#include <gtkmm.h>

#include "skeleton/toolbar.h"

namespace CORE
{
    class MainToolBar : public SKELETON::ToolBar
    {
        friend class Core;

        Gtk::Entry m_entry_url;
        SKELETON::ImgButton m_button_go;

        SKELETON::ImgToggleButton m_button_bbslist;
        SKELETON::ImgToggleButton m_button_favorite;
        SKELETON::ImgToggleButton m_button_board;
        SKELETON::ImgToggleButton m_button_thread;
        SKELETON::ImgToggleButton m_button_image;

        Gtk::VSeparator m_vspr;

      public:

        MainToolBar(); 
        virtual ~MainToolBar(){}

      protected:

        virtual void pack_buttons();
    };
}


#endif
