// ライセンス: GPL2

//
// 記事投稿確認ダイアログ
//

#ifndef _CONFIRMDIAG_H
#define _CONFIRMDIAG_H

#include "skeleton/detaildiag.h"

namespace MESSAGE
{
    class ConfirmDiag : public SKELETON::DetailDiag
    {
        Gtk::CheckButton m_chkbutton;

      public:

        ConfirmDiag( const std::string& url, const std::string& message );

        Gtk::CheckButton& get_chkbutton(){ return m_chkbutton; }

      private:

        void slot_switch_page( GtkNotebookPage*, guint page ) override;
    };
}

#endif
