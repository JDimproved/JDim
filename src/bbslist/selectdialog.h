// ライセンス: GPL2

//
// お気に入り挿入ダイアログ
//

#ifndef _SELECTDIALOG_H
#define _SELECTDIALOG_H

#include "skeleton/prefdiag.h"

#include <vector>
#include <memory>


namespace BBSLIST
{
    class SelectListView;
    
    class SelectListDialog : public SKELETON::PrefDiag
    {
        Glib::RefPtr< Gtk::TreeStore >& m_treestore;
        std::vector< std::string > m_vec_path;

        Gtk::Grid m_grid;

        Gtk::Label m_label_name;
        Gtk::Entry m_entry_name;

        Gtk::Label m_label_dirs;
        Gtk::ComboBoxText m_combo_dirs;
        Gtk::ToggleButton m_bt_show_tree;

        std::unique_ptr<SelectListView> m_selectview;

      public:

        SelectListDialog( Gtk::Window* parent, const std::string& url, Glib::RefPtr< Gtk::TreeStore >& treestore );
        ~SelectListDialog() noexcept override;

        std::string get_name() const;
        std::string get_path() const;

      protected:

        void slot_ok_clicked() override;

      private:

        void slot_show_tree();
        void timeout() override;
    };
}


#endif
