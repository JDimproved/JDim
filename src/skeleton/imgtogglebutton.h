// ライセンス: GPL2

// 画像つきドグルボタン

#ifndef _IMGTOGGLEBUTTON_H
#define _IMGTOGGLEBUTTON_H

#include <gtkmm.h>
#include <string>

namespace SKELETON
{
    class ImgToggleButton : public Gtk::ToggleButton
    {
        Gtk::Image* m_img;
        Gtk::Label m_label;
        Gtk::HBox m_hbox;

      public:

        ImgToggleButton( const int id, const std::string label = std::string() );

        ImgToggleButton( const Gtk::StockID& stock_id,
                         const std::string label = std::string(),
                         const Gtk::BuiltinIconSize icon_size = Gtk::ICON_SIZE_MENU );

      private:

        void set( const std::string& label );

    };
}

#endif
