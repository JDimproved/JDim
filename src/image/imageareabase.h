// ライセンス: 最新のGPL

//
// 画像クラスのベースクラス
//

#ifndef _IMAGEAREABASE_H
#define _IMAGEAREABASE_H

#include <gtkmm.h>

#include "jdlib/constptr.h"


namespace DBIMG
{
    class Img;
}


namespace IMAGE
{
    class ImageAreaBase : public Gtk::Image
    {
        std::string m_url;
        JDLIB::ConstPtr< DBIMG::Img > m_img;

        std::string m_errmsg; // エラーメッセージ

        bool m_ready;      // show_image()が呼ばれたらtrueにセット

        int m_width_org;
        int m_height_org;
        int m_width;
        int m_height;

       

      protected:

        JDLIB::ConstPtr< DBIMG::Img >& get_img(){ return  m_img; }
        void set_errmsg( const std::string& errmsg ){ m_errmsg = errmsg; }
        void set_ready( bool ready ){ m_ready = ready; }

        void set_width_org( int width_org ){ m_width_org = width_org; }
        void set_height_org( int height_org ){ m_height_org = height_org; }
        void set_width( int width ){ m_width = width; }
        void set_height( int height ){ m_height = height; }

      public:

        ImageAreaBase( const std::string& url );
        ~ImageAreaBase();

        const std::string& get_url() const{ return m_url;}
        const std::string& get_errmsg() const{ return m_errmsg;}        
        const bool is_ready() const { return m_ready; }
        const int get_width_org() const { return m_width_org; }
        const int get_height_org() const { return m_height_org; }
        const int get_width() const { return m_width; }
        const int get_height() const { return m_height; }

        virtual void show_image(){}
        void set_fit_in_win( bool fit );
        const bool is_cached();

      protected:

        void set_image(  Glib::RefPtr< Gdk::Pixbuf >& pixbuf, bool mosaic, bool do_scale, double scale );
    };
}

#endif
