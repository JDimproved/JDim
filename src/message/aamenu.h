// ライセンス: GPL2

//
// AA 選択ポップアップメニュークラス
//

#include <gtkmm.h>

#include "skeleton/popupwinbase.h"

namespace MESSAGE
{
    class AAMenu : public Gtk::Menu
    {
        Gtk::Window& m_parent;
        SKELETON::PopupWinBase m_popup;
        Gtk::TextView m_textview;

        std::string m_url;
        int m_activeitem;

      public:

        AAMenu( Gtk::Window& parent, const std::string& url );
        virtual ~AAMenu(){}

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
