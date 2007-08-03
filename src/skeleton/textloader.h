// ライセンス: GPL2
//
// テキストファイルの簡易ローダ
//

#ifndef _TEXTLODER_H
#define _TEXTLODER_H

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
        bool m_loaded; // 読み込み済みか

        char* m_rawdata;
        int m_lng_rawdata;
        std::string m_data;

      public:

        TextLoader();
        ~TextLoader();

        const std::string& get_data() const { return m_data; }

        void load_text();
        void download_text();

      protected:

        virtual const std::string get_url() = 0;
        virtual const std::string get_path() = 0;
        virtual const std::string get_charset() = 0;

        // ロード用データ作成
        virtual void create_loaderdata( JDLIB::LOADERDATA& data ) = 0;

        // ロード後に呼び出される
        virtual void parse_data() = 0;

      private:

        void init();
        void clear();

        virtual void receive_data( const char* data, size_t size );
        virtual void receive_finish();
    };
}

#endif
