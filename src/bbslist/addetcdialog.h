// ライセンス: GPL2
//
// 外部板追加ダイアログ
//

#ifndef _ADDETCDIALOG_H
#define _ADDETCDIALOG_H

#include "skeleton/prefdiag.h"

namespace BBSLIST
{
    class AddEtcDialog : public SKELETON::PrefDiag
    {
        Gtk::Grid m_grid;
        Gtk::Label m_label_name;
        Gtk::Entry m_entry_name;
        Gtk::Label m_label_url;
        Gtk::Entry m_entry_url;

        Gtk::Frame m_frame;
        Gtk::Grid m_grid_auth;
        Gtk::Label m_label_id;
        Gtk::Entry m_entry_id;
        Gtk::Label m_label_pw;
        Gtk::Entry m_entry_pw;

      public:

        AddEtcDialog( const bool move, const std::string& url, const std::string& name,
                      const std::string& id, const std::string& passwd );
        ~AddEtcDialog() noexcept override;

        std::string get_name() const { return m_entry_name.get_text(); }
        std::string get_url() const { return m_entry_url.get_text(); }
        std::string get_id() const { return m_entry_id.get_text(); }
        std::string get_passwd() const { return m_entry_pw.get_text(); }
    };


    /**
     * @brief 外部BBSMENUを追加/編集するダイアログ
     */
    class AddEtcBBSMenuDialog : public SKELETON::PrefDiag
    {
        /// @brief タイトルと入力欄だけでは分かりにくいため補足説明をつける
        Gtk::Label m_label_supplement;

        Gtk::Label m_label_name;
        Gtk::Label m_label_url;
        Gtk::Entry m_entry_name;
        Gtk::Entry m_entry_url;

        Gtk::Grid m_grid;

      public:

        AddEtcBBSMenuDialog( Gtk::Window* parent, const bool edit, const Glib::ustring& url, const Glib::ustring& name );
        ~AddEtcBBSMenuDialog() noexcept override = default;

        std::string get_name() const { return m_entry_name.get_text(); }
        std::string get_url() const { return m_entry_url.get_text(); }
    };
}

#endif
