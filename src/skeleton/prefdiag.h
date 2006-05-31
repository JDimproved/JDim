// ライセンス: 最新のGPL

// 設定ダイアログの基底クラス

#ifndef _PREFDIAG_H
#define _PREFDIAG_H

#include <gtkmm.h>

namespace SKELETON
{
    class PrefDiag : public Gtk::Dialog
    {
        std::string m_url;
        virtual void slot_ok_clicked(){}

      public:

        PrefDiag( const std::string& url, bool add_cancel = true ) : m_url( url )
        {
            if( add_cancel ) add_button( Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL );

            add_button( Gtk::Stock::OK, Gtk::RESPONSE_OK )
            ->signal_clicked().connect( sigc::mem_fun(*this, &PrefDiag::slot_ok_clicked ) );
        }

        virtual ~PrefDiag(){}

        const std::string& get_url() const { return m_url; }
    };
}

#endif
