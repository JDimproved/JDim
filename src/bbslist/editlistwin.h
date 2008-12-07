// ライセンス: GPL2

//
// お気に入り編集ウィンドウ
//

#ifndef _EDITLISTWIN_H
#define _EDITLISTWIN_H

#include <gtkmm.h>

namespace BBSLIST
{
    class SelectListView;

    class EditListWin : public Gtk::Window  
    {
        SelectListView* m_selectview;

        Gtk::VBox m_vbox;
        Gtk::Label m_label;

        Gtk::HBox m_hbox;
        Gtk::Button m_bt_close;

      public:

        EditListWin( const std::string& url, Glib::RefPtr< Gtk::TreeStore >& treestore );

        void clock_in();

      private:

        void slot_close();
    };
};

#endif
