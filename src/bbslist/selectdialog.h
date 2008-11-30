// ライセンス: GPL2

//
// お気に入り挿入ダイアログ
//

#ifndef _SELECTDIALOG_H
#define _SELECTDIALOG_H

#include "skeleton/prefdiag.h"
#include "skeleton/label_entry.h"

#include <vector>

namespace BBSLIST
{
    class SelectListView;
    
    class SelectListDialog : public SKELETON::PrefDiag
    {
        Glib::RefPtr< Gtk::TreeStore >& m_treestore;
        std::vector< Gtk::TreePath > m_vec_path;

        SKELETON::LabelEntry m_label_name;

        Gtk::HBox m_hbox_dirs;
        Gtk::Label m_label_dirs;
        Gtk::ComboBoxText m_combo_dirs;

        Gtk::ToggleButton m_bt_show_tree;

        SelectListView* m_selectview;

      public:

        SelectListDialog( const std::string& url, Glib::RefPtr< Gtk::TreeStore >& treestore );
        virtual ~SelectListDialog();

        Gtk::TreePath get_path();

      protected:

        virtual void slot_ok_clicked();

      private:

        void slot_show_tree();
    };
};


#endif
