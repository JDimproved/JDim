// ライセンス: GPL2

// Gtk::AboutDialog( gtkmm >= 2.6 )の代わりのクラス

#ifndef _ABOUTDIAG_H
#define _ABOUTDIAG_H

#include <gtkmm.h>


namespace SKELETON
{
    class AboutDiag : public Gtk::Dialog
    {
        Gtk::Notebook m_notebook;

        // 情報タブ
        Gtk::Label m_label_tab_info;
        Gtk::Image m_image_logo;
        Gtk::Label m_label_version;
        Gtk::VBox m_vbox_info;
        Gtk::Label m_label_info;
        Gtk::Label m_label_comments;
        Gtk::HBox m_hbox_url;
        Gtk::Button m_button_website;
        Gtk::Label m_label_copyright;

        // ライセンスタブ
        Gtk::Label m_label_tab_license;
        Gtk::ScrolledWindow m_scrollwindow_license;
        Gtk::TextView m_textview_license;

        // 動作環境タブ
        Gtk::VBox m_vbox_environment;
        Gtk::HButtonBox m_hbuttonbox_environment;
		Gtk::Button m_button_copy_environment;
        Gtk::Label m_label_tab_environment;
        Gtk::ScrolledWindow m_scrollwindow_environment;
        Gtk::TreeView m_treeview_environment;
        void set_environment_list();

        Glib::ustring m_website_url;

        void init();
        void slot_button_website_clicked();
        void slot_close_clicked() {}
        void slot_copy_environment();

      public:

        AboutDiag( const Glib::ustring& title );

        ~AboutDiag(){}

        int run();

        void set_logo( const Glib::RefPtr< Gdk::Pixbuf >& logo );
        Glib::RefPtr< Gdk::Pixbuf > get_logo();

        void set_version( const Glib::ustring& version );
        Glib::ustring get_version();

        void set_comments( const Glib::ustring& comments );
        Glib::ustring get_comments();

        void set_website( const Glib::ustring& website );
        Glib::ustring get_website();

        void set_website_label( const Glib::ustring& website_label );
        Glib::ustring get_website_label();

        void set_copyright( const Glib::ustring& copyright );
        Glib::ustring get_copyright();

        void set_license( const Glib::ustring& license );
        Glib::ustring get_license();
    };
}

#endif
