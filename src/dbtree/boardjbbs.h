// ライセンス: GPL2

//
// JBBS 型板
//

#ifndef _BOARDJBBS_H
#define _BOARDJBBS_H

#include "boardbase.h"

#include <memory>


namespace DBTREE
{
    class RuleLoader;
    class SettingLoader;

    class BoardJBBS : public BoardBase
    {
        std::unique_ptr<RuleLoader> m_ruleloader;
        std::unique_ptr<SettingLoader> m_settingloader;

      public:

        BoardJBBS( const std::string& root, const std::string& path_board,const std::string& name );
        ~BoardJBBS() noexcept override;

        std::string url_datpath() const override;

        // 新スレ作成時の書き込みメッセージ作成
        std::string create_newarticle_message( const std::string& subject, const std::string& name,
                                               const std::string& mail, const std::string& msg,
                                               const bool utf8_post ) override;

        // 新スレ作成用のbbscgi のURL
        std::string url_bbscgi_new() const override;
        
        // 新スレ作成用のsubbbscgi のURL
        std::string url_subbbscgi_new() const override;

        // ローカルルール
        std::string localrule() const override;

        // SETTING.TXT
        std::string settingtxt() const override;
        std::string default_noname() const override;

        // SETTING.TXT のURL
        std::string url_settingtxt() const override;

      private:

        bool is_valid( const std::string& filename ) const override;
        ArticleBase* append_article( const std::string& datbase, const std::string& id, const bool cached ) override;
        void parse_subject( const char* str_subject_txt ) override;
        void regist_article( const bool is_online ) override;

        void load_rule_setting() override;
        void download_rule_setting() override;
    };
}

#endif
