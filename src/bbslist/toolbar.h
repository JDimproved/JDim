// ライセンス: GPL2

// ツールバーのクラス

#ifndef _BBSLIST_TOOLBAR_H
#define _BBSLIST_TOOLBAR_H

#include <gtkmm.h>

#include "skeleton/toolbar.h"
#include "skeleton/compentry.h"

enum
{
    COMBO_BBSLIST = 0,
    COMBO_FAVORITE = 1
};


namespace BBSLIST
{
    class BBSListToolBar : public SKELETON::ToolBar
    {
        friend class BBSListViewBase;
        friend class BBSListViewMain;
        friend class FavoriteListView;
        friend class SelectListView;

        // ラベルバー
        Gtk::HBox m_hbox_label;
        Gtk::ComboBoxText m_combo;

        // ツールバー
        bool m_toolbar_shown;
        SKELETON::SearchEntry m_entry_search;
        SKELETON::ImgButton m_button_up_search;
        SKELETON::ImgButton m_button_down_search;

      public:

        BBSListToolBar();
        virtual ~BBSListToolBar(){}

        void set_combo( int page ){ m_combo.set_active( page ); }
        int  get_combo(){ return m_combo.get_active_row_number(); }

        void remove_label();

        virtual void pack_buttons();
        virtual void unpack_buttons();
    };
}


#endif
