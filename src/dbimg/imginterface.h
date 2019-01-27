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
    // DBIMG::get_type_*() で取得する画像タイプ
    enum{
        T_NOIMG = 0,

        T_JPG,
        T_PNG,
        T_GIF,
        T_BMP,

        T_LARGE,
        T_NOSIZE,
        T_OPENFAILED,
        T_WRITEFAILED,
        T_NOT_FOUND,
        T_NODATA,

        T_UNKNOWN,    // 画像ではない
        T_FORCEIMAGE, // 拡張子がなくても画像として扱う
    };

    class Img;

    void create_root();
    void delete_root();

    void clock_in();

    // 読み込み待ちのためクロックを回すImgクラスをセット/リセット
    void set_clock_in( Img* img );
    void reset_clock_in( Img* img );

    // 画像データの先頭のシグネチャを見て画像のタイプを取得
    // 画像ではない場合は T_NOIMG を返す
    int get_image_type( const unsigned char *sign );

    // 拡張子だけをみて画像の種類を判断
    // キャッシュに無くても判断可能
    // 画像ではない場合は T_UNKNOWN を返す
    int get_type_ext( const std::string& url );
    int get_type_ext( const char* url, int n );

    // 実際の画像ファイルの種類を判断
    // 画像がキャッシュに無いときは判断不能( T_UNKNOWN を返す )
    int get_type_real( const std::string& url );

    DBIMG::Img* get_img( const std::string& url );
    const std::string get_cache_path( const std::string& url );

    // ロード開始
    // refurl : 参照元のスレのアドレス
    // mosaic : モザイク表示するか
    void download_img( const std::string& url, const std::string& refurl, const bool mosaic );

    // 時間差ロード
    // first : 一番最初の画像か
    void download_img_wait( const std::string& url, const std::string& refurl, const bool mosaic, const int first );

    void stop_load( const std::string& url );
    bool save( const std::string& url, Gtk::Window* parent, const std::string& path_to );
    void delete_cache( const std::string& url );
    void delete_all_files();
    int get_width( const std::string& url );
    int get_height( const std::string& url );
    bool is_cached( const std::string& url );
    int get_type( const std::string& url );
    bool get_abone( const std::string& url );
    void set_abone( const std::string& url, bool abone );
    bool is_loading( const std::string& url );
    bool is_wait( const std::string& url );
    int get_code( const std::string& url );
    const std::string get_str_code( const std::string& url );
    bool get_mosaic( const std::string& url );
    void set_mosaic( const std::string& url, bool mosaic );
    void show_large_img( const std::string& url );
    bool is_zoom_to_fit( const std::string& url );
    void set_zoom_to_fit( const std::string& url, bool fit );
    int get_size( const std::string& url );
    void set_size( const std::string& url, int size );
    const std::string get_refurl( const std::string& url );

    size_t byte( const std::string& url );
    size_t get_filesize( const std::string& url );
    bool is_protected( const std::string& url );
    bool is_fake( const std::string& url );
    void set_protect( const std::string& url, bool protect );
}

#endif
