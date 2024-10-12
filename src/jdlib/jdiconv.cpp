// ライセンス: GPL2

//#define _DEBUG
//#define _DEBUG_ICONV
#include "jddebug.h"

#include "jdiconv.h"
#include "misccharcode.h"
#include "miscmsg.h"

#include "config/globalconf.h"

#include <cstdio>
#include <cstring>
#include <errno.h>
#include <string_view>


using namespace JDLIB;


/** @brief コンストラクタ
 *
 * @details broken_sjis_be_utf8 の値は about:config の設定を使う
 * @param[in] to   変換先の文字エンコーディング
 * @param[in] from 変換元の文字エンコーディング
 */
Iconv::Iconv( const Encoding to, const Encoding from )
    : Iconv( to, from, CONFIG::get_broken_sjis_be_utf8() )
{
}


/** @brief コンストラクタ
 *
 * @param[in] to   変換先の文字エンコーディング
 * @param[in] from 変換元の文字エンコーディング
 * @param[in] broken_sjis_be_utf8 trueなら不正なMS932文字列をUTF-8と見なす (MS932 -> UTF-8の変換限定)
 */
Iconv::Iconv( const Encoding to, const Encoding from, const bool broken_sjis_be_utf8 )
    : m_enc_to( to )
    , m_enc_from( from )
    , m_broken_sjis_be_utf8{ broken_sjis_be_utf8 }
{
    const char* from_str = MISC::encoding_to_iconv_cstr( ( from == Encoding::unknown ) ? to : from );
    const char* to_str = MISC::encoding_to_iconv_cstr( ( to == Encoding::unknown ) ? from : to );

#ifdef _DEBUG
    std::cout << "Iconv::Iconv coding = " << from_str << " to " << to_str << std::endl;
#endif

    errno = 0;
    m_cd = g_iconv_open( to_str, from_str );

    if( m_cd == reinterpret_cast<GIConv>(-1) ) open_by_alternative_names( to_str, from_str );
}


void Iconv::open_by_alternative_names( const char* to_str, const char* from_str )
{
    // "MS932"で失敗したら"CP932"で試してみる
    if( m_enc_to == Encoding::sjis ) m_cd = g_iconv_open( "CP932", from_str );
    else if( m_enc_from == Encoding::sjis ) m_cd = g_iconv_open( to_str, "CP932" );

    // "EUCJP-MS"で失敗したら"EUCJP"で試してみる
    if( m_cd == reinterpret_cast<GIConv>(-1) && ( errno & EINVAL ) != 0 )
    {
        if( m_enc_to == Encoding::eucjp ) m_cd = g_iconv_open( "EUCJP//TRANSLIT", from_str );
        else if( m_enc_from == Encoding::eucjp )
        {
            const std::string coding_to_translit = std::string( to_str ) + "//TRANSLIT";
            m_cd = g_iconv_open( coding_to_translit.c_str(), "EUCJP" );
        }
    }

    if( m_cd == reinterpret_cast<GIConv>(-1) ){
        std::string msg = "can't open iconv coding = ";
        msg += MISC::encoding_to_iconv_cstr( m_enc_from );
        msg += " to ";
        msg += MISC::encoding_to_iconv_cstr( m_enc_to );
        MISC::ERRMSG( msg );
    }
}


Iconv::~Iconv()
{
#ifdef _DEBUG
    std::cout << "Iconv::~Iconv\n";
#endif

    if( m_cd != reinterpret_cast<GIConv>(-1) ) g_iconv_close( m_cd );
}


/** @brief テキストの文字エンコーディングを変換する
 *
 * @details UTF-8から別のエンコーディングに変換するときは表現できない文字をHTML数値文字参照(10進数表記)に置き換える。
 * @param[in] str_in    変換するテキストへのポインター
 * @param[in] size_in   変換するテキストの長さ
 * @return 変換したテキストへのconst参照
 * @note 返り値は Iconv オブジェクトを破棄、または convert() を再び呼び出すとdangling referenceになる。
 */
const std::string& Iconv::convert( char* str_in, std::size_t size_in )
{
    return convert( str_in, size_in, m_buf );
}


/** @brief テキストの文字エンコーディングを変換する
 *
 * @details UTF-8から別のエンコーディングに変換するときは表現できない文字をHTML数値文字参照(10進数表記)に置き換える。
 * @param[in] str_in    変換するテキストへのポインター
 * @param[in] size_in   変換するテキストの長さ
 * @param[out] out_buf  変換したテキストを格納するバッファ。渡される前に保持していた内容はクリアされる。
 * @return 変換したテキストへの参照 (= `out_buf`)
 */
