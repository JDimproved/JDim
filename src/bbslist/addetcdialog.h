// ライセンス: GPL2
//
// 外部板追加ダイアログ
//

#ifndef _ADDETCDIALOG_H
#define _ADDETCDIALOG_H

#include "skeleton/prefdiag.h"
#include "skeleton/label_entry.h"

namespace BBSLIST
{
    class AddEtcDialog : public SKELETON::PrefDiag
    {
        SKELETON::LabelEntry m_entry_name;
        SKELETON::LabelEntry m_entry_url;

        Gtk::Frame m_frame;
        Gtk::VBox m_vbox;
        SKELETON::LabelEntry m_entry_id;
        SKELETON::LabelEntry m_entry_pw;

      public:

        AddEtcDialog( const bool move, const std::string& url, const std::string& _name, const std::string& _id, const std::string& _passwd );

        const std::string get_name(){ return m_entry_name.get_text(); }
        const std::string get_url(){ return m_entry_url.get_text(); }
        const std::string get_id(){ return m_entry_id.get_text(); }
        const std::string get_passwd(){ return m_entry_pw.get_text(); }
    };   

}

#endif
