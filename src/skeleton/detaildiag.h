// ライセンス: GPL2

// 詳細表示ダイアログ

#ifndef _DETAILDIAG_H
#define _DETAILDIAG_H

#include "gtkmmversion.h"

#include "prefdiag.h"

#if GTKMM_CHECK_VERSION(3,0,0)
using GtkNotebookPage = Gtk::Widget;
#endif

namespace SKELETON
{
    class View;

    class DetailDiag : public SKELETON::PrefDiag
    {
        Gtk::Notebook m_notebook;
        Gtk::Label m_message;
        SKELETON::View* m_detail;

      public:

        DetailDiag( Gtk::Window* parent, const std::string& url,
                    const bool add_cancel,
                    const std::string& message, const std::string& tab_message,
                    const std::string& detail_html, const std::string& tab_detail
            );
        ~DetailDiag();

      protected:

        Gtk::Notebook& get_notebook(){ return  m_notebook; }
        SKELETON::View* get_detail(){ return m_detail; }

      private:

        virtual void slot_switch_page( GtkNotebookPage*, guint page );
        void timeout() override;
    };
}

#endif
