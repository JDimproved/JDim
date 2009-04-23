// ライセンス: GPL2
//
// タブの切り替えメニュー
//

#include <gtkmm.h>

namespace SKELETON
{
    class DragableNoteBook;
    class Admin;

    class TabSwitchMenu : public Gtk::Menu
    {
      public:

        TabSwitchMenu( DragableNoteBook* notebook, Admin* admin );
        ~TabSwitchMenu();
    };

}
