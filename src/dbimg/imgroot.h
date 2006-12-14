// ライセンス: GPL2

// 画像データベースのルートクラス

#ifndef _IMGROOT_H
#define _IMGROOT_H

#include <string>
#include <list>

namespace DBIMG
{
    class Img;

    class ImgRoot
    {
        std::list< Img* > m_list_img;
        
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
        void delete_cache( const std::string& url, bool redraw );

        // 全キャッシュ削除
        void delete_all_files();
    };
}

#endif
