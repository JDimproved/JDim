// ライセンス: GPL2

// ツールバーのクラス

#ifndef _BBSLIST_TOOLBAR_H
#define _BBSLIST_TOOLBAR_H

#include <gtkmm.h>

#include "skeleton/toolbar.h"

enum
{
    COMBO_BBSLIST = 0,
    COMBO_FAVORITE = 1
};


namespace BBSLIST
{
    class BBSListToolBar : public SKELETON::ToolBar
    {
        bool m_enable_slot;

        // ラベルバー
        Gtk::HBox m_hbox_label;
        Gtk::ComboBoxText m_combo;

      public:

        BBSListToolBar();
        virtual ~BBSListToolBar(){}

        void set_combo( int page );

      protected:

        virtual void pack_buttons();

      private:

        void slot_combo_changed();
    };
}


#endif
