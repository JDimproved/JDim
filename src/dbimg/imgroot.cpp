// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "imgroot.h"
#include "img.h"
#include "delimgcachediag.h"
#include "imginterface.h"

#include "jdlib/confloader.h"

#ifdef _DEBUG
#include "jdlib/misctime.h"
#endif

#include "cache.h"
#include "command.h"
#include "prefdiagfactory.h"
#include "urlreplacemanager.h"

#include <sys/time.h>

#include <sstream>

using namespace DBIMG;

ImgRoot::ImgRoot()
{
    // GdkPixbufが直接サポートしていない画像フォーマットは利用できるか確認する
    for( const Gdk::PixbufFormat& fmt : Gdk::Pixbuf::get_formats() ) {
        const Glib::ustring name = fmt.get_name();
        if( name == "webp" ) {
            m_webp_support = true;
        }
        else if( name == "avif" ) {
            m_avif_support = true;
        }
    }
}


ImgRoot::~ImgRoot()
{
    for( auto& key_val : m_map_img ) {
        key_val.second->terminate_load();
        key_val.second.reset(); // call unique_ptr::reset()
    }
}


void ImgRoot::clock_in()
{
    if( m_list_wait.size() ){

        for( Img* img : m_list_wait ) img->clock_in();

        remove_clock_in();
    }
}


// 読み込み待ちのためクロックを回すImgクラスをセット/リセット
void ImgRoot::set_clock_in( Img* img )
{
    m_list_wait.push_back( img );
}

void ImgRoot::reset_clock_in( Img* img )
{
    // リストに登録しておいて remove_clock_in()で消す
    m_list_delwait.push_back( img );
}


void ImgRoot::remove_clock_in()
{
    if( m_list_delwait.size() ){

#ifdef _DEBUG
        std::cout << "ImgRoot::remove_clock_in\n";
#endif

        for( Img* img : m_list_delwait ) {
#ifdef _DEBUG
            std::cout << img->url() << std::endl;
#endif
            m_list_wait.remove( img );
        }

        m_list_delwait.clear();
    }
}


//
// Imgクラス取得
// データベースに無ければImgクラスを作る
//
Img* ImgRoot::get_img( const std::string& url )
{
    auto it = m_map_img.find( url );
    if( it != m_map_img.end() ) return it->second.get();

    // 無ければ作る
    auto uniq = std::make_unique<Img>( url );
    Img* img = uniq.get();
    m_map_img.emplace( url, std::move( uniq ) );
    return img;
}


//
// 画像データの先頭のシグネチャを見て画像のタイプを取得
// 画像ではない場合は T_NOIMG を返す
//
// また、拡張子が偽装されていると未対応の画像が読み込まれる場合がある
// 読み込みが未対応の場合は T_NOT_SUPPORT を返す
//
int ImgRoot::get_image_type( const unsigned char *sign ) const
{
    int type = T_NOIMG;

    // jpeg は FF D8
    if( sign[ 0 ] == 0xFF
        && sign[ 1 ] == 0xD8 ) type = T_JPG;

    // png は 0x89 0x50 0x4e 0x47 0xd 0xa 0x1a 0xa
    else if( sign[ 0 ] == 0x89
             && sign[ 1 ] == 0x50
             && sign[ 2 ] == 0x4e
             && sign[ 3 ] == 0x47
             && sign[ 4 ] == 0x0d
             && sign[ 5 ] == 0x0a
             && sign[ 6 ] == 0x1a
             && sign[ 7 ] == 0x0a ) type = T_PNG;

    // gif
    else if( sign[ 0 ] == 'G'
             && sign[ 1 ] == 'I'
             && sign[ 2 ] == 'F' ) type = T_GIF;

    // bitmap
    else if( sign[ 0 ] == 'B'
             && sign[ 1 ] == 'M' ) type = T_BMP;

    // webp
    else if( sign[ 0 ] == 'R'
             && sign[ 1 ] == 'I'
             && sign[ 2 ] == 'F'
             && sign[ 3 ] == 'F'

             && sign[ 8 ] == 'W'
             && sign[ 9 ] == 'E'
             && sign[ 10 ] == 'B'
             && sign[ 11 ] == 'P' ) type = m_webp_support ? T_WEBP : T_NOT_SUPPORT;

    // avif
    else if( sign[ 4 ] == 'f'
             && sign[ 5 ] == 't'
             && sign[ 6 ] == 'y'
             && sign[ 7 ] == 'p'
             && sign[ 8 ] == 'a'
             && sign[ 9 ] == 'v'
             && sign[ 10 ] == 'i'
             && sign[ 11 ] == 'f' ) type = m_avif_support ? T_AVIF : T_NOT_SUPPORT;

    return type;
}


