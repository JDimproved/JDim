// ライセンス: GPL2

// ツールバーのクラス

#ifndef _BBSLIST_TOOLBAR_H
#define _BBSLIST_TOOLBAR_H

#include <gtkmm.h>

#include "skeleton/imgbutton.h"
#include "skeleton/entry.h"

#include "controlutil.h"
#include "controlid.h"

namespace BBSLIST
{
    class BBSListtToolBar : public Gtk::HBox
    {
        friend class BBSListViewBase;

        SKELETON::JDEntry m_entry_search;
        SKELETON::ImgButton m_button_up_search;
        SKELETON::ImgButton m_button_down_search;

        Gtk::Tooltips m_tooltip;

        BBSListtToolBar() :
        m_button_up_search( Gtk::Stock::GO_UP ),
        m_button_down_search( Gtk::Stock::GO_DOWN )
        {
            m_tooltip.set_tip( m_button_up_search, CONTROL::get_label_motion( CONTROL::SearchPrev ) );
            m_tooltip.set_tip( m_button_down_search, CONTROL::get_label_motion( CONTROL::SearchNext ) );
            pack_start( m_entry_search );
            pack_end( m_button_up_search, Gtk::PACK_SHRINK );
            pack_end( m_button_down_search, Gtk::PACK_SHRINK );
        }

    };
}


#endif