std::string& Iconv::convert( char* str_in, std::size_t size_in, std::string& out_buf )
{
#ifdef _DEBUG
    std::cout << "Iconv::convert size_in = " << size_in << std::endl;
#endif

    if( m_cd == reinterpret_cast<GIConv>(-1) ) {
        out_buf.clear();
        return out_buf;
    }

    if( const std::size_t size_in_x2 = size_in * 2;
            size_in_x2 >= out_buf.size() ) {
        out_buf.resize( size_in > 0 ? size_in_x2 : 1 );
    }

    char* buf_in_tmp = str_in;
    const char* buf_in_end = str_in + size_in;

    char* buf_out_tmp = out_buf.data();
    const char* buf_out_end = out_buf.data() + out_buf.size();

    const char* pre_check = nullptr; // 前回チェックしたUTF-8の先頭

    const auto grow = [&] {
        const std::size_t used = buf_out_tmp - out_buf.data();
        out_buf.resize( out_buf.size() * 2 );
        buf_out_tmp = out_buf.data() + used;
        buf_out_end = out_buf.data() + out_buf.size();
    };

    // iconv 実行
    do{
        std::size_t byte_left_in = buf_in_end - buf_in_tmp;
        std::size_t byte_left_out = buf_out_end - buf_out_tmp;

        errno = 0;
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

                if( m_enc_from == Encoding::sjis )
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

                // MS932からUTF-8へエンコーディング変換失敗したとき
                if( m_enc_from == Encoding::sjis && m_enc_to == Encoding::utf8
                        && m_broken_sjis_be_utf8 ){

                    // MS932の文字列の中にUTF-8の混在を許容するときの処理
                    char* bpos = buf_in_tmp;
                    const char* epos = buf_in_tmp;

                    // マルチバイト文字列の先頭と終端を調べる
                    while( bpos > str_in && static_cast<unsigned char>( *( bpos - 1 ) ) >= 128 ) --bpos;
                    while( epos < ( buf_in_tmp + byte_left_in ) && static_cast<unsigned char>( *epos ) >= 128 ) ++epos;

                    const std::size_t lng = epos - bpos;

                    // 1byteは不正、また毎回同じ文字列を判定しない
                    if( lng > 1 && bpos != pre_check ){

                        // 一文字ずつUTF-8の文字か確認
                        constexpr std::size_t byte = 0;
                        pre_check = bpos;

                        // マルチバイト文字列がUTF-8なら出力に追加する
                        if( MISC::is_utf8( std::string_view{ bpos, lng }, byte ) ){
#ifdef _DEBUG_ICONV
                            std::string message = "iconv to be utf-8: ";
                            message.append( bpos, lng );
                            MISC::MSG( message );
#endif

                            // MS932が壊れていたところをマークする
                            constexpr std::string_view span_bgn = "<span class=\"BROKEN_SJIS\">";
                            constexpr std::string_view span_end = "</span>";

                            // 出力したマルチバイトUTF-8文字列の先頭を調べる
                            while( buf_out_tmp > out_buf.data() &&
                                static_cast<unsigned char>( *( buf_out_tmp - 1 ) ) >= 128 ) --buf_out_tmp;

                            if( ( buf_out_tmp + lng + span_bgn.size() + span_end.size() ) >= buf_out_end ){
                                grow();
                            }
                            buf_out_tmp += span_bgn.copy( buf_out_tmp, span_bgn.size() );
                            std::memcpy( buf_out_tmp, bpos, lng );
                            buf_out_tmp += lng;
                            buf_out_tmp += span_end.copy( buf_out_tmp, span_end.size() );
                            buf_in_tmp = bpos + lng;

                            continue;
                        }
                    }
                }

                // unicode 文字からの変換失敗
                // 数値文字参照(&#????;)形式にする
                if( m_enc_from == Encoding::utf8 ){

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
                        std::cout << "utf32 = " << MISC::utf32tostr( unich ) << " byte = " << byte << std::endl;
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

                // UTF-8へ変換する場合は U+FFFD REPLACEMENT CHARACTER に置き換える
                if( m_enc_to == Encoding::utf8 ){
                    ++buf_in_tmp;
                    *(buf_out_tmp++) = static_cast<char>( 0xef );
                    *(buf_out_tmp++) = static_cast<char>( 0xbf );
                    *(buf_out_tmp++) = static_cast<char>( 0xbd );
                    continue;
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

    const std::size_t size_out = buf_out_tmp - out_buf.data();
    out_buf.resize( size_out );

#ifdef _DEBUG
    std::cout << "Iconv::convert size_out = " << size_out << std::endl;
#endif
    return out_buf;
}
