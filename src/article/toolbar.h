// ライセンス: GPL2

// ツールバーのクラス
//
// ARTICLE::ArticleView* 以外では使わない
//

#ifndef _ARTICLE_TOOLBAR_H
#define _ARTICLE_TOOLBAR_H

#include <gtkmm.h>

#include "skeleton/toolbar.h"
#include "skeleton/compentry.h"

namespace ARTICLE
{
    class ArticleToolBar : public SKELETON::ToolBar
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

        // ラベル、ボタンバー
        Gtk::Entry m_label;
        Gtk::Button m_button_board;
        SKELETON::ImgButton m_button_favorite;
        SKELETON::ImgButton m_button_write;
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

      public:

        ArticleToolBar(); 
        virtual ~ArticleToolBar(){}

        // 検索バー表示
        void show_searchbar();

        // 検索バーを消す
        void hide_searchbar();

        void set_label( const std::string& label );
        const std::string get_label(){ return m_label.get_text(); }

        // スレが壊れている
        void set_broken();

        // DAT落ち
        void set_old();

      protected:

        virtual void pack_buttons();

      private:

        // vboxがrealizeした
        virtual void slot_vbox_realize();
    };


    ////////////////////////////////////////////


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
