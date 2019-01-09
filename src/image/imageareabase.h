// ライセンス: GPL2

//
// 画像クラスのベースクラス
//

#ifndef _IMAGEAREABASE_H
#define _IMAGEAREABASE_H

#include <gtkmm.h>

#include "skeleton/dispatchable.h"

#include "jdlib/constptr.h"
#include "jdlib/jdthread.h"
#include "jdlib/imgloader.h"

namespace DBIMG
{
    class Img;
}


namespace IMAGE
{
    class ImageAreaBase : public Gtk::Image, public SKELETON::Dispatchable
    {
        std::string m_url;
        JDLIB::ConstPtr< DBIMG::Img > m_img;
        Gdk::InterpType m_interptype;

        std::string m_errmsg; // エラーメッセージ

        bool m_ready; // 画像がsetされた

        int m_width;
        int m_height;

        // スレッド用変数
        JDLIB::Thread m_thread;

    protected:
        Glib::RefPtr< JDLIB::ImgLoader > m_imgloader;

    public:

        // interptype :
        // 0 -> INTERP_NEAREST
        // 1 -> INTERP_BILINEAR
        // 3 -> INTERP_HYPER
        ImageAreaBase( const std::string& url, const int interptype );
        ~ImageAreaBase();

        void stop();
        void wait();

        const std::string& get_url() const{ return m_url;}
        const std::string& get_errmsg() const{ return m_errmsg;}        

        bool is_ready() const { return m_ready; }
        bool is_loading(){ return m_thread.is_running(); }

        int get_width() const { return m_width; }
        int get_height() const { return m_height; }

        virtual void show_image() = 0;

        void set_fit_in_win( bool fit );

        virtual void load_image_thread();

    protected:

        JDLIB::ConstPtr< DBIMG::Img >& get_img(){ return  m_img; }
        void set_errmsg( const std::string& errmsg ){ m_errmsg = errmsg; }
        void set_ready( bool ready ){ m_ready = ready; }

        void set_width( const int width );
        void set_height( const int height );

        void load_image();

        bool create_imgloader( const bool pixbufonly, std::string& errmsg );

    private:

        void callback_dispatch() override;
        virtual void set_image();
        void set_mosaic( Glib::RefPtr< Gdk::Pixbuf > pixbuf );
    };
}

#endif
