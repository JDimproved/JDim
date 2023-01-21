// ライセンス: GPL2

#ifndef _JDICONV_H
#define _JDICONV_H

#include <string>
#include <vector>

#include <gmodule.h> // GIConv


// iconv の内部で確保するバッファサイズ(バイト)
constexpr int BUF_SIZE_ICONV_OUT = 512 * 1024;


namespace JDLIB
{
    /** @brief テキストの文字エンコーディングを変換するクラス */
    class Iconv
    {
        GIConv m_cd; ///< iconv実装は環境で違いがあるためGlibのラッパーAPIを利用する

        std::vector<char> m_buf; ///< 出力バッファ

        std::string m_coding_from; ///< 変換元の文字エンコーディング

    public:
        
        Iconv( const std::string& coding_to, const std::string& coding_from );
        ~Iconv();

        // テキストの文字エンコーディングを変換する
        const char* convert( char* str_in, int size_in, int& size_out );
    };
}

#endif
