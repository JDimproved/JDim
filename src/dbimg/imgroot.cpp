// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "imgroot.h"
#include "img.h"
#include "delimgdiag.h"

#include "jdlib/confloader.h"

#ifdef _DEBUG
#include "jdlib/misctime.h"
#endif

#include "skeleton/msgdiag.h"

#include "cache.h"
#include "command.h"

#include <gtkmm.h>
#include <sys/time.h>

#include <sstream>

using namespace DBIMG;

ImgRoot::ImgRoot()
{
}


ImgRoot::~ImgRoot()
{
    std::list< Img* >::iterator it;
    for( it = m_list_img.begin(); it != m_list_img.end(); ++it ) delete( *it );  
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
        m_list_img.push_back( img );
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
    Img* img;

    // 線形リストなので遅い
    std::list< Img* >::iterator it;
    for( it = m_list_img.begin(); it != m_list_img.end(); ++it ){

        img = *( it );
        if( img->url() == url ) return img;
    }

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
void ImgRoot::delete_cache( const std::string& url, bool redraw )
{
#ifdef _DEBUG
    std::cout << "ImgRoot::delete_cache  url = " << url << std::endl;
#endif

    Img* img = search_img( url );
    if( img && ! img->is_protected() ){
        img->clear();
        img->clear_load_data();
    }

    // ビューを閉じる
    CORE::core_set_command( "close_image_view", url );

    // キャッシュ削除
    std::string path = CACHE::path_img( url );
    if( CACHE::file_exists( path ) == CACHE::EXIST_FILE ) unlink( path.c_str() );

    // info 削除
    path = CACHE::path_img_info( url );
    if( CACHE::file_exists( path ) == CACHE::EXIST_FILE ) unlink( path.c_str() );

    // 再描画
    if( redraw ){
        CORE::core_set_command( "redraw_article" );
        CORE::core_set_command( "redraw_message" );
    }
}




//
// 全キャッシュ削除
//
// image/info フォルダにあるファイルを全て取得してinfoファイルから
// 含まれているURLを取得して保護されていない画像は delete_cache() を呼んで消す
//
void ImgRoot::delete_all_files()
{
#ifdef _DEBUG
    std::cout << "ImgRoot::delete_all_files\n";
#endif

    DelImgDiag deldiag;
    if( deldiag.run() != Gtk::RESPONSE_OK ) return;

    std::vector< std::string > list_urls;
    int num_files = 0;
    int64_t size_files = 0;

    std::list< std::string >list_infofile;
    std::string path_info_root = CACHE::path_img_info_root();
    list_infofile = CACHE::get_filelist( path_info_root );

    std::list< std::string >::iterator it_info = list_infofile.begin();
    for(; it_info != list_infofile.end(); ++it_info ){

        std::string path_info = CACHE::path_img_info_root() + ( *it_info );

        JDLIB::ConfLoader cf( path_info, std::string() );

        std::string url = cf.get_option( "url", "" );
        bool protect = cf.get_option( "protect", 0 );

        if( url.empty() ) continue;
        if( protect ) continue;

        std::string path_file = CACHE::path_img( url );

        // 経過日数を計算
        struct timeval tv;
        struct timezone tz;
        gettimeofday( &tv, &tz );
        time_t mtime = CACHE::get_filemtime( path_file );

        // 404などでinfoファイルはあるが画像が無い場合は infoのmtimeを見る
        if( ! mtime ) mtime = CACHE::get_filemtime( path_info );

        time_t days = ( tv.tv_sec - mtime ) / ( 60 * 60 * 24 );
        if( days >= CONFIG::get_del_img_day() ){

#ifdef _DEBUG
            std::cout << "\ninfo  = " << path_info << std::endl;
            std::cout << "path = " << path_file << std::endl;
            std::cout << "url = " << url << std::endl;
            std::cout << "protect = " << protect << std::endl;
            std::cout << "mtime = " << MISC::timettostr( mtime ) << std::endl;
            std::cout << "days = " << days << std::endl;
#endif
            list_urls.push_back( url );
            ++num_files;
            size_files += CACHE::get_filesize( path_file );
        }
    }

    if( !num_files ){
        SKELETON::MsgDiag mdiag2( NULL, "削除するファイルはありません", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK );
        mdiag2.run();
        return;
    }

    std::stringstream ss;
    ss << "削除するファイルの数 : " << num_files << "\n"
       << "削除するファイルの合計サイズ : " << ( size_files / 1024 / 1024 ) << "M\n\n"
       << "本当に画像を削除しますか？";

    SKELETON::MsgDiag mdiag2( NULL, ss.str(), false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL );
    if( mdiag2.run() != Gtk::RESPONSE_OK ) return;

    std::vector< std::string >::iterator it_url = list_urls.begin();
    for(; it_url != list_urls.end(); ++it_url ){

#ifdef _DEBUG
        std::cout << "delete " << *it_url << std::endl;
#endif

        delete_cache( *it_url,
                      false // ビューの再描画はしない
            );
    }

    // ビュー再描画
    CORE::core_set_command( "redraw_article" );
    CORE::core_set_command( "redraw_message" );
}