//
// 拡張子からタイプを取得
// 画像ではない場合は T_UNKNOWN を返す
//
int ImgRoot::get_type_ext( const std::string& url ) const
{
    return get_type_ext( url.c_str(), url.length() );
}


int ImgRoot::get_type_ext( const char* url, int n ) const
{
    // Urlreplaceによる画像コントロールを取得する
    int imgctrl = CORE::get_urlreplace_manager()->get_imgctrl( url );

    // URLに拡張子があっても画像として扱わない
    if( imgctrl & CORE::IMGCTRL_FORCEBROWSER ) return T_UNKNOWN;

    if( is_jpg( url, n ) ) return T_JPG;
    if( is_png( url, n ) ) return T_PNG;
    if( is_gif( url, n ) ) return T_GIF;
    if( is_bmp( url, n ) ) return T_BMP;
    if( m_webp_support && is_webp( url, n ) ) return T_WEBP;
    if( m_avif_support && is_avif( url, n ) ) return T_AVIF;

    // URLに拡張子がない場合でも画像として扱うか
    if( imgctrl & CORE::IMGCTRL_FORCEIMAGE ) return T_FORCEIMAGE;

    return T_UNKNOWN;
}


// 今のところ拡張子だけを見る
bool ImgRoot::is_jpg( const char* url, int n )
{
    // .jpg
    if( *( url + n -4 ) == '.' &&
        *( url + n -3 ) == 'j' &&
        *( url + n -2 ) == 'p' &&
        *( url + n -1 ) == 'g'  ) return true;

    if( *( url + n -4 ) == '.' &&
        *( url + n -3 ) == 'J' &&
        *( url + n -2 ) == 'P' &&
        *( url + n -1 ) == 'G'  ) return true;

    // .jpeg
    if( *( url + n -5 ) == '.' &&
        *( url + n -4 ) == 'j' &&
        *( url + n -3 ) == 'p' &&
        *( url + n -2 ) == 'e' &&
        *( url + n -1 ) == 'g'  ) return true;

    if( *( url + n -5 ) == '.' &&
        *( url + n -4 ) == 'J' &&
        *( url + n -3 ) == 'P' &&
        *( url + n -2 ) == 'E' &&
        *( url + n -1 ) == 'G'  ) return true;

    return false;
}

bool ImgRoot::is_png( const char* url, int n )
{
    // .png
    if( *( url + n -4 ) == '.' &&
        *( url + n -3 ) == 'p' &&
        *( url + n -2 ) == 'n' &&
        *( url + n -1 ) == 'g'  ) return true;

    if( *( url + n -4 ) == '.' &&
        *( url + n -3 ) == 'P' &&
        *( url + n -2 ) == 'N' &&
        *( url + n -1 ) == 'G'  ) return true;

    return false;
}

