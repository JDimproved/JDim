// ライセンス: GPL2

// 画像データベースのルートクラス

#ifndef _IMGROOT_H
#define _IMGROOT_H

#include <string>
#include <map>

namespace DBIMG
{
    class Img;

    class ImgRoot
    {
        std::map< std::string, Img* > m_map_img;
        
      public:
        ImgRoot();
        ~ImgRoot();

        // Imgクラス取得(無ければ作成)
        Img* get_img( const std::string& url );

        // 検索(無ければNULL)
        Img* search_img( const std::string& url );

        // 画像データの先頭のシグネチャを見て画像のタイプを取得
        // 画像ではない場合は T_NOIMG を返す
        const int get_image_type( const unsigned char *sign );

        // 拡張子から画像タイプを取得
        // 画像ではない場合は T_UNKNOWN を返す
        int get_type_ext( const std::string& url );
        int get_type_ext( const char* url, int n );

        // キャッシュ削除
        void delete_cache( const std::string& url );

        // 全キャッシュ削除
        void delete_all_files();

      private:

        bool is_jpg( const char* url, int n );
        bool is_png( const char* url, int n );
        bool is_gif( const char* url, int n );
        bool is_bmp( const char* url, int n );

        void redraw_imgs();
        void reset_imgs();
    };
}

#endif
