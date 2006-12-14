// ライセンス: GPL2

#ifndef _JDICONV_H
#define _JDICONV_H

#include <iconv.h>
#include <string>

// iconv の内部で確保するバッファサイズ(バイト)
//  BUF_SIZE_ICONV_IN を超える入力は扱えないので注意
#define BUF_SIZE_ICONV_IN ( 1024 * 1024 )
#define BUF_SIZE_ICONV_OUT ( BUF_SIZE_ICONV_IN /2 * 3 )

namespace JDLIB
{
    class Iconv
    {
        iconv_t m_cd;

        size_t m_byte_left_in;
        char* m_buf_in;
        char* m_buf_in_tmp;

        char* m_buf_out;

        std::string m_coding_from;

    public:
        
        Iconv( const std::string& coding_from, const std::string& coding_to );
        ~Iconv();

        const char* convert( char* str_in, int size_in, int& size_out );
    };
}

#endif
