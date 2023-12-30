// ライセンス: GPL2
//
// 埋め込み画像クラス
// 

#ifndef _EMVEDDEDIMAGE_H
#define _EMVEDDEDIMAGE_H

#include <gtkmm.h>

#include "skeleton/dispatchable.h"

#include "jdlib/imgloader.h"

#include <thread>


namespace DBIMG
{
    class Img;
}

namespace ARTICLE
{
    class EmbeddedImage : public SKELETON::Dispatchable
    {
        std::string m_url;
        Glib::RefPtr< Gdk::Pixbuf > m_pixbuf;
        DBIMG::Img* m_img;
        std::thread m_thread;

        Glib::RefPtr< JDLIB::ImgLoader > m_imgloader;

      public:

        explicit EmbeddedImage( const std::string& url );
        ~EmbeddedImage() override;

        Glib::RefPtr< Gdk::Pixbuf > get_pixbuf(){ return m_pixbuf; }

        void show();
        void resize_thread();

      private:
        void stop();
        void wait();
        void callback_dispatch() override;
    };
}

#endif
