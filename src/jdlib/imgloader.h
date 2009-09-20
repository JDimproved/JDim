// ライセンス: GPL2

//
// 画像ローダ
//

#ifndef _IMGLOADER_H
#define _IMGLOADER_H

#include <gtkmm.h>

namespace JDLIB
{
    class ImgLoader
    {
        Glib::RefPtr< Gdk::PixbufLoader > m_loader;

        std::string m_file;
        std::string m_errmsg;
        int m_width;
        int m_height;
        bool m_stop;

        bool m_pixbufonly;
        int m_y;

      public:

        ImgLoader( const std::string& file );

        virtual ~ImgLoader(){}

        Glib::RefPtr< Gdk::PixbufLoader > get_loader(){ return  m_loader; }
        const std::string& get_errmsg() const { return m_errmsg; }
        const int get_width() const { return m_width; }
        const int get_height() const{ return m_height; }

        bool get_size();
        const bool load( const bool& stop, const bool pixbufonly, const bool sizeonly );

      private:
        void slot_size_prepared( int w, int h );
        void slot_area_updated(int x, int y, int w, int h );
    };
}

#endif
