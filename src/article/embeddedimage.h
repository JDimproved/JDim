// ライセンス: GPL2
//
// 埋め込み画像クラス
// 

#ifndef _EMVEDDEDIMAGE_H
#define _EMVEDDEDIMAGE_H

#include <gtkmm.h>

#include "skeleton/dispatchable.h"

#include "jdlib/constptr.h"
#include "jdlib/jdthread.h"

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
        JDLIB::ConstPtr< DBIMG::Img > m_img;
        JDLIB::Thread m_thread;

        bool m_stop;
        int m_width;
        int m_height;

      public:

        EmbeddedImage( const std::string& url );
        ~EmbeddedImage();

        Glib::RefPtr< Gdk::Pixbuf > get_pixbuf(){ return m_pixbuf; }

        const int get_width() const { return m_width; }
        const int get_height() const { return m_height; }

        void show();
        void resize_thread();

      private:
        void stop();
        void wait();
        virtual void callback_dispatch();
    };
}

#endif
