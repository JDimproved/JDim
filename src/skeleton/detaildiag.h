// ライセンス: GPL2

// 詳細表示ダイアログ

#ifndef _DETAILDIAG_H
#define _DETAILDIAG_H

#include "prefdiag.h"

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
        virtual ~DetailDiag();

      protected:

        Gtk::Notebook& get_notebook(){ return  m_notebook; }
        SKELETON::View* get_detail(){ return m_detail; }

      private:

        virtual void slot_switch_page( GtkNotebookPage*, guint page );
        virtual void timeout();
    };
}

#endif
