// ライセンス: GPL2

// 画像つきボタン

#ifndef _IMGBUTTON_H
#define _IMGBUTTON_H

#include <gtkmm.h>
#include <string>

namespace SKELETON
{
    class ImgButton : public Gtk::Button
    {
        Gtk::Image* m_img;
        Gtk::Label m_label;
        Gtk::HBox m_hbox;

      public:

        ImgButton( const int id, const std::string label = std::string() );

        ImgButton( const Gtk::StockID& stock_id,
                   const std::string label = std::string(),
                   const Gtk::BuiltinIconSize icon_size = Gtk::ICON_SIZE_MENU );
      private:

        void set( const std::string& label );
    };
}

#endif