bool ImgRoot::is_gif( const char* url, int n )
{
    // .gif
    if( *( url + n -4 ) == '.' &&
        *( url + n -3 ) == 'g' &&
        *( url + n -2 ) == 'i' &&
        *( url + n -1 ) == 'f'  ) return true;

    if( *( url + n -4 ) == '.' &&
        *( url + n -3 ) == 'G' &&
        *( url + n -2 ) == 'I' &&
        *( url + n -1 ) == 'F'  ) return true;

    return false;
}

bool ImgRoot::is_bmp( const char* url, int n )
{
    // .bmp
    if( *( url + n -4 ) == '.' &&
        *( url + n -3 ) == 'b' &&
        *( url + n -2 ) == 'm' &&
        *( url + n -1 ) == 'p'  ) return true;

    if( *( url + n -4 ) == '.' &&
        *( url + n -3 ) == 'B' &&
        *( url + n -2 ) == 'M' &&
        *( url + n -1 ) == 'P'  ) return true;

    return false;
}


bool ImgRoot::is_webp( const char* url, int n )
{
    // .webp
    if( *( url + n -5 ) == '.' &&
        *( url + n -4 ) == 'w' &&
        *( url + n -3 ) == 'e' &&
        *( url + n -2 ) == 'b' &&
        *( url + n -1 ) == 'p'  ) return true;

    if( *( url + n -5 ) == '.' &&
        *( url + n -4 ) == 'W' &&
        *( url + n -3 ) == 'E' &&
        *( url + n -2 ) == 'B' &&
        *( url + n -1 ) == 'P'  ) return true;

    return false;
}


bool ImgRoot::is_avif( const char* url, int n )
{
    // .avif
    if( *( url + n -5 ) == '.' &&
        *( url + n -4 ) == 'a' &&
        *( url + n -3 ) == 'v' &&
        *( url + n -2 ) == 'i' &&
        *( url + n -1 ) == 'f'  ) return true;

    if( *( url + n -5 ) == '.' &&
        *( url + n -4 ) == 'A' &&
        *( url + n -3 ) == 'V' &&
        *( url + n -2 ) == 'I' &&
        *( url + n -1 ) == 'F'  ) return true;

    return false;
}


//
// キャッシュ削除
//
void ImgRoot::delete_cache( const std::string& url )
{
#ifdef _DEBUG
    std::cout << "ImgRoot::delete_cache  url = " << url << std::endl;
#endif

    Img* img = get_img( url );
    if( img && img->is_protected() ) return;

    // キャッシュ削除
    std::string path = CACHE::path_img( url );
    if( CACHE::file_exists( path ) == CACHE::EXIST_FILE ) unlink( to_locale_cstr( path ) );

    // info 削除
    path = CACHE::path_img_info( url );
    if( CACHE::file_exists( path ) == CACHE::EXIST_FILE ) unlink( to_locale_cstr( path ) );

    // 再描画
    if( img ) img->reset(); // call Img::reset()
    CORE::core_set_command( "close_image", url );
    CORE::core_set_command( "redraw_article" );
    CORE::core_set_command( "redraw_message" );
}


//
// 全キャッシュ削除
//
// image/info フォルダにあるファイルを全て取得して
// 期限切れのファイルを削除
//
void ImgRoot::delete_all_files()
{
#ifdef _DEBUG
    std::cout << "ImgRoot::delete_all_files\n";
#endif

    auto pref = CORE::PrefDiagFactory( nullptr, CORE::PREFDIAG_DELIMG, "" );
    int ret = pref->run();
    if( ret != Gtk::RESPONSE_OK ) return;

    // 画像キャッシュ削除ダイアログ表示
    DelImgCacheDiag deldiag;
    deldiag.run();

    reset_imgs();

    CORE::core_set_command( "close_nocached_image_views" );
    CORE::core_set_command( "redraw_article" );
    CORE::core_set_command( "redraw_message" );
}


//
// Img クラスの情報をリセット
//
void ImgRoot::reset_imgs()
{
    for( auto& key_val : m_map_img ) {
        key_val.second->reset(); // call Img::reset()
    }
}
