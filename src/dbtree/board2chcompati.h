// ライセンス: GPL2

//
// 2ch 互換型板
//

#ifndef _BOARD2CHCOMPATI_H
#define _BOARD2CHCOMPATI_H

#include "boardbase.h"

namespace DBTREE
{
    class SettingLoader;
    class RuleLoader;

    class Board2chCompati : public BoardBase
    {
        SettingLoader* m_settingloader;
        RuleLoader* m_ruleloader;

      public:

        Board2chCompati( const std::string& root, const std::string& path_board, const std::string& name, const std::string& basicauth );
        virtual ~Board2chCompati();

        // 書き込み用クッキー
        virtual const std::string cookie_for_write();

        // 新スレ作成用のメッセージ変換
        virtual const std::string create_newarticle_message( const std::string& subject,
                                                             const std::string& name, const std::string& mail, const std::string& msg );
        // 新スレ作成用のbbscgi のURL
        virtual const std::string url_bbscgi_new();
        
        // 新スレ作成用のsubbbscgi のURL
        virtual const std::string url_subbbscgi_new();

        // ローカルルール
        virtual const std::string localrule();

        // SETTING.TXT 
        virtual const std::string settingtxt();
        virtual const std::string default_noname();
        virtual const int line_number();
        virtual const int message_count();

      private:

        virtual bool is_valid( const std::string& filename );

        virtual ArticleBase* append_article( const std::string& id, bool cached );
        virtual void parse_subject( const char* str_subject_txt );

        virtual void load_rule_setting();
        virtual void download_rule_setting();

        // レス数であぼーん(グローバル)
        virtual const int get_abone_number_global();
    };
}

#endif
