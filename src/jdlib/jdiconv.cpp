// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "jdiconv.h"
#include "miscmsg.h"

#include <errno.h>

using namespace JDLIB;

Iconv::Iconv( const std::string& coding_from, const std::string& coding_to )
    : m_buf_in( 0 ), m_buf_out( 0 ), m_coding_from( coding_from )
{
#ifdef _DEBUG
    std::cout << "Iconv::Iconv coding = " << m_coding_from << " to " << coding_to << std::endl;
#endif
    
    m_buf_in = ( char* )malloc( BUF_SIZE_ICONV_IN );
    m_buf_out = ( char* )malloc( BUF_SIZE_ICONV_OUT );
    
    m_cd = iconv_open( coding_to.c_str(), m_coding_from.c_str() ); 

    // MS932で失敗したらCP932で試してみる
    if( m_cd == ( iconv_t ) -1 ){
        if( coding_to == "MS932" ) m_cd = iconv_open( "CP932", m_coding_from.c_str() );
        else if( coding_from == "MS932" ) m_cd = iconv_open( coding_to.c_str(), "CP932" ); 
    }

    if( m_cd == ( iconv_t ) -1 ){
        MISC::ERRMSG( "can't open iconv coding = " + m_coding_from + " to " + coding_to );
    }
    m_byte_left_in = 0;
}

Iconv::~Iconv()
{
#ifdef _DEBUG
    std::cout << "Iconv::~Iconv\n";
#endif    
    
    if( m_buf_in ) free( m_buf_in );
    if( m_buf_out ) free( m_buf_out );
    if( m_cd != ( iconv_t ) -1 ) iconv_close( m_cd );
}


const char* Iconv::convert( char* str_in, int size_in, int& size_out )
{
#ifdef _DEBUG
    std::cout << "Iconv::convert size_in = " << size_in 
              <<" left = " << m_byte_left_in << std::endl;
#endif

    assert( m_byte_left_in + size_in < BUF_SIZE_ICONV_IN );
    if( m_cd == ( iconv_t ) -1 ) return NULL;
    
    size_t byte_left_out = BUF_SIZE_ICONV_OUT;
    char* buf_out = m_buf_out;

    // 前回の残りをコピー
    if( m_byte_left_in ){
        memcpy( m_buf_in, m_buf_in_tmp, m_byte_left_in );
        m_buf_in_tmp = m_buf_in;
        memcpy( m_buf_in + m_byte_left_in , str_in, size_in );    
    }
    else m_buf_in_tmp = str_in;

    m_byte_left_in += size_in;

    // iconv 実行
    do{

#ifdef _DEBUG
        std::cout << "m_byte_left_in = " << m_byte_left_in << std::endl;
        std::cout << "byte_left_out = " << byte_left_out << std::endl;
#endif
    
        int ret = iconv( m_cd, ( ICONV_CONST char**)&m_buf_in_tmp, &m_byte_left_in, &buf_out, &byte_left_out );

#ifdef _DEBUG
        std::cout << "--> ret = " << ret << std::endl;
        std::cout << "m_byte_left_in = " << m_byte_left_in << std::endl;
        std::cout << "byte_left_out = " << byte_left_out << std::endl;
#endif
        assert( byte_left_out >= 0 );

        //　エラー
        if( ret == -1 ){

            if( errno == EILSEQ ){

                char str_tmp[256];
                unsigned char code0 = *m_buf_in_tmp;
                unsigned char code1 = *(m_buf_in_tmp+1);
                unsigned char code2 = *(m_buf_in_tmp+2);

                if( m_coding_from == "MS932" )
                {

                    // 空白(0xa0)
                    if( code0 == 0xa0 ){
                        *m_buf_in_tmp = 0x20;
                        continue;
                    }

                    // マッピング失敗
                    // □(0x81a0)を表示する
                    if( ( code0 >= 0x81 && code0 <=0x9F )
                        || ( code0 >= 0xe0 && code0 <=0xef ) ){

                        *m_buf_in_tmp = 0x81;
                        *(m_buf_in_tmp+1) = 0xa0;

                        snprintf( str_tmp, 256, "iconv 0x%x%x -> □ (0x81a0) ", code0, code1 );
                        MISC::MSG( str_tmp );
                        continue;
                    }
                }

                //その他、1文字を空白にして続行
                snprintf( str_tmp, 256, "iconv EILSEQ left = %zd code = %x %x %x", m_byte_left_in, code0, code1, code2 );
                MISC::ERRMSG( str_tmp );
                *m_buf_in_tmp = 0x20;
            }

            else if( errno == EINVAL ){
                MISC::ERRMSG( "iconv EINVAL\n" );
                break;
            }

            else if( errno == E2BIG ){
                MISC::ERRMSG( "iconv E2BIG\n" );
                break;
            }
        }
    
    } while( m_byte_left_in > 0 );

    size_out = BUF_SIZE_ICONV_OUT - byte_left_out;
    m_buf_out[ size_out ] = '\0';
    
    return m_buf_out;
}
