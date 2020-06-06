// ライセンス: GPL2

//
// 2ch 
//

#ifndef _BOARD2CH_H
#define _BOARD2CH_H

#include "board2chcompati.h"

namespace DBTREE
{
    enum
    {
        DEFAULT_NUMBER_MAX_2CH = 1000,  // デフォルト最大レス数
        DEFAULT_MAX_DAT_LNG = 512 // デフォルトのdatの最大サイズ(Kバイト)
    };

    class Board2ch : public Board2chCompati
    {
      public:

        Board2ch( const std::string& root, const std::string& path_board,const std::string& name );
        ~Board2ch() noexcept;

        // ユーザーエージェント
        const std::string& get_agent() override; // ダウンロード用
        const std::string& get_agent_w() override; // 書き込み用

        // 読み込み用プロキシ
        std::string get_proxy_host() override;
        int get_proxy_port() override;
        std::string get_proxy_basicauth() override;

        // 書き込み用プロキシ
        std::string get_proxy_host_w() override;
        int get_proxy_port_w() override;
        std::string get_proxy_basicauth_w() override;

        // 読み書き用クッキー
        std::string cookie_for_request() const override;

        // 書き込み時のリファラ
        std::string get_write_referer() override;

        // 新スレ作成用のメッセージ変換
        std::string create_newarticle_message( const std::string& subject, const std::string& name,
                                               const std::string& mail, const std::string& msg ) override;

        // 新スレ作成用のbbscgi のURL
        std::string url_bbscgi_new() override;
        
        // 新スレ作成用のsubbbscgi のURL
        std::string url_subbbscgi_new() override;

        // datの最大サイズ(Kバイト)
        int get_max_dat_lng() const override { return DEFAULT_MAX_DAT_LNG; }

      protected:

        // クッキー
        std::string get_hap() const override;
        void set_hap( const std::string& hap ) override;

        // クッキーの更新 (クッキーをセットした時に実行)
        void update_hap() override;

      private:

        // デフォルト最大レス数
        int get_default_number_max_res() override { return DEFAULT_NUMBER_MAX_2CH; }

        ArticleBase* append_article( const std::string& datbase, const std::string& id, const bool cached ) override;
    };
}

#endif
