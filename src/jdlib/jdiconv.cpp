// ライセンス: GPL2

//#define _DEBUG
//#define _DEBUG_ICONV
#include "jddebug.h"

#include "jdiconv.h"
#include "misccharcode.h"
#include "miscmsg.h"

#include <cstdio>
#include <errno.h>


using namespace JDLIB;


/** @brief コンストラクタ
 *
 * @param[in] coding_to   変換先の文字エンコーディング
 * @param[in] coding_from 変換元の文字エンコーディング
 */
Iconv::Iconv( const std::string& coding_to, const std::string& coding_from )
    : m_coding_from( coding_from )
{
#ifdef _DEBUG
    std::cout << "Iconv::Iconv coding = " << m_coding_from << " to " << coding_to << std::endl;
#endif

    m_cd = g_iconv_open( coding_to.c_str(), m_coding_from.c_str() );

    // MS932で失敗したらCP932で試してみる
    if( m_cd == ( GIConv ) -1 ){
        if( coding_to == "MS932" ) m_cd = g_iconv_open( "CP932", m_coding_from.c_str() );
        else if( coding_from == "MS932" ) m_cd = g_iconv_open( coding_to.c_str(), "CP932" );
    }

    // "EUCJP-*"で失敗したら"EUCJP"で試してみる
    if( m_cd == ( GIConv ) - 1 && ( errno & EINVAL ) != 0 )
    {
        if( coding_to.rfind( "EUCJP-", 0 ) == 0 )
        {
            m_cd = g_iconv_open( "EUCJP//TRANSLIT", coding_from.c_str() );
        }
        else if( coding_from.rfind( "EUCJP-", 0 ) == 0 )
        {
            const std::string coding_to_translit = coding_to + "//TRANSLIT";
            m_cd = g_iconv_open( coding_to_translit.c_str(), "EUCJP" );
        }
    }

    if( m_cd == ( GIConv ) -1 ){
        MISC::ERRMSG( "can't open iconv coding = " + m_coding_from + " to " + coding_to );
    }
}

Iconv::~Iconv()
{
#ifdef _DEBUG
    std::cout << "Iconv::~Iconv\n";
#endif

    if( m_cd != ( GIConv ) -1 ) g_iconv_close( m_cd );
}


/** @brief テキストの文字エンコーディングを変換する
 *
 * @details UTF-8から別のエンコーディングに変換するときは表現できない文字をHTML数値文字参照(10進数表記)に置き換える。
 * @param[in] str_in    変換するテキストへのポインター
 * @param[in] size_in   変換するテキストの長さ
 * @param[out] size_out 変換した結果の長さ
 * @return 変換したテキストへのポインター。
 * @note 返り値は Iconv オブジェクトを破棄、または convert() を再び呼び出すとdangling pointerになる。
 */
