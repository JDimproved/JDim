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

        explicit ImgButton( const int id, const std::string& label = {} );

        explicit ImgButton( const Gtk::StockID& stock_id,
                            const std::string& label = {},
                            const Gtk::BuiltinIconSize icon_size = Gtk::ICON_SIZE_MENU );

        ~ImgButton() noexcept;

      private:

        void set( const std::string& label );
    };
}

#endif
