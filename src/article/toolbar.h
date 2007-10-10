// ライセンス: GPL2

// ツールバーのクラス
//
// ARTICLE::ArticleView* 以外では使わない
//

#ifndef _ARTICLE_TOOLBAR_H
#define _ARTICLE_TOOLBAR_H

#include <gtkmm.h>

#include "skeleton/imgbutton.h"
#include "skeleton/compentry.h"

namespace ARTICLE
{
    class ArticleToolBar : public Gtk::VBox
    {
        int m_status;

        friend class ArticleViewBase;
        friend class ArticleViewMain;
        friend class ArticleViewRes;
        friend class ArticleViewName;
        friend class ArticleViewID;
        friend class ArticleViewBM;
        friend class ArticleViewRefer;
        friend class ArticleViewURL;
        friend class ArticleViewDrawout;

        Gtk::Tooltips m_tooltip;

        // ラベル、ボタンバー
        Gtk::ScrolledWindow m_scrwin;
        Gtk::HBox m_buttonbar;
        Gtk::Entry m_label;
        bool m_toolbar_shown;
        Gtk::Button m_button_board;
        SKELETON::ImgButton m_button_favorite;
        SKELETON::ImgButton m_button_write;
        SKELETON::ImgButton m_button_close;
        SKELETON::ImgButton m_button_delete;
        SKELETON::ImgButton m_button_reload;
        SKELETON::ImgButton m_button_stop;
        SKELETON::ImgButton m_button_open_search;

        // 検索バー
        Gtk::HBox m_searchbar;
        bool m_searchbar_shown;
        SKELETON::SearchEntry m_entry_search;
        SKELETON::ImgButton m_button_close_search;
        SKELETON::ImgButton m_button_up_search;
        SKELETON::ImgButton m_button_down_search;
        SKELETON::ImgButton m_button_drawout_and;
        SKELETON::ImgButton m_button_drawout_or;
        SKELETON::ImgButton m_button_clear_hl;

        ArticleToolBar( bool show_bar ); 
        virtual ~ArticleToolBar(){}

        // 検索バー表示
        void show_searchbar();

        // 検索バーを消す
        void hide_searchbar();

        // ツールバーを表示
        void show_toolbar();

        // ツールバーを隠す
        void hide_toolbar();

        void set_label( const std::string& label );

        const std::string get_label(){ return m_label.get_text(); }

        // vboxがrealizeしたらラベル(Gtk::Entry)の背景色を変える
        void slot_vbox_realize();

        // スレが壊れている
        void broken();

        // DAT落ち
        void old();

        // タブのロック
        void lock();

        // タブのアンロック
        void unlock();
    };


    class SearchToolBar : public Gtk::ScrolledWindow
    {
        friend class ArticleViewSearch;

        Gtk::HBox m_hbox;

        Gtk::Tooltips m_tooltip;

        SKELETON::SearchEntry m_entry_search;
        SKELETON::ImgButton m_button_close;
        SKELETON::ImgButton m_button_reload;
        SKELETON::ImgButton m_button_stop;

        SearchToolBar();
        virtual ~SearchToolBar(){}
    };
}


#endif
