// ライセンス: GPL2
//
// 2chのSETTING.TXTのローダ
//

#ifndef _SETTINGLOADER_H
#define _SETTINGLOADER_H

#include "skeleton/loadable.h"

#include <string>

namespace JDLIB
{
    class LOADERDATA;
}


namespace DBTREE
{
    class SettingLoader : public SKELETON::Loadable
    {
        bool m_loaded; // 読み込み済みか

        std::string m_url_boadbase;
        char* m_rawdata;
        int m_lng_rawdata;

        std::string m_settingtxt;

        // デフォルト名無し
        std::string m_default_noname;

        // 最大改行数/2
        int m_line_number;

        // 最大書き込みバイト数
        int m_message_count;

      public:
        SettingLoader( const std::string& url_boardbase );
        ~SettingLoader();

        const std::string& settingtxt() const { return m_settingtxt; }
        const std::string& default_noname() const { return m_default_noname; }
        const int line_number() { return m_line_number; }
        const int message_count() { return m_message_count; }

        void load_setting();
        void download_setting();

      private:

        void clear();
        void parse( const std::string& setting );

        virtual void receive_data( const char* data, size_t size );
        virtual void receive_finish();
    };
}

#endif
