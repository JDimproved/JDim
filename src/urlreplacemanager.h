// ライセンス: GPL2

#ifndef _URLREPLACEMANAGER_H
#define _URLREPLACEMANAGER_H

#include <string>
#include <list>

#include "jdlib/jdregex.h"

namespace CORE
{

    struct UrlreplaceItem
    {
        std::string match;
        std::string replace;
        std::string referer;
        bool force_image;
    };

    class Urlreplace_Manager
    {
        std::list< UrlreplaceItem > m_list_cmd;

    public:

        Urlreplace_Manager();
        virtual ~Urlreplace_Manager(){}

        // URLを任意の正規表現で変換する
        const std::string exec( const std::string &url, bool &force_image );

        // URLからリファラを求める
        const std::string referer( const std::string &url );

      private:

        void conf2list( const std::string& conf );

        // 置換文字列を変換
        void replace( JDLIB::Regex &regex, std::string &str );
    };

    ///////////////////////////////////////
    // インターフェース

    Urlreplace_Manager* get_urlreplace_manager();
    void delete_urlreplace_manager();
}


#endif
