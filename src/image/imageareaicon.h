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
    class ImageAreaIcon : public ImageAreaBase, public SKELETON::Dispatchable
    {
        bool m_thread_running;
        pthread_t m_thread;
        bool m_shown;

        Glib::RefPtr< Gdk::Pixbuf > m_pixbuf;
        Glib::RefPtr< Gdk::Pixbuf > m_pixbuf_loading;
        Glib::RefPtr< Gdk::Pixbuf > m_pixbuf_err;
        Glib::RefPtr< Gdk::Pixbuf > m_pixbuf_icon;

      public:
        ImageAreaIcon( const std::string& url );
        ~ImageAreaIcon();

        virtual void show_image();
        virtual void show_image_thread();

      private:

        void show_indicator( bool loading );
        virtual void callback_dispatch();
        void set_image();
    };
}

#endif
