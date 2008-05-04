// ライセンス: GPL2

// 画像つきツールボタン

#ifndef _IMGTOOLBUTTON_H
#define _IMGTOOLBUTTON_H

#include <gtkmm.h>
#include <string>

namespace SKELETON
{
    class ImgToolButton : public Gtk::ToolButton
    {
        Gtk::Image* m_img;

      public:

        ImgToolButton( const int id );

        ImgToolButton( const Gtk::StockID& stock_id,
                   const Gtk::BuiltinIconSize icon_size = Gtk::ICON_SIZE_MENU );
    };
}

#endif
