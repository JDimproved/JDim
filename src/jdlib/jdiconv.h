// ライセンス: GPL2

#ifndef _JDICONV_H
#define _JDICONV_H

#include "jdencoding.h"

#include <string>
#include <vector>

#include <gmodule.h> // GIConv


namespace JDLIB
{
    /** @brief テキストの文字エンコーディングを変換するクラス */
    class Iconv
    {
        GIConv m_cd; ///< iconv実装は環境で違いがあるためGlibのラッパーAPIを利用する

        std::string m_buf; ///< 出力バッファ

        Encoding m_enc_to;   ///< 変換先の文字エンコーディング
        Encoding m_enc_from; ///< 変換元の文字エンコーディング
        bool m_broken_sjis_be_utf8; ///< trueなら不正なMS932文字列をUTF-8と見なす (MS932 -> UTF-8の変換限定)

    public:

        Iconv( const std::string& coding_to, const std::string& coding_from );
        Iconv( const std::string& coding_to, const std::string& coding_from, const bool broken_sjis_be_utf8 );
        Iconv( const Encoding to, const Encoding from );
        Iconv( const Encoding to, const Encoding from, const bool broken_sjis_be_utf8 );
        ~Iconv();

        // テキストの文字エンコーディングを変換する
        const std::string& convert( char* str_in, std::size_t size_in );
        std::string& convert( char* str_in, std::size_t size_in, std::string& out_buf );

    private:
        void open_by_alternative_names( const char* to_str, const char* from_str );
    };
}

#endif
