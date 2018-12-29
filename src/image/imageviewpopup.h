// ライセンス: GPL2

//
// 画像ポップアップクラス
//

#ifndef _IMAGEVIEWPOPUP_H
#define _IMAGEVIEWPOPUP_H

#include "imageviewbase.h"

namespace IMAGE
{
    class ImageViewPopup : public ImageViewBase
    {
        Gtk::EventBox m_event_frame;
        Gtk::EventBox m_event_margin;
        Gtk::Label* m_label;
        size_t m_length_prev;

        bool m_clicked;

      public:

        ImageViewPopup( const std::string& url );
        ~ImageViewPopup();

        void clock_in() override;

        // 親ウィンドウは無し
        Gtk::Window* get_parent_win() override { return nullptr; }

        void stop() override;
        void show_view() override;
        void close_view() override;
        const bool operate_view( const int control ) override;

      protected:

        Gtk::Menu* get_popupmenu( const std::string& url ) override;

      private:

        void show_view_impl();

        // クリックした時の処理
        void clicked() override;

        void update_label();
        void set_label( const std::string& status );
        void remove_label();
    };

}

#endif

