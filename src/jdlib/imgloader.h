// ライセンス: GPL2

//
// 画像ローダ
//

#ifndef _IMGLOADER_H
#define _IMGLOADER_H

#include <gtkmm.h>

#include <mutex>

namespace JDLIB
{
    // 画像ロードレベル、必要なデータ量順に定義
    enum
    {
        LOADLEVEL_NORMAL = 0,   // 画像データ全体を読み込む
        LOADLEVEL_PIXBUFONLY,   // pixbufを作るのに十分なデータを読み込む
        LOADLEVEL_SIZEONLY,     // サイズを計算するのに十分なデータを読み込む

        LOADLEVEL_INIT
    };
    
    class ImgLoader : public Glib::Object
    {
        Glib::RefPtr< Gdk::PixbufLoader > m_loader;
        std::mutex m_loader_lock;
        
        std::string m_file;
        std::string m_errmsg;
        int m_width;
        int m_height;
        
        bool m_stop;
        int m_y;
        int m_loadlevel;
        int m_loadedlevel;
        
    public:
        virtual ~ImgLoader();
        static Glib::RefPtr< ImgLoader > get_loader( const std::string& file );
        
        const std::string& get_errmsg() const { return m_errmsg; }
        bool get_size( int& width, int& height );
        
        void request_stop();
        
        bool load( const bool pixbufonly = false );
        Glib::RefPtr<Gdk::Pixbuf> get_pixbuf( const bool pixbufonly = false );
        Glib::RefPtr<Gdk::PixbufAnimation> get_animation();
        
        bool equals( const std::string& file ) const;
        
    private:
        ImgLoader( const std::string& file );
        bool load_imgfile( const int loadlevel );
        
        void slot_size_prepared( int w, int h );
        void slot_area_updated(int x, int y, int w, int h );
    };
    
    class ImgProvider
    {
        std::list< Glib::RefPtr< ImgLoader > > m_cache;
        
    public:
        std::mutex m_provider_lock; // ImgProvider操作時の必須ロック
        
    public:
        virtual ~ImgProvider() noexcept {}
        static ImgProvider& get_provider();
        
        Glib::RefPtr< ImgLoader > get_loader( const std::string& file );
        void set_loader( Glib::RefPtr< ImgLoader > loader );
        
    private:
        ImgProvider();
    };
}

#endif
