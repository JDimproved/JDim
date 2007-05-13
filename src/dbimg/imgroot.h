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

        // url が画像のURLかどうか
        bool is_loadable( const std::string& url );
        bool is_loadable( const char* url, int n );

        // キャッシュ削除
        void delete_cache( const std::string& url );

        // 全キャッシュ削除
        void delete_all_files();

      private:
        void redraw_imgs();
        void reset_imgs();
    };
}

#endif
