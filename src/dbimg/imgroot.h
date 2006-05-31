// ライセンス: 最新のGPL

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

        Img* get_img( const std::string& url );
        Img* search_img( const std::string& url );

        bool is_loadable( const std::string& url );
        bool is_loadable( const char* url, int n );

        void delete_cache( const std::string& url, bool redraw );
        void delete_all_files();
    };
}

#endif
