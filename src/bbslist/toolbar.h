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
    class BBSListtToolBar : public Gtk::VBox
    {
        friend class BBSListViewBase;
        friend class BBSListViewMain;
        friend class FavoriteListView;

        // ラベルバー
        Gtk::HBox m_hbox_label;
        Gtk::Entry m_label;
        SKELETON::ImgButton m_button_close;

        // 検索バー
        Gtk::HBox m_hbox_search;
        SKELETON::JDEntry m_entry_search;
        SKELETON::ImgButton m_button_up_search;
        SKELETON::ImgButton m_button_down_search;

        Gtk::Tooltips m_tooltip;

        void set_label( const std::string& label )
        {
            m_label.set_text( label );
            m_tooltip.set_tip( m_label, label );
        }

        // vboxがrealizeしたらラベル(Gtk::Entry)の背景色を変える
        void slot_vbox_realize()
        {
            Gdk::Color color_bg = get_style()->get_bg( Gtk::STATE_NORMAL );
            m_label.modify_base( Gtk::STATE_NORMAL, color_bg );

            color_bg = get_style()->get_bg( Gtk::STATE_ACTIVE );
            m_label.modify_base( Gtk::STATE_ACTIVE, color_bg );
        }


        BBSListtToolBar() :
        m_button_close( Gtk::Stock::CLOSE ),
        m_button_up_search( Gtk::Stock::GO_UP ),
        m_button_down_search( Gtk::Stock::GO_DOWN )
        {
            // ラベルバー
            signal_realize().connect( sigc::mem_fun(*this, &BBSListtToolBar::slot_vbox_realize ) );

            // スレ名ラベル
            // Gtk::Label を使うと勝手にリサイズするときがあるので
            // 面倒でも　Gtk::Entry を使う。背景色は on_realize() で指定する。
            m_label.set_editable( false );
            m_label.set_activates_default( false );
            m_label.set_has_frame( false );
            m_tooltip.set_tip( m_button_close, CONTROL::get_label_motion( CONTROL::Quit ) );
            m_hbox_label.pack_start( m_label, Gtk::PACK_EXPAND_WIDGET, 2 );
            m_hbox_label.pack_start( m_button_close, Gtk::PACK_SHRINK );

            // 検索バー
            m_tooltip.set_tip( m_button_up_search, CONTROL::get_label_motion( CONTROL::SearchPrev ) );
            m_tooltip.set_tip( m_button_down_search, CONTROL::get_label_motion( CONTROL::SearchNext ) );
            m_hbox_search.pack_start( m_entry_search );
            m_hbox_search.pack_end( m_button_up_search, Gtk::PACK_SHRINK );
            m_hbox_search.pack_end( m_button_down_search, Gtk::PACK_SHRINK );


            pack_start( m_hbox_label, Gtk::PACK_SHRINK );
            pack_start( m_hbox_search, Gtk::PACK_SHRINK );
        }

    };
}


#endif
