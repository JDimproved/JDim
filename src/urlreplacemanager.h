// ライセンス: GPL2

#ifndef _URLREPLACEMANAGER_H
#define _URLREPLACEMANAGER_H

#include "jdlib/jdregex.h"

#include <string>
#include <vector>


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
        JDLIB::RegexPattern creg;
        std::string replace;
        std::string referer;
        int imgctrl;
        bool match_break;
    };

    class Urlreplace_Manager
    {
        std::vector<UrlreplaceItem> m_list_cmd;

      public:

        Urlreplace_Manager();
        virtual ~Urlreplace_Manager() noexcept;

        // URLを任意の正規表現で変換する
        bool exec( std::string& url );

        // URLからリファラを求める
        bool referer( const std::string& url, std::string& refstr );

        // URLの画像コントロールを取得する
        int get_imgctrl( const std::string& url );

      private:

        void conf2list( const std::string& conf );
    };

    ///////////////////////////////////////
    // インターフェース

    Urlreplace_Manager* get_urlreplace_manager();
    void delete_urlreplace_manager();
}


#endif
