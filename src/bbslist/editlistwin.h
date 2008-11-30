// ライセンス: GPL2

//
// お気に入り編集ウィンドウ
//

#ifndef _EDITLISTWIN_H
#define _EDITLISTWIN_H

#include <gtkmm.h>

namespace BBSLIST
{
    // public SKELETON::PrefDiag を継承すると最大化ボタンが表示されないことと、
    // メインループに戻らなくなり dispatchmanagerが動かないため画像ポップアップが
    // 表示されなくなる
    class EditListWin : public Gtk::Window  
    {
        Gtk::VBox m_vbox;
        Gtk::Label m_label;

        Gtk::HBox m_hbox;
        Gtk::Button m_bt_close;

      public:

        EditListWin( const std::string& url, Glib::RefPtr< Gtk::TreeStore >& treestore );

      private:

        void slot_close();
    };
};

#endif
