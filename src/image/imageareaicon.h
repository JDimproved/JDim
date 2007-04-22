// ライセンス: GPL2

//
// アイコン画像クラス
//

#ifndef _IMAGEAREAICON_H
#define _IMAGEAREAICON_H

#include "skeleton/dispatchable.h"

#include "imageareabase.h"

#include <pthread.h>

namespace IMAGE
{
    // m_imagetype にセットする値
    enum
    {
        IMAGE_SHOW_ICON = 0,
        IMAGE_SHOW_INDICATOR
    };


    class ImageAreaIcon : public ImageAreaBase, public SKELETON::Dispatchable
    {
        pthread_t m_thread;
        bool m_stop;

        bool m_shown;
        int m_imagetype; // dispatch()前に表示する画像を入れる

        Glib::RefPtr< Gdk::Pixbuf > m_pixbuf;
        Glib::RefPtr< Gdk::Pixbuf > m_pixbuf_loading;
        Glib::RefPtr< Gdk::Pixbuf > m_pixbuf_err;
        Glib::RefPtr< Gdk::Pixbuf > m_pixbuf_icon;

      public:

        ImageAreaIcon( const std::string& url );
        virtual ~ImageAreaIcon();

        virtual void show_image();
        void show_image_thread();

      private:

        void stop();
        void wait();
        void show_indicator( bool loading );
        virtual void callback_dispatch();
        void set_image();
    };
}

#endif
