// License: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "misccharcode.h"

#include <cstring>


// チェックする最大バイト数
#define CHECK_LIMIT 1024

/*--- 制御文字とASCII -----------------------------------------------*/

// [ 制御文字 ] 0x00〜0x1F 0x7F
#define CTRL_RNAGE( target ) ( target < 0x20 || target == 0x7F )

// [ ASCII ] 0x20〜0x7E
#define ASCII_RANGE( target ) ( (unsigned char)( target - 0x20 ) < 0x5F )

// [ 制御文字とASCII ] 0x00〜0x7F
#define CTRL_AND_ASCII_RANGE( target ) ( (unsigned char)target < 0x80 )


/*---- EUC-JP -------------------------------------------------------*/
//
// [ カナ ]
// 1バイト目 0x8E
#define EUC_CODE_KANA( target ) ( (unsigned char)target == 0x8E )
// 2バイト目 0xA1〜0xDF
#define EUC_RANGE_KANA( target ) ( (unsigned char)( target - 0xA1 ) < 0x3F )
//
// [ 補助漢字 ]
// 1バイト目 0x8F
#define EUC_CODE_SUB_KANJI( target ) ( (unsigned char)target == 0x8F )
//
// [ 漢字 ]
// 1バイト目 0xA1〜0xFE
// 2バイト目 0xA1〜0xFE
#define EUC_RANGE_MULTI( target ) ( (unsigned char)( target - 0xA1 ) < 0x5E )
//
bool MISC::is_euc( const char* input, size_t read_byte )
{
    if( ! input ) return false;

    size_t byte = read_byte;
    const size_t input_length = strlen( input );

    while( byte < input_length && byte < CHECK_LIMIT )
    {
        // 制御文字かアスキー
        if( CTRL_AND_ASCII_RANGE( input[ byte ] ) )
        {
            ++byte;
        }
        // カナ
        else if( EUC_CODE_KANA( input[ byte ] )
            && EUC_RANGE_KANA( input[ byte + 1 ] ) )
        {
            byte += 2;
        }
        // 補助漢字
        else if( EUC_CODE_SUB_KANJI( input[ byte ] )
                  && EUC_RANGE_MULTI( input[ byte + 1 ] )
                  && EUC_RANGE_MULTI( input[ byte + 2 ] ) )
        {
            byte += 3;
        }
        // 漢字
        else if( EUC_RANGE_MULTI( input[ byte ] )
                  && EUC_RANGE_MULTI( input[ byte + 1 ] ) )
        {
            byte += 2;
        }
        // その他
        else
        {
            return false;
        }
    }

    return true;
}


/*---- ISO-2022-JP --------------------------------------------------*/
//
// エスケープシーケンスの開始文字 ESC(0x1B)
#define JIS_ESC_SEQ_START( target ) ( target == 0x1B )
//
bool MISC::is_jis( const char* input, size_t& byte )
{
    if( ! input ) return false;

    const size_t input_length = strlen( input );

    while( byte < input_length && byte < CHECK_LIMIT )
    {
        // ESCが出現したか否かだけで判断
        if( JIS_ESC_SEQ_START( input[ byte ] ) ) return true;
        // JISに該当しないコード 0x80〜
        else if( ! CTRL_AND_ASCII_RANGE( input[ byte ] ) ) return false;

        ++byte;
    }

    // ループが終了していたら制御文字かアスキー
    return false;
}


