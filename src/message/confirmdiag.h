// ライセンス: GPL2

//
// 記事投稿確認ダイアログ
//

#ifndef _CONFIRMDIAG_H
#define _CONFIRMDIAG_H

#include "skeleton/prefdiag.h"

namespace SKELETON
{
    class View;
}

namespace MESSAGE
{
    class ConfirmDiag : public SKELETON::PrefDiag
    {
        Gtk::Notebook m_notebook;
        Gtk::Label m_message;
        SKELETON::View* m_localrule;
        Gtk::CheckButton m_chkbutton;

      public:

        ConfirmDiag( const std::string& url, const std::string& message );
        virtual ~ConfirmDiag();

        Gtk::CheckButton& get_chkbutton(){ return m_chkbutton; }

      private:

        void slot_switch_page( GtkNotebookPage*, guint page );
        virtual void timeout();
    };
}

#endif