const char* Iconv::convert( char* str_in, int size_in, int& size_out )
{
#ifdef _DEBUG
    std::cout << "Iconv::convert size_in = " << size_in << std::endl;
#endif

    if( m_cd == ( GIConv ) -1 ) return nullptr;

    if( const std::size_t size_in_x2 = size_in * 2;
            size_in_x2 >= m_buf.size() ) {
        m_buf.resize( size_in > 0 ? size_in_x2 : 1 );
    }

    char* buf_in_tmp = str_in;
    const char* buf_in_end = str_in + size_in;

    char* buf_out_tmp = m_buf.data();
    char* buf_out_end = m_buf.data() + m_buf.size();

    const auto grow = [&] {
        const std::size_t used = buf_out_tmp - m_buf.data();
        m_buf.resize( m_buf.size() * 2 );
        buf_out_tmp = m_buf.data() + used;
        buf_out_end = m_buf.data() + m_buf.size();
    };

    // iconv 実行
    do{
        std::size_t byte_left_in = buf_in_end - buf_in_tmp;
        std::size_t byte_left_out = buf_out_end - buf_out_tmp;

#ifdef _DEBUG
        std::cout << "byte_left_in = " << byte_left_in << std::endl;
        std::cout << "byte_left_out = " << byte_left_out << std::endl;
#endif

        const int ret = g_iconv( m_cd, &buf_in_tmp, &byte_left_in, &buf_out_tmp, &byte_left_out );

#ifdef _DEBUG
        std::cout << "--> ret = " << ret << std::endl;
        std::cout << "byte_left_in = " << byte_left_in << std::endl;
        std::cout << "byte_left_out = " << byte_left_out << std::endl;
#endif

        //　エラー
        if( ret == -1 ){

            if( errno == EILSEQ ){

#ifdef _DEBUG_ICONV
                char str_tmp[256];
#endif
                const unsigned char code0 = *buf_in_tmp;
                const unsigned char code1 = buf_in_end > ( buf_in_tmp + 1 ) ? buf_in_tmp[1] : 0;
                const unsigned char code2 = buf_in_end > ( buf_in_tmp + 2 ) ? buf_in_tmp[2] : 0;

                if( m_coding_from == "MS932" )
                {
                    // 空白(0xa0)
                    if( code0 == 0xa0 ){
                        *buf_in_tmp = 0x20;
                        continue;
                    }

                    // <>の誤判別 ( 開発スレ 489 を参照 )
                    if( code1 == 0x3c && code2 == 0x3e ){
                        *buf_in_tmp = '?';
#ifdef _DEBUG_ICONV
                        snprintf( str_tmp, 256, "iconv 0x%x%x> -> ?<>", code0, code1 );
                        MISC::MSG( str_tmp );
#endif
                        continue;
                    }

                    // マッピング失敗
                    // □(0x81a0)を表示する
                    if( ( code0 >= 0x81 && code0 <=0x9F )
                        || ( code0 >= 0xe0 && code0 <=0xef ) ){

                        buf_in_tmp[0] = static_cast<char>( 0x81 );
                        buf_in_tmp[1] = static_cast<char>( 0xa0 );

#ifdef _DEBUG_ICONV
                        snprintf( str_tmp, 256, "iconv 0x%x%x -> □ (0x81a0) ", code0, code1 );
                        MISC::MSG( str_tmp );
#endif
                        continue;
                    }
                }

                // unicode 文字からの変換失敗
                // 数値文字参照(&#????;)形式にする
                if( m_coding_from == "UTF-8" ){

                    // https://github.com/JDimproved/JDim/issues/214 （emoji subdivision flagの処理）について
                    //
                    // TAG LATIN SMALL LETTER と CANCEL TAG については、libcのiconv()関数にかけると、
                    // エラーを返さず、なおかつこれらの文字を無視してしまうので、
                    // U+1F3F4 WAVING BLACK FLAGが現れた時は、上の範囲のUTF-8文字が続いてないか、
                    // このblockの範囲で確認してから、脱出する。


                    bool is_converted_to_ucs2 = false;  // 数値文字参照に変換されたかどうか
                    bool is_handling_emoji_subdivision_flag = false;  // emoji subdivision flags の処理の途中か

                    for ( ; ; ) {
                        int byte;
                        const char32_t unich = MISC::utf8toutf32( buf_in_tmp, byte );
                        if( byte <= 1 ) break;

                        // emoji subdivision flags の処理
                        if ( is_handling_emoji_subdivision_flag ) {
                            // Tag Latin Small Letterの範囲か、Cancel Tagでなければ、処理中断
                            if ( byte != 4 ) break;
                            if ( unich < 917601 ) break; // U+E0061 TAG LATIN SMALL LETTER A
                            if ( unich > 917631 ) break; // U+E007F CANCEL TAG
                        }

                        const std::string uni_str = std::to_string( unich );
#ifdef _DEBUG
                        std::cout << "utf32 = " << unich << " byte = " << byte << std::endl;
#endif
                        buf_in_tmp += byte;

                        if( ( buf_out_tmp + uni_str.size() + 3 ) >= buf_out_end ) {
                            grow();
                        }

                        *(buf_out_tmp++) = '&';
                        *(buf_out_tmp++) = '#';
                        uni_str.copy( buf_out_tmp, uni_str.size() );
                        buf_out_tmp += uni_str.size();
                        *(buf_out_tmp++) = ';';

                        byte_left_out -= uni_str.size() + 3;
                        is_converted_to_ucs2 = true;  // 一度変換されたのでマーク

                        if ( ! is_handling_emoji_subdivision_flag ) {
                            if ( ( byte == 4 ) && ( unich == 127988 ) ){ // U+1F3F4 WAVING BLACK FLAG
                                // emoji subdivision flags の処理開始
                                is_handling_emoji_subdivision_flag = true;
                                continue; // 連続処理
                            }
                        } else {
                            // まだ emoji subdivision flags の処理中
                            continue;
                        }

                        break;
                    }

                    // 数値文字参照に変換された場合は、continueする
                    if ( is_converted_to_ucs2 ) continue;

                }

                // 時々空白(0x20)で EILSEQ が出るときがあるのでもう一度トライする
                if( code0 == 0x20 ) continue;

#ifdef _DEBUG_ICONV
                std::snprintf( str_tmp, 256, "iconv EILSEQ left = %zu code = %x %x %x", byte_left_in, code0, code1, code2 );
                MISC::ERRMSG( str_tmp );
#endif

                // BOFの確認
                if( ( buf_out_end - buf_out_tmp ) <= 3 ) {
                    grow();
                }

                //その他、1文字を?にして続行
                ++buf_in_tmp;
                *(buf_out_tmp++) = '?';
            }

            else if( errno == EINVAL ){
#ifdef _DEBUG_ICONV
                MISC::ERRMSG( "iconv EINVAL\n" );
#endif
                break;
            }

            else if( errno == E2BIG ){
#ifdef _DEBUG_ICONV
                MISC::ERRMSG( "iconv E2BIG\n" );
#endif
                grow();
                continue;
            }
        }

    } while( buf_in_tmp < buf_in_end );

    size_out = buf_out_tmp - m_buf.data();
    *buf_out_tmp = '\0';

#ifdef _DEBUG
    std::cout << "Iconv::convert size_out = " << size_out << std::endl;
#endif
    return m_buf.data();
}