/*---- Shift_JIS ----------------------------------------------------*/
//
// [ カナ ] 0xA1〜0xDF
#define SJIS_RANGE_KANA( target ) ( (unsigned char)( target - 0xA1 ) < 0x3F )
//
// [ 漢字 ]
// 1バイト目 0x81〜0x9F 0xE0〜0xFC( 0xEF )
//#define SJIS_RANGE_1( target ) ( (unsigned char)( target ^ 0x20 ) - 0xA1 < 0x2F )
#define SJIS_RANGE_1( target ) ( ( (unsigned char)target ^ 0x20 ) - 0xA1 < 0x3C )
// 0x81〜0x9F
#define SJIS_RANGE_1_H( target ) ( (unsigned char)( target - 0x81 ) < 0x1F )
// 0xE0〜0xFC
#define SJIS_RANGE_1_T( target ) ( (unsigned char)( target - 0xE0 ) < 0x1D )
//
// 2バイト目 0x40〜0x7E 0x80〜0xFC
#define SJIS_RANGE_2( target ) ( (unsigned char)( target - 0x40 ) < 0xBD && target != 0x7F )
// 0x40〜0x7E
#define SJIS_RANGE_2_H( target ) ( (unsigned char)( target - 0x40 ) < 0x3F )
// 0x80〜0xFC
#define SJIS_RANGE_2_T( target ) ( (unsigned char)( target - 0x80 ) < 0x7D )
//
bool MISC::is_sjis( const char* input, size_t read_byte )
{
    if( ! input ) return false;

    size_t byte = read_byte;
    const size_t input_length = strlen( input );

    while( byte < input_length && byte < CHECK_LIMIT )
    {
        // 制御文字かアスキー
        if( CTRL_AND_ASCII_RANGE( input[ byte ] ) )
        {
            ++byte;
        }
        // カナ
        else if( SJIS_RANGE_KANA( input[ byte ] ) )
        {
            ++byte;
        }
        // 漢字(MS932)
        else if( SJIS_RANGE_1( input[ byte ] )
                  && SJIS_RANGE_2( input[ byte + 1 ] ) )
        {
            byte += 2;
        }
        // その他
        else
        {
            return false;
        }
    }

    return true;
}


namespace {
/*---- UTF ---------------------------------------------------------*/
//
/// @brief U+0080〜U+10FFFF (2〜4バイト) の1バイト目か調べる。
///
/// 0xC0, 0xC1, 0xF5〜0xFF の使用は禁止されている。(RFC3629)
inline bool utf8_head_multi_byte( std::uint8_t x ) { return static_cast<std::uint8_t>( x - 0xC2 ) <= ( 0xF4 - 0xC2 ); }

/// U+0080〜U+07FF (2バイト) の1バイト目か調べる。先頭ビットは 110, 範囲は 0xC2〜0xDF
inline bool utf8_head_bytes2( std::uint8_t x ) { return static_cast<std::uint8_t>( x - 0xC2 ) <= ( 0xDF - 0xC2 ); }

/// U+0800〜U+FFFF (3バイト) の1バイト目か調べる。先頭ビットは 1110, 範囲は 0xE0〜0xEF
inline bool utf8_head_bytes3( std::uint8_t x ) { return ( x & 0xF0 ) == 0xE0; }

/// U+10000〜U+10FFFF (4バイト) の1バイト目か調べる。先頭ビットは 11110, 範囲は 0xF0〜0xF4
inline bool utf8_head_bytes4( std::uint8_t x ) { return static_cast<std::uint8_t>( x - 0xF0 ) <= ( 0xF4 - 0xF0 ); }

/// U+0080〜 (2〜4バイト) の2バイト目以降か簡易的に調べる。先頭ビットは 10, 範囲は 0x80～0xBF
inline bool utf8_following_byte( std::uint8_t x ) { return ( x & 0xC0 ) == 0x80; }

} // namespace

bool MISC::is_utf8( const char* input, size_t read_byte )
{
    if( ! input ) return false;

    bool valid = true;

    size_t byte = read_byte;
    const size_t input_length = strlen( input );

    while( byte < input_length && byte < CHECK_LIMIT )
    {
        // 制御文字かアスキー
        if( CTRL_AND_ASCII_RANGE( input[ byte ] ) )
        {
            ++byte;
            continue;
        }
        // UTF-8の1バイト目の範囲ではない
        else if( ! utf8_head_multi_byte( input[ byte ] ) )
        {
            return false;
        }

        int byte_count = 1;

        // 4,3,2バイト
        if( utf8_head_bytes4( input[ byte ] ) ) byte_count = 4;
        else if( utf8_head_bytes3( input[ byte ] ) ) byte_count = 3;
        else if( utf8_head_bytes2( input[ byte ] ) ) byte_count = 2;

        ++byte;

        // 2バイト目以降
        while( byte_count > 1 )
        {
            if( utf8_following_byte( input[ byte ] ) )
            {
                ++byte;
            }
            else
            {
                valid = false;
                break;
            }
            --byte_count;
        }
    }

    return valid;
}


