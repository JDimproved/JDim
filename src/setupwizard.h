// ライセンス: GPL2

//
// セットアップウィザード
//

#ifndef _SETUPWIZARD_H
#define _SETUPWIZARD_H

#include "gtkmmversion.h"

#include <gtkmm.h>


namespace CORE
{

    class PageStart : public Gtk::Grid
    {
        Gtk::Image m_icon;
        Gtk::Label m_label;

        Gtk::Label m_label2;

      public:

        PageStart();
    };


////////////////////////////////////////////


    class PageNet : public Gtk::Grid
    {
        Gtk::Window* m_parent{};
        Gtk::Image m_icon;
        Gtk::Label m_label;

        Gtk::Button m_proxy;
        Gtk::Button m_browser;

        // フレームの追加
        Gtk::Frame m_frame;

        Gtk::Label m_label_browser;

      public:

        explicit PageNet( Gtk::Window* parent );

      private:

        void slot_setup_proxy();
        void slot_setup_browser();
    };


/////////////////////////////////////////////


    class PageFont : public Gtk::Grid
    {
        Gtk::Image m_icon;
        Gtk::Label m_label;

        Gtk::Grid m_table;

        Gtk::Label m_label_res;
        Gtk::Label m_label_mail;
        Gtk::Label m_label_popup;
        Gtk::Label m_label_tree;

        Gtk::FontButton m_font_res;
        Gtk::FontButton m_font_mail;
        Gtk::FontButton m_font_popup;
        Gtk::FontButton m_font_tree;

      public:

        PageFont();

      private:

        void slot_font_res();
        void slot_font_mail();
        void slot_font_popup();
        void slot_font_tree();
    };


/////////////////////////////////////////////


    /** @brief テーマを選択するページ
     *
     * @details ユーザーがテーマを選択した後、変更はセットアップ完了後に適用されます。
     */
    class PageTheme : public Gtk::Grid
    {
        Gtk::Image m_icon;
        Gtk::Label m_label;

        Gtk::RadioButtonGroup m_radiogroup;
        Gtk::RadioButton m_system_theme;
        Gtk::RadioButton m_dark_theme;

        Gtk::Label m_label_inst;

      public:

        PageTheme();

      private:

        void slot_system_theme();
        void slot_dark_theme();
    };


/////////////////////////////////////////////


    class PagePane : public Gtk::Grid
    {
        Gtk::Image m_icon;
        Gtk::Label m_label;

        Gtk::RadioButtonGroup m_radiogroup;
        Gtk::RadioButton m_2pane;
        Gtk::RadioButton m_3pane;
        Gtk::RadioButton m_v3pane;

        Gtk::Label m_label_inst;

      public:

        PagePane();

      private:

        void slot_2pane();
        void slot_3pane();
        void slot_v3pane();
    };


/////////////////////////////////////////////

    class PageEnd : public Gtk::Grid
    {
        Gtk::Image m_icon;
        Gtk::Label m_label;

        Gtk::Label m_label2;

      public:

        PageEnd();
    };



/////////////////////////////////////////////


    class SetupWizard : public Gtk::Dialog
    {
        sigc::connection m_sigc_switch_page;
        Gtk::Notebook m_notebook;

        PageStart  m_page_start;
        PageNet  m_page_network;
        PageFont m_page_font;
        PageTheme m_page_theme;
        PagePane m_page_pane;
        PageEnd  m_page_end;

        Gtk::Button* m_fin;
        Gtk::Button m_back;
        Gtk::Button m_next;

      public:
        SetupWizard();
        ~SetupWizard() override;

      private:
        void slot_switch_page( Gtk::Widget* notebookpage, guint page );

        void slot_back();
        void slot_next();
        void slot_fin();
    };   

}

#endif
