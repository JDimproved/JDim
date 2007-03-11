// ライセンス: GPL2

//
// AA 選択ポップアップメニュークラス
//

#ifndef _AAMENU_H
#define _AAMENU_H

#include <gtkmm.h>

#include "popupwinbase.h"

namespace SKELETON
{
    typedef sigc::signal< void, const std::string& > SIG_AAMENU_SELECTED;

    class AAMenu : public Gtk::Menu
    {
        SIG_AAMENU_SELECTED m_sig_selected;

        Gtk::Window& m_parent;
        SKELETON::PopupWinBase m_popup;
        Gtk::TextView m_textview;

        int m_activeitem;

      public:

        AAMenu( Gtk::Window& parent );
        virtual ~AAMenu(){}

        SIG_AAMENU_SELECTED sig_selected() { return m_sig_selected; }

        int get_size();

      protected:

        virtual void on_map();
        virtual void on_hide();
        virtual bool on_key_press_event (GdkEventKey* event);

      private:

        void set_text( const std::string& text );
        void create_popupmenu();

        void slot_select_item( int num );
        void slot_configured_popup( int width );
        void slot_aainput_menu_clicked( int num );
    };
}

#endif