//
// 日本語文字コードの判定
//
// 各コードの判定でtrueの間は文字数分繰り返されるので
// 速度の求められる繰り返し処理などで使わない事
//
int MISC::judge_char_code( const std::string& str )
{
    int code = CHARCODE_UNKNOWN;

    if( str.empty() ) return code;

    size_t read_byte = 0;

    // JISの判定
    if( is_jis( str.c_str(), read_byte ) ) code = CHARCODE_JIS;
    // JISの判定で最後まで進んでいたら制御文字かアスキー
    else if( read_byte == str.length() ) code = CHARCODE_ASCII;
    // is_jis()でASCII範囲外のバイトが現れた箇所から判定する
    // UTF-8の範囲
    else if( is_utf8( str.c_str(), read_byte ) ) code = CHARCODE_UTF;
    // EUC-JPの範囲
    else if( is_euc( str.c_str(), read_byte ) ) code = CHARCODE_EUC_JP;
    // Shift_JISの範囲
    else if( is_sjis( str.c_str(), read_byte ) ) code = CHARCODE_SJIS;

    return code;
}


/** @brief utf-8文字のbyte数を返す
 *
 * @param [in] utf8str  入力文字 (UTF-8)
 * @return 文字の長さ(バイト)が ASCII なら 1, UTF-8 なら 2 or 3 or 4, NULまたは不正な文字なら 0 を返す。
 */
int MISC::utf8bytes( const char* utf8str )
{
    int byte = 0;

    if( utf8str && *utf8str != '\0' ){
        const auto ch = static_cast<unsigned char>( *utf8str );
        if( ch <= 0x7F ) byte = 1;
        else if( utf8_head_bytes3( ch ) ) byte = 3;
        else if( utf8_head_bytes4( ch ) ) byte = 4;
        else if( utf8_head_bytes2( ch ) ) byte = 2;
#ifdef _DEBUG
        else { // 不正なUTF8
            std::cout << "MISC::utf8bytes : invalid 1st byte: char = "
                      << static_cast<unsigned int>( ch ) << std::endl;
        }
#endif
    }

    for( int i = 1; i < byte; ++i ){
        if( ! utf8_following_byte( utf8str[ i ] ) ){
#ifdef _DEBUG
            // 不正なUTF8
            std::cout << "MISC::utf8bytes : invalid code: char = " << static_cast<unsigned int>( utf8str[0] );
            std::cout << ", " << static_cast<unsigned int>( utf8str[1] );
            if( byte > 2 ) std::cout << ", " << static_cast<unsigned int>( utf8str[2] );
            if( byte > 3 ) std::cout << ", " << static_cast<unsigned int>( utf8str[3] );
            std::cout << std::endl;
#endif
            byte = 0;
            break;
        }
    }

    return byte;
}


/** @brief UTF-8 バイト列から UTF-32 コードポイント に変換
 *
 * @param[in] utf8str 入力文字 (UTF-8)
 * @param[out] byte 長さ(バイト数). utf8str が ASCII なら 1, UTF-8 なら 2 or 3 or 4, NULまたは不正な文字なら 0 を返す
 * @return Unicode コードポイント
 */
char32_t MISC::utf8toutf32( const char* utf8str, int& byte )
{
    char32_t unich = 0;
    byte = MISC::utf8bytes( utf8str );

    switch( byte ){
    case 1:
        unich = utf8str[0];
        break;

    case 2:
        unich = utf8str[0] & 0x1F;
        unich = ( unich << 6 ) + ( utf8str[1] & 0x3F );
        break;

    case 3:
        unich = utf8str[0] & 0x0F;
        unich = ( unich << 6 ) + ( utf8str[1] & 0x3F );
        unich = ( unich << 6 ) + ( utf8str[2] & 0x3F );
        break;

    case 4:
        unich = utf8str[0] & 0x07;
        unich = ( unich << 6 ) + ( utf8str[1] & 0x3F );
        unich = ( unich << 6 ) + ( utf8str[2] & 0x3F );
        unich = ( unich << 6 ) + ( utf8str[3] & 0x3F );
        break;

    default:
        break;
    }

    return unich;
}
