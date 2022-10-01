// ライセンス: GPL2

//
// 文字列置換の管理クラス
//

#ifndef REPLACESTRMANAGER_H
#define REPLACESTRMANAGER_H

#include "jdlib/jdregex.h"

#include <array>
#include <list>
#include <string>
#include <string_view>

namespace XML
{
    class Dom;
}

namespace CORE
{
    enum
    {
        REPLACETARGET_SUBJECT = 0,
        REPLACETARGET_NAME,
        REPLACETARGET_MAIL,
        REPLACETARGET_DATE,
        REPLACETARGET_MESSAGE,
        REPLACETARGET_MAX
    };
    constexpr std::array<const char*, REPLACETARGET_MAX> kReplStrTargetNames = {
        "subject", "name", "mail", "date", "msg"
    };
    constexpr std::array<const char*, REPLACETARGET_MAX> kReplStrTargetLabels = {
        "スレタイトル", "名前", "メール", "日付/ID", "本文"
    };

    struct ReplaceStrCondition
    {
        // データ互換性のためフラグの並び(ビット位置)は変更しない
        unsigned char active : 1; // 0
        unsigned char icase  : 1; // 1
        unsigned char regex  : 1; // 2
        unsigned char wchar  : 1; // 3
        unsigned char norm   : 1; // 4 reserved

        static ReplaceStrCondition from_ulong( unsigned long condition ) noexcept;
        unsigned long to_ulong() const;
    };

    struct ReplaceStrItem
    {
        ReplaceStrCondition condition;
        std::string pattern;
        std::string replace;
        JDLIB::RegexPattern creg;
    };

    class ReplaceStr_Manager
    {
        std::list<ReplaceStrItem> m_list[ REPLACETARGET_MAX ]{};
        bool m_chref[ REPLACETARGET_MAX ]{};

      public:

        ReplaceStr_Manager();
        ~ReplaceStr_Manager() noexcept = default;

        const std::list<ReplaceStrItem>& get_list( int id ) const noexcept { return m_list[id]; }
        bool list_get_active( const int id ) const;
        void list_clear( const int id );
        void list_append( const int id, ReplaceStrCondition condition,
                          const std::string& pattern, const std::string& replace );

        bool get_chref( int id ) const noexcept { return m_chref[id]; }
        void set_chref( int id, bool chref ) { m_chref[id] = chref; }

        void save_xml();

        // 文字列を置換
        std::string replace( std::string_view str, const int id ) const;

        static XML::Dom* dom_append( XML::Dom* node, const int id, const bool chref );
        static XML::Dom* dom_append( XML::Dom* node, ReplaceStrCondition condition,
                                     const std::string& pattern, const std::string& replace );

      private:

        void xml2list( const std::string& xml );

        static int target_id( const std::string& name );
        static std::string target_name( const int id );
    };

    ///////////////////////////////////////
    // インターフェース

    ReplaceStr_Manager* get_replacestr_manager();
    void delete_replacestr_manager();
}

#endif
