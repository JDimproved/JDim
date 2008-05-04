// ライセンス: GPL2

// 画像つきドグルツールボタン

#ifndef _IMGTOGGLETOOLBUTTON_H
#define _IMGTOGGLETOOLBUTTON_H

#include <gtkmm.h>
#include <string>

namespace SKELETON
{
    class ImgToggleToolButton : public Gtk::ToggleToolButton
    {
        Gtk::Image* m_img;

      public:

        ImgToggleToolButton( const int id );

        ImgToggleToolButton( const Gtk::StockID& stock_id,
                         const Gtk::BuiltinIconSize icon_size = Gtk::ICON_SIZE_MENU );

    };
}

#endif
