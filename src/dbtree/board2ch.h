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
        virtual ~Board2ch();

        // ユーザーエージェント
        virtual const std::string& get_agent(); // ダウンロード用
        virtual const std::string& get_agent_w(); // 書き込み用

        // 読み込み用プロキシ
        virtual const std::string get_proxy_host();
        virtual const int get_proxy_port();
        virtual const std::string get_proxy_basicauth();

        // 書き込み用プロキシ
        virtual const std::string get_proxy_host_w();
        virtual const int get_proxy_port_w();
        virtual const std::string get_proxy_basicauth_w();

        // 書き込み用クッキー
        virtual const std::string cookie_for_write();

        // 書き込み時のリファラ
        virtual const std::string get_write_referer();

        // 新スレ作成用のメッセージ変換
        virtual const std::string create_newarticle_message( const std::string& subject,
                                                             const std::string& name, const std::string& mail, const std::string& msg );

        // 新スレ作成用のbbscgi のURL
        virtual const std::string url_bbscgi_new();
        
        // 新スレ作成用のsubbbscgi のURL
        virtual const std::string url_subbbscgi_new();

        // datの最大サイズ(Kバイト)
        virtual const int get_max_dat_lng() const { return DEFAULT_MAX_DAT_LNG; }

      protected:

        // 冒険の書(HAP)
        virtual const std::string get_hap();

        // 冒険の書(HAP)の更新 (クッキーをセットした時に実行)
        virtual void update_hap();

      private:

        // デフォルト最大レス数
        virtual const int get_default_number_max_res() { return DEFAULT_NUMBER_MAX_2CH; }

        virtual ArticleBase* append_article( const std::string& datbase, const std::string& id, const bool cached );
    };
}

#endif
