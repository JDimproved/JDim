// ライセンス: GPL2

#ifndef _JDICONV_H
#define _JDICONV_H

#include <string>
#include <vector>

#include <gmodule.h> // GIConv


// iconv の内部で確保するバッファサイズ(バイト)
//  BUF_SIZE_ICONV_IN を超える入力は扱えないので注意
enum
{
    BUF_SIZE_ICONV_IN = 1024 * 1024,
    BUF_SIZE_ICONV_OUT = BUF_SIZE_ICONV_IN /2 * 3
};

namespace JDLIB
{
    class Iconv
    {
        GIConv m_cd; // iconv実装は環境で違いがあるためGlibのラッパーAPIを利用する

        size_t m_byte_left_in{};
        std::vector<char> m_buf_in;
        char* m_buf_in_tmp{};

        std::vector<char> m_buf_out;

        std::string m_coding_from;

    public:
        
        Iconv( const std::string& coding_to, const std::string& coding_from );
        ~Iconv();

        const char* convert( char* str_in, int size_in, int& size_out );
    };
}

#endif
