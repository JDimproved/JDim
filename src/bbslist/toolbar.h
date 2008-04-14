// ライセンス: GPL2

// ツールバーのクラス

#ifndef _BBSLIST_TOOLBAR_H
#define _BBSLIST_TOOLBAR_H

#include <gtkmm.h>

#include "skeleton/toolbar.h"
#include "skeleton/imgmenubutton.h"

namespace BBSLIST
{
    class BBSListToolBar : public SKELETON::ToolBar
    {
        // ラベルバー
        Gtk::HBox m_hbox_label;
        SKELETON::ImgMenuButton m_button_toggle;

      public:

        BBSListToolBar();
        virtual ~BBSListToolBar(){}

      protected:

        virtual void pack_buttons();

      private:

        void slot_toggle( int i );
    };
}


#endif
