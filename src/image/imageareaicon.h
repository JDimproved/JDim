// ライセンス: GPL2

//
// アイコン画像クラス
//

#ifndef _IMAGEAREAICON_H
#define _IMAGEAREAICON_H

#include "imageareabase.h"

namespace IMAGE
{
    // m_imagetype にセットする値
    enum
    {
        IMAGE_SHOW_ICON = 0,
        IMAGE_SHOW_INDICATOR
    };


    class ImageAreaIcon : public ImageAreaBase
    {
        bool m_shown;
        int m_imagetype; // dispatch()前に表示する画像を入れる

        Glib::RefPtr< Gdk::Pixbuf > m_pixbuf;
        Glib::RefPtr< Gdk::Pixbuf > m_pixbuf_loading;
        Glib::RefPtr< Gdk::Pixbuf > m_pixbuf_err;
        Glib::RefPtr< Gdk::Pixbuf > m_pixbuf_icon;

      public:

        ImageAreaIcon( const std::string& url );
        ~ImageAreaIcon();

        void show_image() override;

        void load_image_thread() override;

      private:

        int width_indicator();
        int height_indicator();

        void show_indicator( bool loading );

        void set_image() override;
    };
}

#endif
