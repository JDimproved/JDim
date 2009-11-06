// ライセンス: GPL2

//
// 画像ローダ
//

#ifndef _IMGLOADER_H
#define _IMGLOADER_H

#include <gtkmm.h>

namespace JDLIB
{
    class ImgLoader : public Glib::Object
    {
        Glib::RefPtr< Gdk::PixbufLoader > m_loader;
        Glib::Mutex m_loader_lock;
        
        std::string m_file;
        std::string m_errmsg;
        int m_width;
        int m_height;
        
        bool m_stop;
        bool m_stopped; // 実際に読み込みを中断したフラグ
        
        bool m_pixbufonly;
        int m_y;
        
    public:
        virtual ~ImgLoader(){}
        static Glib::RefPtr< ImgLoader > get_loader( const std::string& file );
        
        const std::string& get_errmsg() const { return m_errmsg; }
        bool get_size( int& width, int& height );
        
        void request_stop();
        
        const bool load( const bool pixbufonly = false );
        Glib::RefPtr<Gdk::Pixbuf> get_pixbuf( const bool pixbufonly = false );
        Glib::RefPtr<Gdk::PixbufAnimation> get_animation();
        
        bool equals( const std::string& file ) const;
        
    private:
        ImgLoader( const std::string& file );
        const bool load( const bool pixbufonly, const bool sizeonly );
        
        void slot_size_prepared( int w, int h );
        void slot_area_updated(int x, int y, int w, int h );
    };
    
    class ImgProvider
    {
        std::list< Glib::RefPtr< ImgLoader > > m_cache;
        Glib::Mutex m_cache_lock;
        
    public:
        virtual ~ImgProvider(){}
        static ImgProvider& get_provider();
        
        Glib::RefPtr< ImgLoader > get_loader( const std::string& file );
        void set_loader( Glib::RefPtr< ImgLoader > loader );
        
    private:
        ImgProvider();
    };
}

#endif
