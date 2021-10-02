// ライセンス: GPL2

//
// 2ch 互換型板
//

#ifndef _BOARD2CHCOMPATI_H
#define _BOARD2CHCOMPATI_H

#include "boardbase.h"

#include <memory>


namespace DBTREE
{
    class SettingLoader;
    class RuleLoader;

    class Board2chCompati : public BoardBase
    {
        std::unique_ptr<SettingLoader> m_settingloader;
        std::unique_ptr<RuleLoader> m_ruleloader;

      public:

        Board2chCompati( const std::string& root, const std::string& path_board, const std::string& name, const std::string& basicauth );
        ~Board2chCompati();

        // 書き込み時に必要なキーワード( hana=mogera や suka=pontan など )を
        // 確認画面のhtmlから解析する      
        void analyze_keyword_for_write( const std::string& html ) override;

        // スレ立て時に必要なキーワードをフロントページのhtmlから解析する
        void analyze_keyword_for_newarticle( const std::string& html ) override;

        // 確認画面のHTMLから書き込み、スレ立て時に使うフォームデータを取得する
        std::string parse_form_data( const std::string& html ) override;

        // 新スレ作成用のメッセージ変換
        std::string create_newarticle_message( const std::string& subject, const std::string& name,
                                               const std::string& mail, const std::string& msg ) override;

        // 新スレ作成用のbbscgi のURL
        std::string url_bbscgi_new() const override;
        
        // 新スレ作成用のsubbbscgi のURL
        std::string url_subbbscgi_new() const override;

        // ローカルルール
        std::string localrule() const override;

        // SETTING.TXT のURL
        std::string url_settingtxt() const override;

        // SETTING.TXT 
        std::string settingtxt() const override;
        std::string default_noname() const override;
        int line_number() const override;
        int message_count() const override;
        std::string get_unicode() const override;

      private:

        bool is_valid( const std::string& filename ) const override;

        ArticleBase* append_article( const std::string& datbase, const std::string& id, const bool cached ) override;
        void parse_subject( const char* str_subject_txt ) override;
        void regist_article( const bool is_online ) override;

        void load_rule_setting() override;
        void download_rule_setting() override;

        // レス数であぼーん(グローバル)
        int get_abone_low_number_global() const override;
        int get_abone_high_number_global() const override;

        // htmlからキーワードを解析する
        std::string analyze_keyword_impl( const std::string& html, bool full_parse );
    };
}

#endif
