// ライセンス: GPL2
//
// 2chのSETTING.TXTのローダ
//

#ifndef _SETTINGLOADER_H
#define _SETTINGLOADER_H

#include "skeleton/textloader.h"

#include <string>

namespace JDLIB
{
    class LOADERDATA;
}

namespace DBTREE
{
    class SettingLoader : public SKELETON::TextLoader
    {
        std::string m_url_boadbase;

        // デフォルト名無し
        std::string m_default_noname;

        // 最大改行数/2
        int m_line_number;

        // 最大書き込みバイト数
        int m_message_count;

        // 特殊文字書き込み
        std::string m_unicode;

      public:

        SettingLoader( const std::string& url_boardbase );
        ~SettingLoader();

        const std::string& default_noname() const { return m_default_noname; }
        int line_number() { return m_line_number; }
        int message_count() { return m_message_count; }
        const std::string& get_unicode() const { return m_unicode; }

      protected:

        const std::string get_url() override;
        const std::string get_path() override;
        const std::string get_charset() override;

        // ロード用データ作成
        void create_loaderdata( JDLIB::LOADERDATA& data ) override;

        // ロード後に呼び出される
        void parse_data() override;
    };
}

#endif
