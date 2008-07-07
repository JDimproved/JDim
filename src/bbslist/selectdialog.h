// ライセンス: GPL2

//
// お気に入り追加の時の選択ダイアログ
//

#ifndef _SELECTDIALOG_H
#define _SELECTDIALOG_H

#include <gtkmm.h>

namespace BBSLIST
{
    class SelectListView;
    
    // ダイアログ
    class SelectListDialog : public Gtk::Dialog
    {
        SelectListView* m_selectview;

      public:

        SelectListDialog( const std::string& url, Glib::RefPtr< Gtk::TreeStore >& store );
        virtual ~SelectListDialog();

        Gtk::TreePath get_path();

        virtual int run();

      private:

        void slot_close_dialog();
    };
};


#endif
