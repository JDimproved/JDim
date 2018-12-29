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
        Admin* m_parentadmin;
        DragableNoteBook* m_parentnote;
        bool m_deactivated;
        int m_size;

        std::vector< Gtk::MenuItem* > m_vec_items;
        std::vector< Gtk::Label* > m_vec_labels;
        std::vector< Gtk::Image* > m_vec_images;


      public:

        TabSwitchMenu( DragableNoteBook* notebook, Admin* admin );
        ~TabSwitchMenu();

        void remove_items();
        void append_items();

        void update_labels();
        void update_icons();

        void deactivate();

      protected:

        void on_deactivate() override;

    };
}
