// ライセンス: GPL2

//
// お気に入り編集ウィンドウ
//

#ifndef _EDITLISTWIN_H
#define _EDITLISTWIN_H

#include <gtkmm.h>

namespace BBSLIST
{
    class EditListToolBar;
    class SelectListView;

    class EditListWin : public Gtk::Window  
    {
        SelectListView* m_selectview;

        Gtk::VBox m_vbox;
        Gtk::Label m_label;

        EditListToolBar* m_toolbar;

      public:

        EditListWin( const std::string& url, Glib::RefPtr< Gtk::TreeStore >& treestore );

        void clock_in();
        void append_item();

      private:

        // 閉じる
        void slot_close();

        // 検索関係
        void slot_focus_entry_search();
        void slot_changed_search();
        void slot_active_search();
        void slot_operate_search( const int controlid );
        void slot_up_search();
        void slot_down_search();
        void slot_undo();
        void slot_redo();
        void slot_undo_buffer_changed();
    };
}

#endif
