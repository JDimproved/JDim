// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "imgloader.h"

using namespace JDLIB;


ImgLoader::ImgLoader( const std::string& file )
    : m_file( file ),
      m_width( 0 ),
      m_height( 0 ),
      m_stop( false ),
      m_y( 0 )
{}
    

// 画像サイズ取得
bool ImgLoader::get_size()
{
    bool stop = false;
    return load( stop, true, true );
}


// 画像読み込み
// stop を trueにすると読み込みを停止する
// 動画でpixbufonly = true の時はアニメーションさせない
// sizeonly = true の時はサイズの取得のみ
const bool ImgLoader::load( const bool& stop, const bool pixbufonly, const bool sizeonly )
{
    if( sizeonly && m_width && m_height ) return true;
    if( m_loader ) return true;

    m_pixbufonly = pixbufonly;

#ifdef _DEBUG
    std::cout << "ImgLoader sizeonly = " << sizeonly
              << " file = " << m_file << std::endl;
    size_t total = 0;
#endif

    bool ret = true;

    FILE* f = NULL;
    const size_t bufsize = 8192;
    size_t readsize = 0;
    guint8 data[ bufsize ];

    f = fopen( m_file.c_str(), "rb" );
    if( ! f ){
        m_errmsg = "cannot file open";
        return false;
    }

    try {
        m_loader = Gdk::PixbufLoader::create();

#if GTKMMVER > 240
        if( sizeonly ) m_loader->signal_size_prepared().connect( sigc::mem_fun( *this, &ImgLoader::slot_size_prepared ) );
#endif

        m_loader->signal_area_updated().connect( sigc::mem_fun( *this, &ImgLoader::slot_area_updated ) );

        while( ! m_stop && ! stop  ){

            readsize = fread( data, 1, bufsize, f );
            if( readsize ) m_loader->write( data, readsize );

#ifdef _DEBUG
            total += readsize;
            std::cout << readsize << " / " << total << std::endl;
#endif
            if( feof( f ) ) break;

#if GTKMMVER <= 240 // gdkのバージョンが古い場合はpixbufを取得してサイズを得る

            if( sizeonly && m_loader->get_pixbuf() ){
                m_width = m_loader->get_pixbuf()->get_width();
                m_height = m_loader->get_pixbuf()->get_height();
                m_stop = true;
            }
#endif
        }

        m_loader->close();
    }
    catch( Glib::Error& err )
    {
#ifdef _DEBUG
        std::string stop_s = ( m_stop || stop )  ? "true" : "false";
        std::cout << "ImgLoader (" << stop_s << ") : " << m_file << std::endl;
#endif
        if( ! m_stop && ! stop ){
            m_errmsg = err.what();
            m_loader.clear();
            ret = false;
        }
    }

    fclose( f );

#ifdef _DEBUG
    std::cout << "ImgLoader::load read = " << total << "  w = " << m_width << " h = " << m_height << std::endl;
#endif

    return ret;
}


// PixbufLoaderが生成する画像の大きさを計算するのに必要な量のデータを受け取った時のシグナルハンドラ
void ImgLoader::slot_size_prepared( int w, int h )
{
#ifdef _DEBUG
    std::cout << "ImgLoader::slot_size_prepared w = " << w << " h = " << h << std::endl;
#endif

    m_width = w;
    m_height = h;
#ifndef _WIN32
    m_stop = true;
#endif
}


// 画像の一部分が更新された時のシグナルハンドラ
void ImgLoader::slot_area_updated(int x, int y, int w, int h )
{
    if( m_pixbufonly ){

#ifdef _DEBUG
        std::cout << "ImgLoader::slot_area_updated x = " << x << " y = " << y << " w = " << w << " h = " << h << std::endl;
#endif

#ifndef _WIN32
        // アニメーション画像を表示する際、幅や高さが元の値と異なる時に、全ての画像データを
        // 読み込まなくても pixbuf だけ取り出せれば良いので、pixbufを取り出せるようになった時点で
        // 画像データの読み込みを途中で止めて表示待ち時間を短縮する
        if( y < m_y ) m_stop = true;

        // ただしwindowsでは読み込みを中断すると、通常の画像でpixbufが壊れてしまうことがある
#endif
        m_y = y;
    }
}
