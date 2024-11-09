// ライセンス: GPL2

//
// 画像ポップアップクラス
//

#ifndef _IMAGEVIEWPOPUP_H
#define _IMAGEVIEWPOPUP_H

#include "imageviewbase.h"

#include <memory>


namespace IMAGE
{
    class ImageViewPopup : public ImageViewBase
    {
        Glib::RefPtr<Gtk::CssProvider> m_provider;
        std::unique_ptr<Gtk::Label> m_label;
        size_t m_length_prev{};

        bool m_clicked{};

      public:

        explicit ImageViewPopup( const std::string& url );
        ~ImageViewPopup() noexcept override = default;

        void clock_in() override;

        // 親ウィンドウは無し
        Gtk::Window* get_parent_win() override { return nullptr; }

        void stop() override;
        void show_view() override;
        void close_view() override;
        bool operate_view( const int control ) override;

      protected:

        Gtk::Menu* get_popupmenu( const std::string& url ) override;

      private:

        void show_view_impl();

        // クリックした時の処理
        void clicked() override;

        void update_label();
        void set_label( const std::string& status );
        void remove_label();

        void adjust_client_size( Gtk::Widget& widget );
    };

}

#endif

