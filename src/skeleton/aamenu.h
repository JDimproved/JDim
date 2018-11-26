// ライセンス: GPL2

//
// AA 選択ポップアップメニュークラス
//

#ifndef _AAMENU_H
#define _AAMENU_H

#include <gtkmm.h>
#include <map>

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

        std::map< Gtk::MenuItem*, int > m_map_items;

        Gtk::MenuItem* m_activeitem;

      public:

        AAMenu( Gtk::Window& parent );
        ~AAMenu();

        // 選択されたらemitされる
        SIG_AAMENU_SELECTED sig_selected() { return m_sig_selected; }

      protected:

        void on_map() override;
        void on_hide() override;
        bool on_key_press_event (GdkEventKey* event) override;

      private:

        int get_size();

        void set_text( const std::string& text );
        void create_menuitem( Glib::RefPtr< Gtk::ActionGroup > actiongroup, Gtk::Menu* menu, const int id );
        void create_popupmenu();

        bool move_down();
        bool move_up();

        void slot_select_item( Gtk::MenuItem* item );
        void slot_configured_popup( int width, int height );
        void slot_aainput_menu_clicked( Gtk::MenuItem* item );
    };
}

#endif
