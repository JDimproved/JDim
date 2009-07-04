// ライセンス: GPL2
//
// タブの切り替えメニュー
//

#include <gtkmm.h>
#include <vector>

namespace SKELETON
{
    class DragableNoteBook;
    class Admin;

    class TabSwitchMenu : public Gtk::Menu
    {
        DragableNoteBook* m_parentnote;
        std::vector< Gtk::Image* > m_vec_images;
        bool m_deactivated;

      public:

        TabSwitchMenu( DragableNoteBook* notebook, Admin* admin );
        ~TabSwitchMenu();

        void update_icons();

      protected:

        virtual void on_deactivate();

    };
}
