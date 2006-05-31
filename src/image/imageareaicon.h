// ライセンス: 最新のGPL

//
// アイコン画像クラス
//

#ifndef _IMAGEAREAICON_H
#define _IMAGEAREAICON_H

#include "imageareabase.h"

#include <pthread.h>

namespace IMAGE
{
    class ImageAreaIcon : public ImageAreaBase
    {
        Glib::Dispatcher m_disp;

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

      private:
        static void* launcher( void* );
        virtual void show_image_thread();
        void slot_set_image();
    };
}

#endif
