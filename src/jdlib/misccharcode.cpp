// License: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "misccharcode.h"


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
bool MISC::is_euc( const char* input, size_t& read_byte )
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
bool MISC::is_sjis( const char* input, size_t& read_byte )
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


/*---- UTF ---------------------------------------------------------*/
//
// 0xC0・0xC1はセキュリティ上の問題で使用が禁止されている
//
// [ 1バイト目の範囲 ] 0xC2〜0xFD [ RFC2279(破棄) ]
// [ 1バイト目の範囲 ] 0xC2〜0xF4 [ RFC6329 ]
#define UTF_RANGE_1( target ) ( (unsigned char)( target - 0xC2 ) < 0x33 )
//
// [ 1バイト目 (2バイト文字) ] 先頭2ビットが1
#define UTF_FLAG_2( target ) ( ( target & 0xC0 ) == 0xC0 )
//
// [ 1バイト目 (3バイト文字) ] 先頭3ビットが1
#define UTF_FLAG_3( target ) ( ( target & 0xE0 ) == 0xE0 )
//
// [ 1バイト目 (4バイト文字) ] 先頭4ビットが1
#define UTF_FLAG_4( target ) ( ( target & 0xF0 ) == 0xF0 )
//
// [ 2バイト目以降 ] 0x80〜0xBF
#define UTF_RANGE_MULTI_BYTE( target ) ( (unsigned char)( target - 0x80 ) < 0x40 )
//
bool MISC::is_utf( const char* input, size_t& read_byte )
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
        else if( ! UTF_RANGE_1( input[ byte ] ) )
        {
            byte = 0;
            return false;
        }

        int byte_count = 1;

        // 4,3,2バイト
        if( UTF_FLAG_4( input[ byte ] ) ) byte_count = 4;
        else if( UTF_FLAG_3( input[ byte ] ) ) byte_count = 3;
        else if( UTF_FLAG_2( input[ byte ] ) ) byte_count = 2;

        ++byte;

        // 2バイト目以降
        while( byte_count > 1 )
        {
            if( UTF_RANGE_MULTI_BYTE( input[ byte ] ) )
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
    // UTF-8の範囲
    else if( is_utf( str.c_str(), read_byte ) ) code = CHARCODE_UTF;
    // EUC-JPの範囲
    else if( is_euc( str.c_str(), read_byte ) ) code = CHARCODE_EUC_JP;
    // Shift_JISの範囲
    else if( is_sjis( str.c_str(), read_byte ) ) code = CHARCODE_SJIS;

    return code;
}

