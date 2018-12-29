// ライセンス: GPL2

#ifndef _URLREPLACEMANAGER_H
#define _URLREPLACEMANAGER_H

#include <string>
#include <list>
#include <map>

#include "jdlib/jdregex.h"

namespace CORE
{
    enum
    {
        IMGCTRL_INIT = 0,
        IMGCTRL_NONE = 1,

        IMGCTRL_FORCEIMAGE = 2,
        IMGCTRL_FORCEBROWSER = 4,
        IMGCTRL_GENUINE = 8,
        IMGCTRL_THUMBNAIL = 16,
    };

    struct UrlreplaceItem
    {
        std::string match;
        std::string replace;
        std::string referer;
        int imgctrl;
        bool match_break;
    };

    class Urlreplace_Manager
    {
        std::list< UrlreplaceItem > m_list_cmd;
        std::map< std::string, int > m_map_imgctrl; // 画像コントロールのキャッシュ

      public:

        Urlreplace_Manager();
        virtual ~Urlreplace_Manager() noexcept {}

        // URLを任意の正規表現で変換する
        const bool exec( std::string &url );

        // URLからリファラを求める
        const bool referer( const std::string &url, std::string &referer );

        // URLの画像コントロールを取得する
        const int get_imgctrl( const std::string &url );

      private:

        void conf2list( const std::string& conf );
        const int get_imgctrl_impl( const std::string &url );

        // 置換文字列を変換
        void replace( JDLIB::Regex &regex, std::string &str );
    };

    ///////////////////////////////////////
    // インターフェース

    Urlreplace_Manager* get_urlreplace_manager();
    void delete_urlreplace_manager();
}


#endif
