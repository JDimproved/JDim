// ライセンス: GPL2

//
// 画像データベースへのインターフェース関数
//

#ifndef _IMGINTERFACE_H
#define _IMGINTERFACE_H

#include <string>

namespace Gtk
{
    class Window;
}

namespace DBIMG
{
    class Img;

    void create_root();
    void delete_root();

    // ロード可能な画像ファイルかチェック
    bool is_loadable( const std::string& url );
    bool is_loadable( const char* url, int n );

    DBIMG::Img* get_img( const std::string& url );
    std::string get_cache_path( const std::string& url );
    void download_img( const std::string& url, const std::string& refurl );
    void stop_load( const std::string& url );
    bool save( const std::string& url, Gtk::Window* parent, const std::string& path_to );
    void delete_cache( const std::string& url );
    void delete_all_files();
    int get_width( const std::string& url );    
    int get_height( const std::string& url );    
    bool is_cached( const std::string& url );
    bool get_abone( const std::string& url ); 
    void set_abone( const std::string& url, bool abone ); 
    bool is_loading( const std::string& url );
    int get_code( const std::string& url );
    std::string get_str_code( const std::string& url );
    bool get_mosaic( const std::string& url );
    void set_mosaic( const std::string& url, bool mosaic );
    bool is_zoom_to_fit( const std::string& url );
    void set_zoom_to_fit( const std::string& url, bool fit );
    int get_size( const std::string& url );
    void set_size( const std::string& url, int size );
    std::string get_refurl( const std::string& url );

    size_t byte( const std::string& url );
    size_t get_filesize( const std::string& url );
    bool is_protected( const std::string& url );
    void set_protect( const std::string& url, bool protect );
}

#endif
