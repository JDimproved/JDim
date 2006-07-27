// ライセンス: 最新のGPL

#ifndef _BOARD_PREFERENCES_H
#define _BOARD_PREFERENCES_H

#include "skeleton/prefdiag.h"
#include "skeleton/editview.h"
#include "skeleton/label_entry.h"

namespace BOARD
{
    class Preferences : public SKELETON::PrefDiag
    {
        Gtk::Notebook m_notebook;
        Gtk::VBox m_vbox;

        // クッキー & hana
        Gtk::Frame m_frame_cookie;
        Gtk::HBox m_hbox_cookie;
        SKELETON::EditView m_edit_cookies;
        Gtk::VBox m_vbox_cookie;
        Gtk::Button m_button_cookie;

        // 名無し書き込みチェック
        Gtk::CheckButton m_check_noname;

        // 情報
        Gtk::VBox m_vbox_info;
        Gtk::Label m_label_name;
        SKELETON::LabelEntry m_label_url;
        SKELETON::LabelEntry m_label_cache;

        SKELETON::LabelEntry m_label_noname;
        SKELETON::LabelEntry m_label_line;
        SKELETON::LabelEntry m_label_byte;

        // あぼーん
        SKELETON::EditView m_edit_word, m_edit_regex;

        // SETTING.TXT
        SKELETON::EditView m_edit_settingtxt;

      public:
        Preferences( const std::string& url );

      private:
        void slot_delete_cookie();
        virtual void slot_ok_clicked();
    };

}

#endif
