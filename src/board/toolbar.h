// ライセンス: GPL2

// ツールバーのクラス

#ifndef _BOARD_TOOLBAR_H
#define _BOARD_TOOLBAR_H

#include <gtkmm.h>

#include "skeleton/imgbutton.h"
#include "skeleton/compentry.h"

namespace BOARD
{
    class BoardToolBar : public Gtk::VBox
    {

        friend class BoardView;

        Gtk::ScrolledWindow m_scrwin;
        Gtk::HBox m_buttonbar;
        bool m_toolbar_shown;
        SKELETON::SearchEntry m_entry_search;
        SKELETON::ImgButton m_button_close;
        SKELETON::ImgButton m_button_reload;
        SKELETON::ImgButton m_button_delete;
        SKELETON::ImgButton m_button_stop;
        SKELETON::ImgButton m_button_favorite;
        SKELETON::ImgButton m_button_up_search;
        SKELETON::ImgButton m_button_down_search;
        SKELETON::ImgButton m_button_new_article;

        Gtk::Tooltips m_tooltip;

        BoardToolBar( bool show_bar );

        // ツールバーを表示
        void show_toolbar();

        // ツールバーを隠す
        void hide_toolbar();

        // タブのロック
        void lock();

        // タブのアンロック
        void unlock();
    };
}


#endif
