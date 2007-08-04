// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "imgroot.h"
#include "img.h"
#include "delimgcachediag.h"

#include "jdlib/confloader.h"

#ifdef _DEBUG
#include "jdlib/misctime.h"
#endif

#include "skeleton/msgdiag.h"

#include "config/globalconf.h"

#include "cache.h"
#include "command.h"
#include "prefdiagfactory.h"

#include <sys/time.h>

#include <sstream>

using namespace DBIMG;

ImgRoot::ImgRoot()
{
}


ImgRoot::~ImgRoot()
{
    std::map< std::string, Img* >::iterator it;
    for( it = m_map_img.begin(); it != m_map_img.end(); ++it ) delete ( *it ).second;  
}


//
// Imgクラス取得
// データベースに無ければImgクラスを作る
//
Img* ImgRoot::get_img( const std::string& url )
{
    Img* img = search_img( url );

    // 無ければ作る
    if( img == NULL ){
        img = new Img( url );
        m_map_img.insert( make_pair( url, img ) );
    }

    return img;
}


//
//　検索
//
// DBになくてもImgクラスは作らない
//
Img* ImgRoot::search_img( const std::string& url )
{
    std::map< std::string, Img* >::iterator it = m_map_img.find( url );
    if( it != m_map_img.end() ) return ( *it ).second;

    return NULL;
}



bool ImgRoot::is_loadable( const std::string& url )
{
    return is_loadable( url.c_str(), url.length() );
}


bool ImgRoot::is_loadable( const char* url, int n )
{
    // 今のところ拡張子だけを見る

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

    // .png
    if( *( url + n -4 ) == '.' &&
        *( url + n -3 ) == 'p' &&
        *( url + n -2 ) == 'n' &&
        *( url + n -1 ) == 'g'  ) return true;

    if( *( url + n -4 ) == '.' &&
        *( url + n -3 ) == 'P' &&
        *( url + n -2 ) == 'N' &&
        *( url + n -1 ) == 'G'  ) return true;

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
    if( CACHE::file_exists( path ) == CACHE::EXIST_FILE ) unlink( path.c_str() );

    // info 削除
    path = CACHE::path_img_info( url );
    if( CACHE::file_exists( path ) == CACHE::EXIST_FILE ) unlink( path.c_str() );

    // 再描画
    if( img ) img->reset();
    redraw_imgs();
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

    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( NULL, CORE::PREFDIAG_DELIMG, "" );
    int ret = pref->run();
    delete pref;
    if( ret != Gtk::RESPONSE_OK ) return;

    // 画像キャッシュ削除ダイアログ表示
    DelImgCacheDiag* deldiag = new DelImgCacheDiag();
    deldiag->run();
    delete deldiag;

    reset_imgs();
    redraw_imgs();
}


//
// 画像の再描画
//
void ImgRoot::redraw_imgs()
{
    CORE::core_set_command( "close_nocached_image_views" );
    CORE::core_set_command( "redraw_article" );
    CORE::core_set_command( "redraw_message" );
}


//
// Img クラスの情報をリセット
//
void ImgRoot::reset_imgs()
{
    std::map< std::string, Img* >::iterator it;
    for( it = m_map_img.begin(); it != m_map_img.end(); ++it ){
        Img* img = ( *it ).second;
        if( img ) img->reset();
    }
}



