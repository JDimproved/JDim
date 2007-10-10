// ライセンス: GPL2

// ツールバーのクラス

#ifndef _BBSLIST_TOOLBAR_H
#define _BBSLIST_TOOLBAR_H

#include <gtkmm.h>

#include "skeleton/imgbutton.h"
#include "skeleton/compentry.h"



enum
{
    COMBO_BBSLIST = 0,
    COMBO_FAVORITE = 1
};



namespace BBSLIST
{
    class BBSListtToolBar : public Gtk::VBox
    {
        friend class BBSListViewBase;
        friend class BBSListViewMain;
        friend class FavoriteListView;
        friend class SelectListView;

        bool m_toolbar_shown;

        // ラベルバー
        Gtk::HBox m_hbox_label;
        Gtk::ComboBoxText m_combo;
        SKELETON::ImgButton m_button_close;

        // 検索バー
        Gtk::HBox m_hbox_search;
        SKELETON::SearchEntry m_entry_search;
        SKELETON::ImgButton m_button_up_search;
        SKELETON::ImgButton m_button_down_search;

        Gtk::Tooltips m_tooltip;

        BBSListtToolBar( bool show_bar );
        virtual ~BBSListtToolBar(){}

        void set_combo( int page ){ m_combo.set_active( page ); }
        int  get_combo(){ return m_combo.get_active_row_number(); }

        // ツールバーを表示
        void show_toolbar();

        // ツールバーを隠す
        void hide_toolbar();

        void remove_label();
    };
}


#endif
