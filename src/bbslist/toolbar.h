// ライセンス: GPL2

// ツールバーのクラス

#ifndef _BBSLIST_TOOLBAR_H
#define _BBSLIST_TOOLBAR_H

#include <gtkmm.h>

#include "skeleton/toolbar.h"

namespace BBSLIST
{
    class BBSListToolBar : public SKELETON::ToolBar
    {
        // ラベルバー
        Gtk::HBox m_hbox_label;

        Gtk::Button m_button_toggle;

      public:

        BBSListToolBar();
        virtual ~BBSListToolBar(){}

      protected:

        virtual void pack_buttons();
    };
}


#endif
