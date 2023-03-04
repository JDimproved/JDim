// ライセンス: GPL2
//
// テキストファイルの簡易ローダ
//
// ロードしたファイルはget_path()で示されたパスに保存される
// get_path() が empty() ならば保存しない
//

#ifndef _TEXTLODER_H
#define _TEXTLODER_H

#include "jdencoding.h"
#include "loadable.h"

#include <string>

namespace JDLIB
{
    class LOADERDATA;
}

namespace SKELETON
{
    class TextLoader : public SKELETON::Loadable
    {
        bool m_loaded{}; // 読み込み済みか

        std::string m_rawdata;
        std::string m_data;

      public:

        TextLoader();
        ~TextLoader();

        const std::string& get_data() const { return m_data; }

        // 一度ロードしたらreset()を呼ばない限りリロードしない
        void reset();

        // キャッシュからロード
        void load_text( const Encoding encoding );

        // ダウンロードを開始する
        // HTTP 304 Not Modified の時はキャッシュから読み込む
        void download_text( const Encoding encoding );

      protected:

        virtual std::string get_url() const = 0;
        virtual std::string get_path() const = 0;

        // ロード用データ作成
        virtual void create_loaderdata( JDLIB::LOADERDATA& data ) = 0;

        // ロード後に呼び出される
        virtual void parse_data() = 0;

      private:

        void init();
        void clear();

        void receive_data( const char* data, size_t size ) override;
        void receive_finish() override;

        // HTTP応答ヘッダーのクッキーを取り扱う場合は派生クラスでoverrideする
        virtual void receive_cookies() {}
    };
}

#endif
