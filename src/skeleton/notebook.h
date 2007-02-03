// ライセンス: GPL2
//
// Notebook クラス
//

#ifndef _NOTEBOOK_H
#define _NOTEBOOK_H

#include <gtkmm.h>

namespace SKELETON
{
    class JDNotebook : public Gtk::Notebook
    {
      public:

        JDNotebook();
        ~JDNotebook();

        // unpack = true の時取り除く
        int append_remove_page( bool unpack, Widget& child, const Glib::ustring& tab_label, bool use_mnemonic = false );
    };
}

#endif
