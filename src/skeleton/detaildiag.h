// ライセンス: GPL2

// 詳細表示ダイアログ

#ifndef _DETAILDIAG_H
#define _DETAILDIAG_H

#include "prefdiag.h"

#include <memory>


namespace SKELETON
{
    class View;

    class DetailDiag : public SKELETON::PrefDiag
    {
        Gtk::Notebook m_notebook;
        Gtk::Label m_message;
        std::unique_ptr<SKELETON::View> m_detail;

      public:

        DetailDiag( Gtk::Window* parent, const std::string& url,
                    const bool add_cancel,
                    const std::string& message, const std::string& tab_message,
                    const std::string& detail_html, const std::string& tab_detail
            );
        ~DetailDiag() noexcept;

      protected:

        Gtk::Notebook& get_notebook(){ return  m_notebook; }
        SKELETON::View* get_detail(){ return m_detail.get(); }

      private:

        virtual void slot_switch_page( Gtk::Widget*, guint page );
        void timeout() override;
    };
}

#endif
