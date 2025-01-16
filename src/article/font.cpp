// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "font.h"

#include "fontid.h"
#include "config/globalconf.h"

#include <cstdlib>
#include <cstring>


struct WIDTH_DATA
{
    // 半角モードの時の幅
    unsigned int *width;

    // 全角モードの時の幅
    unsigned int width_wide;
};


static WIDTH_DATA* width_of_char[ FONT_NUM ];
static bool strict_of_char = false;


// UnicodeのPlane 0 基本多言語面(BMP)からPlane 3 第三漢字面(TIP)までキャッシュを持つ。
// 現状のメモリ消費を抑えるためPlane 4からPlane 13は将来割り当てられたときにキャッシュ対応する。
constexpr char32_t kMaxCacheCodePoint{ 0x40000 };


//
// 初期化
//
void ARTICLE::init_font()
{
    // スレビューで文字幅の近似を厳密にするか
    strict_of_char = CONFIG::get_strict_char_width();

    for( WIDTH_DATA*& char_data : width_of_char ) {

        if( char_data ) {

            for( char32_t j = 0; j < kMaxCacheCodePoint; ++j ){

                if( char_data[ j ].width ) delete[] char_data[ j ].width;
            }
            delete[] char_data;
            char_data = nullptr;
        }
    }
}



/** @brief 登録された文字の幅を返す関数
 *
 * @param[in]  code      入力文字 (コードポイント)
 * @param[in]  pre_char  ひとつ前の文字 ( 前の文字が全角の場合は 0 )
 * @param[out] width     半角モードでの幅
 * @param[out] width_wide  全角モードでの幅
 * @param[in]  mode      fontid.h で定義されているフォントのID
 * @return     登録されていればtrue
 */
bool ARTICLE::get_width_of_char( const char32_t code, const char pre_char, int& width, int& width_wide, const int mode )
{
    width = 0;
    width_wide = 0;

    if( ! width_of_char[ mode ] ){
        width_of_char[ mode ] = new WIDTH_DATA[ kMaxCacheCodePoint ]{};

        // 合成文字や制御文字や異字体セレクタの初期化
        constexpr const int blocks[][2] = {
            { 0x0300, 0x036F }, // Combining Diacritical Marks
            { 0x180B, 0x180D }, // Mongolian Free Variation Selector
            { 0x200B, 0x200F }, // ZWSP,ZWNJ,ZWJ,LRM,RLM
            { 0x202A, 0x202E }, // LRE,RLE,PDF,LRO,RLO
            { 0x20D0, 0x20FF }, // Combining Diacritical Marks for Symbols
            { 0x3099, 0x309A }, // COMBINING KATAKANA-HIRAGANA (SEMI-)VOICED SOUND MARK
            { 0xFE00, 0xFE0F }, // VS1-VS16
        };
        for( const auto& [start, end] : blocks ) {
            for( int i = start; i <= end; ++i ) {
                width_of_char[ mode ][ i ].width_wide = -1;
            }
        }
        width_of_char[ mode ][ 0xFEFF ].width_wide = -1; // ZERO WIDTH NO-BREAK SPACE
    }

    if( code > 0 && code < kMaxCacheCodePoint ){

        // 全角モードの幅
        width_wide = width_of_char[ mode ][ code ].width_wide;

        // 半角モードの幅
        width = width_wide;

        // 厳密に求める場合
        if( code < 128 && strict_of_char ){

            if( ! width_of_char[ mode ][ code ].width ){
                width_of_char[ mode ][ code ].width = new unsigned int[ 128 ]{};
            }

            // ここの比較はASCIIの範囲 [0, 127] を想定しているが、
            // char型はCPUアーキテクチャによって符号の有無が変わるので、
            // 明示的にunsigned char型へ変換して比較する。
            const auto pre_char_num = static_cast<unsigned char>(pre_char);
            if( pre_char_num < 128 ) width = width_of_char[ mode ][ code ].width[ pre_char_num ];
        }
    }
    // キャッシュ範囲外のコードポイントは個別に幅を設定する
    // Plane 14 追加特殊用途面(SSP)
    else if( 0xE0001 == code || ( 0xE0020 <= code && code <= 0xE007F ) // タグ文字
             || ( 0xE0100 <= code && code <= 0xE01EF )                 // 異字体セレクタ
    ) {
        width = width_wide = 0;
        return true;
    }

    if( width == -1 ){ // フォント幅の取得に失敗した場合
        width = width_wide = 0;
        return true;
    }
    else if( width && width_wide ) return true;

    return false;
}



/** @brief 文字幅を登録する関数
 *
 * width == -1 はフォント幅の取得に失敗した場合
 * @param[in] code      入力文字 (コードポイント)
 * @param[in] pre_char  ひとつ前の文字 ( 前の文字が全角の場合は 0 )
 * @param[in] width     半角モードでの幅
 * @param[in] width_wide  全角モードでの幅
 * @param[in] mode      fontid.h で定義されているフォントのID
 */
void ARTICLE::set_width_of_char( const char32_t code, const char pre_char, const int width, const int width_wide,
                                 const int mode )
{
    if( code >= kMaxCacheCodePoint ) return;

    // 半角モードの幅を厳密に求める場合
    if( code < 128 && strict_of_char ){

        // ここの比較はASCIIの範囲 [0, 127] を想定しているが、
        // char型はCPUアーキテクチャによって符号の有無が変わるので、
        // 明示的にunsigned char型へ変換して比較する。
        const auto pre_char_num = static_cast<unsigned char>(pre_char);
        if( pre_char_num < 128 ) width_of_char[ mode ][ code ].width[ pre_char_num ] = width;
    }

    // 全角モードの幅
    width_of_char[ mode ][ code ].width_wide = width_wide;
}
