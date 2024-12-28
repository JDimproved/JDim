// License: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "misccharcode.h"

#include "jdiconv.h"

#include <glib.h>

#include <cstdint>
#include <cstring>
#include <iterator>


// チェックする最大バイト数
#define CHECK_LIMIT 1024


namespace MISC::charset {
constexpr const char* iconv[] = { "ISO-8859-1", "ASCII", "EUCJP-MS", "ISO-2022-JP", "MS932", "UTF-8" };
constexpr const char* names[] = { "ISO-8859-1", "ASCII", "EUC-JP", "ISO-2022-JP", "Shift_JIS", "UTF-8" };
static_assert( std::size( iconv ) == std::size( names ) );

template<typename T>
const char* encoding_to_impl( Encoding enc, T&& strings )
{
    if( enc > Encoding::utf8 || enc < Encoding::unknown ) enc = Encoding::unknown;
    return strings[ static_cast<int>( enc ) ];
}
} // namespace MISC::charset


/** @brief `Encoding` からUIに表示したり情報ファイルに保存するのに使う文字エンコーディング名を取得する
 *
 * @param[in] encoding 文字エンコーディング
 * @return encodingに対応するヌル終端文字列。
 * `Encoding::unknown` のとき、または不正な値なら `"ISO-8859-1"` (Latin1) を返す。
 */
const char* MISC::encoding_to_cstr( const Encoding encoding )
{
    return charset::encoding_to_impl( encoding, charset::names );
}


/** @brief `Encoding` から JDLIB::Iconv クラスに渡す文字エンコーディング名を取得する
 *
 * @param[in] encoding 文字エンコーディング
 * @return encodingに対応するヌル終端文字列。
 * `Encoding::unknown` のとき、または不正な値なら `"ISO-8859-1"` (Latin1) を返す。
 */
const char* MISC::encoding_to_iconv_cstr( const Encoding encoding )
{
    return charset::encoding_to_impl( encoding, charset::iconv );
}


/** @brief 文字エンコーディングを表す文字列から`Encoding`を取得する
 *
 * @param[in] encoding 文字エンコーディング名 (not null)
 * @return 文字列に対応する列挙型。見つからなければ `Encoding::unknown` を返す。
 */
Encoding MISC::encoding_from_sv( std::string_view encoding )
{
    assert( encoding.data() );

    for( std::size_t i = std::size( charset::names ) - 1; i > 0; --i ){
        if( encoding == charset::names[i] ) return static_cast<Encoding>( i );
    }
    // 互換性のため JDLIB::Iconv クラスで使うエンコーディング名と一致するかチェックする
    if( encoding == charset::iconv[ static_cast<int>( Encoding::sjis ) ] ) return Encoding::sjis;
    if( encoding == charset::iconv[ static_cast<int>( Encoding::eucjp ) ] ) return Encoding::eucjp;
    return Encoding::unknown;
}


/** @brief Webで使われる文字エンコーディングのラベルから`Encoding`を取得する
 *
 * @details 入力文字列から繋ぎの記号(`-`, `_`)を取り除き大文字小文字を無視して比較する。
 * @param[in] charset 文字エンコーディングのラベル
 * @return 文字列に対応する列挙型。見つからなければ `Encoding::unknown` を返す。
 */
Encoding MISC::encoding_from_web_charset( std::string_view charset )
{
    std::string casefold;
    casefold.reserve( charset.size() );

    for( const char c : charset ) {
        if( c == '_' || c == '-' ) continue;
        casefold.push_back( g_ascii_toupper( c ) );
    }

    if( casefold == "UTF8" ) return Encoding::utf8;
    if( casefold == "SHIFTJIS"
        || casefold == "SJIS"
        || casefold == "XSJIS"
        || casefold == "WINDOWS31J" ) return Encoding::sjis;
    if( casefold == "EUCJP"
        || casefold == "XEUCJP" ) return Encoding::eucjp;
    // その他は使わないため unknown を返す
    return Encoding::unknown;
}


/*--- 制御文字とASCII -----------------------------------------------*/

// [ 制御文字 ] 0x00〜0x1F 0x7F
#define CTRL_RNAGE( target ) ( target < 0x20 || target == 0x7F )

// [ ASCII ] 0x20〜0x7E
#define ASCII_RANGE( target ) ( static_cast<unsigned char>( target - 0x20 ) < 0x5F )

// [ 制御文字とASCII ] 0x00〜0x7F
#define CTRL_AND_ASCII_RANGE( target ) ( static_cast<unsigned char>(target) < 0x80 )


