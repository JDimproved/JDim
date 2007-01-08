// ライセンス: GPL2

#ifndef _EDITVIEWDIALOG_H
#define _EDITVIEWDIALOG_H

#include <gtkmm.h>

#include "editview.h"

namespace SKELETON
{
    class EditViewDialog : public Gtk::Dialog
    {
        SKELETON::EditView m_edit;

      public:

        EditViewDialog( const std::string& str, const std::string& title,
                        bool editable ){
            
            m_edit.set_text( str );
            m_edit.set_editable( editable );

            add_button( Gtk::Stock::OK, Gtk::RESPONSE_OK );
            get_vbox()->pack_start( m_edit  );
            set_title( title );
            show_all_children();

        }

    };
}

#endif