/*---- EUC-JP -------------------------------------------------------*/
//
// [ カナ ]
// 1バイト目 0x8E
#define EUC_CODE_KANA( target ) ( static_cast<unsigned char>(target) == 0x8E )
// 2バイト目 0xA1〜0xDF
#define EUC_RANGE_KANA( target ) ( static_cast<unsigned char>( target - 0xA1 ) < 0x3F )
//
// [ 補助漢字 ]
// 1バイト目 0x8F
#define EUC_CODE_SUB_KANJI( target ) ( static_cast<unsigned char>(target) == 0x8F )
//
// [ 漢字 ]
// 1バイト目 0xA1〜0xFE
// 2バイト目 0xA1〜0xFE
#define EUC_RANGE_MULTI( target ) ( static_cast<unsigned char>( target - 0xA1 ) < 0x5E )
//
/** @brief 文字列のエンコーディングがEUC-JPかチェックする
 *
 * @param[in] input 入力文字列
 * @param[in] read_byte チェックを開始する位置
 * @return EUC-JPのシーケンスに一致していればtrue
 */
bool MISC::is_eucjp( std::string_view input, std::size_t read_byte )
{
    if( input.empty() ) return true;

    size_t byte = read_byte;

    while( byte < input.size() && byte < CHECK_LIMIT )
    {
        // 制御文字かアスキー
        if( CTRL_AND_ASCII_RANGE( input[ byte ] ) )
        {
            ++byte;
        }
        // std::string_view はヌル終端の保証がないため長さをチェックする
        // カナ
        else if( byte + 1 < input.size()
                 && EUC_CODE_KANA( input[ byte ] )
                 && EUC_RANGE_KANA( input[ byte + 1 ] ) )
        {
            byte += 2;
        }
        // 補助漢字
        else if( byte + 2 < input.size()
                 && EUC_CODE_SUB_KANJI( input[ byte ] )
                 && EUC_RANGE_MULTI( input[ byte + 1 ] )
                 && EUC_RANGE_MULTI( input[ byte + 2 ] ) )
        {
            byte += 3;
        }
        // 漢字
        else if( byte + 1 < input.size()
                 && EUC_RANGE_MULTI( input[ byte ] )
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
/** @brief 文字列のエンコーディングがISO-2022-JPか簡易判定する
 *
 * エスケープシーケンス(ESC = \\x1B)の有無と該当しないバイトが含まれるかチェックする。
 * 呼び出し元の処理のため空文字列に対する返り値は他の`is_*`関数と逆(false)になっている。
 * @param[in]     input     入力
 * @param[in,out] read_byte チェックを開始する位置、チェックを打ち切った位置を返す
 * @return
 *   - ESCを見つけたらtrue
 *   - 0x80以上を見つけたらfalse
 *   - 空文字列またはASCIIのみならfalse
 */
bool MISC::is_jis( std::string_view input, std::size_t& read_byte )
{
    if( input.empty() ) return false;

    while( read_byte < input.size() && read_byte < CHECK_LIMIT )
    {
        // ESCが出現したか否かだけで判断
        if( JIS_ESC_SEQ_START( input[ read_byte ] ) ) return true;
        // JISに該当しないコード 0x80〜
        else if( ! CTRL_AND_ASCII_RANGE( input[ read_byte ] ) ) return false;

        ++read_byte;
    }

    // ループが終了していたら制御文字かアスキー
    return false;
}


/*---- Shift_JIS ----------------------------------------------------*/
//
// [ カナ ] 0xA1〜0xDF
#define SJIS_RANGE_KANA( target ) ( static_cast<unsigned char>( target - 0xA1 ) < 0x3F )
//
// [ 漢字 ]
// 1バイト目 0x81〜0x9F 0xE0〜0xFC( 0xEF )
//#define SJIS_RANGE_1( target ) ( (unsigned char)( target ^ 0x20 ) - 0xA1 < 0x2F )
#define SJIS_RANGE_1( target ) ( ( static_cast<unsigned char>(target) ^ 0x20 ) - 0xA1 < 0x3C )
// 0x81〜0x9F
#define SJIS_RANGE_1_H( target ) ( static_cast<unsigned char>( target - 0x81 ) < 0x1F )
// 0xE0〜0xFC
#define SJIS_RANGE_1_T( target ) ( static_cast<unsigned char>( target - 0xE0 ) < 0x1D )
//
// 2バイト目 0x40〜0x7E 0x80〜0xFC
#define SJIS_RANGE_2( target ) ( static_cast<unsigned char>( target - 0x40 ) < 0xBD && target != 0x7F )
// 0x40〜0x7E
#define SJIS_RANGE_2_H( target ) ( static_cast<unsigned char>( target - 0x40 ) < 0x3F )
// 0x80〜0xFC
#define SJIS_RANGE_2_T( target ) ( static_cast<unsigned char>( target - 0x80 ) < 0x7D )
//
/** @brief 文字列のエンコーディングがShift_JISか簡易チェックする
 *
 * @param[in] input 入力文字列
 * @param[in] read_byte チェックを開始する位置
 * @return Shitt_JISのシーケンスに一致していればtrue
 */
bool MISC::is_sjis( std::string_view input, std::size_t read_byte )
{
    if( input.empty() ) return true;

    size_t byte = read_byte;

    while( byte < input.size() && byte < CHECK_LIMIT )
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
        // std::string_view はヌル終端の保証がないため長さをチェックする
        // 漢字(MS932)
        else if( byte + 1 < input.size()
                 && SJIS_RANGE_1( input[ byte ] )
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

/** @brief 文字列のエンコーディングがUTF-8か簡易判定する
 *
 * 2バイト目以降の部分は大まかな判定で厳密ではない。
 * バイト列のチェックだけなのでUTF-8表現のサロゲートペアや非文字、未割り当てのコードポイントでもtrueを返す。
 * @param[in] input 入力
 * @param[in] read_byte チェックを開始する位置
 * @return UTF-8のシーケンスに一致していればtrue
 */
bool MISC::is_utf8( std::string_view input, std::size_t read_byte )
{
    if( input.empty() ) return true;

    size_t byte = read_byte;

    while( byte < input.size() && byte < CHECK_LIMIT )
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

        // std::string_view はヌル終端の保証がないため長さをチェックする
        if( byte + byte_count > input.size() ) return false;

        ++byte;

        // 2バイト目以降
        for( ; byte_count > 1; --byte_count )
        {
            if( ! utf8_following_byte( input[ byte ] ) )
            {
                return false;
            }
            ++byte;
        }
    }

    return true;
}


/** @brief 日本語文字エンコーディングの検出
 *
 * @note 各エンコーディングの判定でtrueの間は文字数分繰り返されるので
 * 速度の求められる繰り返し処理などで使わない事
 * @param[in] str エンコーディングを検出するテキスト
 * @return 検出結果
 */
Encoding MISC::detect_encoding( std::string_view str )
{
    Encoding code = Encoding::unknown;

    if( str.empty() ) return code;

    size_t read_byte = 0;

    // JISの判定
    if( is_jis( str, read_byte ) ) code = Encoding::jis;
    // JISの判定で最後まで進んでいたら制御文字かアスキー
    else if( read_byte == str.length() ) code = Encoding::ascii;
    // is_jis()でASCII範囲外のバイトが現れた箇所から判定する
    // UTF-8の範囲
    else if( is_utf8( str, read_byte ) ) code = Encoding::utf8;
    // EUC-JPの範囲
    else if( is_eucjp( str, read_byte ) ) code = Encoding::eucjp;
    // Shift_JISの範囲
    else if( is_sjis( str, read_byte ) ) code = Encoding::sjis;

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


/**
 * @brief UTF-32 から UTF-8 に変換する
 *
 * @param[in]  uch     変換するコードポイント。有効範囲は [0, 0x10FFFF]
 * @param[out] utf8str 変換後の文字。渡された文字列は末尾にヌル文字を追加する
 * @return 変換したUTF-8のバイト数。入力が有効範囲外のときは 0 が返る
 */
int MISC::utf32toutf8( const char32_t uch,  char* utf8str )
{
    int byte = 0;
    if( ! utf8str ) return byte;

    if( uch <= 0x7F ){ // ascii
        byte = 1;
        utf8str[0] = static_cast<char>( uch );
    }
    else if( uch <= 0x07FF ){
        byte = 2;
        utf8str[0] = static_cast<char>( 0xC0 + ( uch >> 6 ) );
        utf8str[1] = static_cast<char>( 0x80 + ( uch & 0x3F ) );
    }
    else if( uch <= 0xFFFF ){
        byte = 3;
        utf8str[0] = static_cast<char>( 0xE0 + ( uch >> 12 ) );
        utf8str[1] = static_cast<char>( 0x80 + ( ( uch >> 6 ) & 0x3F ) );
        utf8str[2] = static_cast<char>( 0x80 + ( uch & 0x3F ) );
    }
    else if( uch <= 0x10FFFF ){
        byte = 4;
        utf8str[0] = static_cast<char>( 0xF0 + ( uch >> 18 ) );
        utf8str[1] = static_cast<char>( 0x80 + ( ( uch >> 12 ) & 0x3F ) );
        utf8str[2] = static_cast<char>( 0x80 + ( ( uch >> 6 ) & 0x3F ) );
        utf8str[3] = static_cast<char>( 0x80 + ( uch & 0x3F ) );
    }
#ifdef _DEBUG
    else{
        std::cout << "MISC::utf32toutf8 : invalid uch = " << MISC::utf32tostr( uch ) << std::endl;
    }
#endif

    utf8str[byte] = 0;
    return byte;
}


/** @brief UTF-32 の値から Unicode のコードポイントを表す文字列("U+XXXX")を返す
 *
 * @param[in] uch Unicodeコードポイント
 * @return "U+" プレフィックスを付けた 4〜6 桁の16進数値 (A〜F は大文字)。
 * U+10000 未満の値は 4 桁にゼロ埋めする。
 */
std::string MISC::utf32tostr( const char32_t uch )
{
    std::string str( 11u, '\0' );
    const auto length = std::snprintf( str.data(), 11u, "U+%04X", uch );
    str.resize( length );
    return str;
}


/** @brief 特定のUnicodeブロックかコードポイントを調べる
 *
 * @param[in] unich Unicodeコードポイント
 * @return MISC::UnicodeBlock 列挙体
 */
MISC::UnicodeBlock MISC::get_unicodeblock( const char32_t unich )
{
    if( unich <= 0x007F ) return UnicodeBlock::BasicLatin;
    if( unich >= 0x3040 && unich <= 0x309F ) return UnicodeBlock::Hira;
    if( unich >= 0x30A0 && unich <= 0x30FF ) return UnicodeBlock::Kata;

    return UnicodeBlock::Other;
}


/** @brief WAVE DASH(U+301C)などのWindows系UTF-8文字をUnix系文字と相互変換
 *
 * @param[in] str  変換する文字列
 * @param[in] mode 変換モード
 * @return 変換した結果
 */
std::string MISC::utf8_fix_wavedash( const std::string& str, const MISC::WaveDashFix mode )
{
    // WAVE DASH 問題
    constexpr std::size_t size = 4;
    constexpr const std::string_view Win[size] = {
        "\xEF\xBD\x9E", // U+FF5E FULLWIDTH TILDE
        "\xE2\x80\x95", // U+2015 HORIZONTAL BAR
        "\xE2\x88\xA5", // U+2225 PARALLEL TO
        "\xEF\xBC\x8D", // U+FF0D FULLWIDTH HYPHEN-MINUS
    };
    constexpr const std::string_view Unix[size] = {
        "\xE3\x80\x9C", // U+301C WAVE DASH
        "\xE2\x80\x94", // U+2014 EM DASH
        "\xE2\x80\x96", // U+2016 DOUBLE VERTICAL LINE
        "\xE2\x88\x92", // U+2012 MINUS SIGN
    };

    const char* head_bytes;
    const std::string_view* from_list;
    const std::string_view* to_list;
    if( mode == WaveDashFix::WinToUnix ) {
        head_bytes = "\xE2\xEF";
        from_list = Win;
        to_list = Unix;
    }
    else {
        head_bytes = "\xE2\xE3";
        from_list = Unix;
        to_list = Win;
    }

    std::string result = str;

    for( std::size_t i = 0; i < result.size(); ++i ) {
        i = result.find_first_of( head_bytes, i ); // UTF-8の先頭バイトを探す
        if( i == result.npos ) break;

        for( std::size_t s = 0; s < size; ++s ) {
            // 後続のバイト列が一致するかチェック
            if( result.compare( i + 1, 2, from_list[s].data() + 1 ) != 0 ) continue;

            to_list[s].copy( result.data() + i, 3 );
            i += 2;
            break;
        }
    }

    return result;
}


/** @brief 入力の文字エンコーディングを from から to に変換
 *
 * @details 遅いので連続的な処理が必要な時は使わないこと
 * @param[in] str  変換するテキスト
 * @param[in] to   変換先の文字エンコーディング
 * @param[in] from str の文字エンコーディング
 * @return 変換した結果
 */
std::string MISC::Iconv( const std::string& str, const Encoding to, const Encoding from )
{
    if( from == to ) return str;

    JDLIB::Iconv icv( to, from );
    std::string tmp_str{ str };
    std::string encoded;
    icv.convert( tmp_str.data(), tmp_str.size(), encoded );
    return encoded;
}
