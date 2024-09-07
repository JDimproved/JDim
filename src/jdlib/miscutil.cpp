// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "miscutil.h"

#include "hkana.h"
#include "jdiconv.h"
#include "jdregex.h"
#include "misccharcode.h"
#include "miscmsg.h"

#include "dbtree/spchar_decoder.h"
#include "dbtree/node.h"

#include "cssmanager.h"

#include <glib.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <sstream>


//
// str を "\n" ごとに区切ってlistにして出力
//
std::list< std::string > MISC::get_lines( const std::string& str ){
        
    std::list< std::string > lines;
    size_t i = 0, i2 = 0;
    while ( ( i2 = str.find( '\n', i ) ) != std::string::npos ){
        std::size_t r = 0;
        if( (i2 >= 1) && (str[ i2 - 1 ] == '\r') ) r = 1;
        if( i2 - i > 0 ){
            lines.push_back( str.substr( i, i2 - i - r ) );
        }
        i = i2 + 1;
    }

    // 最後の行
    if( i != str.length() +1 ) lines.push_back( str.substr( i ) );
    
    return lines;
}


//
// emacs lisp のリスト型を要素ごとにlistにして出力
//
std::list< std::string > MISC::get_elisp_lists( const std::string& str )
{
#ifdef _DEBUG
    std::cout << "MISC::get_elisp_lists\n";
#endif 
   
    std::list< std::string > lists;
    std::string str2 = utf8_trim( str );
    const char* data = str2.c_str();
    if( data[ 0 ] != '(' ) return lists;

    std::size_t pos = 1;

    while( data[ pos ] != '\0' ){

        // 空白削除
        while( data[ pos ] == ' ' ) ++pos;
        if( data[ pos ] == '\0' ) break;

        std::size_t length = 1;

        // (が現れた
        if( data[ pos ] == '(' ){

            int count = 1;
            while( data[ pos + length ] != '\0' ){
                if( data[ pos + length ] == ')' ) --count;
                else if( data[ pos + length ] == '(' ) ++count;
                ++length;
                if( ! count ) break;
            }
        }

        // 改行 or データが壊れてる
        else if( data[ pos ] == '\n' || data[ pos ] == ')' ){
            ++pos;
            continue;
        }

        // 通常データ
        else{
            //空白 or ) を探す
            while( data[ pos + length ] != ' ' && data[ pos + length ] != ')' && data[ pos + length ] != '\0' ) ++length;
        }

#ifdef _DEBUG
        std::cout << "pos = " << pos << " length = " << length << std::endl;
#endif

        lists.push_back( str2.substr( pos, length ) );
        pos += length;
    }

#ifdef _DEBUG
    for( const std::string& s : lists ) std::cout << "[" << s << "]" << std::endl;
#endif
    
    return lists;
}

//
// strを空白または "" 単位で区切って list で出力
//
std::list< std::string > MISC::split_line( const std::string& str )
{
    constexpr const char* str_space = "\u3000"; // "\xE3\x80\x80" 全角スペース
    constexpr size_t lng_space = 3;

    std::list< std::string > list_str;

    size_t i = 0, lng = str.length();
    for(;;){

        // 空白を取る
        while( 1 ){

            // 半角
            if( str[ i ] == ' ' ) ++i;

            // 全角
            else if( str[ i ] == str_space[ 0 ] &&
                     str[ i +1 ] == str_space[ 1 ] &&
                     str[ i +2 ] == str_space[ 2 ] ) i += lng_space;

            else break;
        }

        // " から始まる ( \"は除く )
        bool dquote = false;
        if( (i < 1 || str[ i -1 ] != '\\') && str[ i ] == '\"' ){
            dquote = true;
            ++i;
        }

        // 空白か " を探す
        std::size_t i2 = i;
        size_t lng_tmp = 1;
        while( i2 < lng ){

            // " 発見( \"は除く )
            if( dquote ){
                if( str[ i2 ] == '\"' && str[ i2-1 ] != '\\' ) break;
            }
            else{
                // 半角
                if( str[ i2 ] == ' ' ) break;

                // 全角
                else if( str[ i2 ] == str_space[ 0 ] &&
                         str[ i2 +1 ] == str_space[ 1 ] &&
                         str[ i2 +2 ] == str_space[ 2 ] ){
                    lng_tmp = lng_space;
                    break;
                }
            }

            ++i2;
        }

        if( i2 - i ) list_str.push_back( str.substr( i, i2 - i ) );
        if( i2 >= lng ) break;
        i = i2 + lng_tmp;
    }
    
    return list_str;
}


// strを delimで区切って list で出力
std::list< std::string > MISC::StringTokenizer( const std::string& str, const char delim )
{
    std::list< std::string > list_str;

    size_t i = 0, i2 = 0, lng = str.length();
    for(;;){

        while( i2 < lng && str[ i2++ ] != delim );
        int tmp = ( i2 >= 1 && ( str[ i2-1 ] == delim || str[ i2 -1 ] == '\n' ) ) ? 1 : 0;
        if( i2 - i ) list_str.push_back( str.substr( i, i2 - i - tmp ) );
        if( i2 >= lng ) break;
        i = i2;
    }
    
    return list_str;
}



//
// list_inから空白行を除いてリストを返す
//
std::list< std::string > MISC::remove_nullline_from_list( const std::list< std::string >& list_in )
{
    std::list< std::string > list_ret;
    for( const std::string& s : list_in ) {
        std::string tmp_str = MISC::utf8_trim( s );
        if( ! tmp_str.empty() ) list_ret.push_back( s );
    }

    return list_ret;
}


//
// list_inの各行から前後の空白を除いてリストを返す
//
std::list< std::string > MISC::remove_space_from_list( const std::list< std::string >& list_in )
{
    std::list< std::string > list_ret;
    for( const std::string& s : list_in ) {
        std::string tmp_str = MISC::utf8_trim( s );
        list_ret.push_back( std::move( tmp_str ) );
    }

    return list_ret;
}


//
// list_inからコメント行(#)を除いてリストを返す
//
std::list< std::string > MISC::remove_commentline_from_list( const std::list< std::string >& list_in )
{
    const char commentchr = '#';

    std::list< std::string > list_ret;
    for( const std::string& s : list_in ) {
        std::string tmp_str = MISC::utf8_trim( s );
        if( tmp_str[0] != commentchr ) list_ret.push_back( s );
    }

    return list_ret;
}



//
// 空白と""で区切られた str_in の文字列をリストにして出力
//
// \"は " に置換される
//
// (例)  "aaa" "bbb" "\"ccc\""  → aaa と bbb と "ccc"
//
std::list< std::string > MISC::strtolist( const std::string& str_in )
{
    std::list< std::string > list_ret;

    std::list<std::string> list_tmp = MISC::split_line( str_in );
    for( const std::string& s : list_tmp ) {
        if( ! s.empty() ) list_ret.push_back( MISC::recover_quot( s ) );
    }

    return list_ret;
}



//
// list_in の文字列リストを空白と""で区切ってストリングにして出力
//
// "は \" に置換される
//
// (例)  "aaa" "bbb" "\"ccc\""
//
std::string MISC::listtostr( const std::list< std::string >& list_in )
{
    std::string str_out;
    for( const std::string& s : list_in ) {
        if( ! s.empty() ) str_out.append( " \"" + MISC::replace_quot( s )  + "\"" );
    }

    return str_out;
}



//
// list_in から空文字列を除き suffix でつなげて返す
// 他のプログラミング言語にあるjoin()と動作が異なり返り値の末尾にもsuffixが付く
//
// (例) {"aa", "", "bb", "cc"}, '!' -> "aa!bb!cc!"
//
std::string MISC::concat_with_suffix( const std::list<std::string>& list_in, char suffix )
{
    std::string str_out;
    for( const std::string& s : list_in ) {
        if( s.empty() ) continue;
        str_out.append( s );
        str_out.push_back( suffix );
    }
    return str_out;
}



/** @brief str前後の半角スペース(U+0020)と全角スペース(U+3000)を削除
 *
 * 半角スペースが含まれてないときはトリミングせずstrをそのまま返す。
 * @param[in] str トリミングする文字列
 * @return トリミングした結果
 */
std::string MISC::utf8_trim( std::string_view str )
{
    constexpr std::string_view str_space = "\xE3\x80\x80"; // U+3000 全角スペース
    constexpr std::size_t lng_space = str_space.size();

    if( str.empty() ) return std::string{};
    // TODO: 半角スペースがなくてもトリミングしたほうがよいか検証する
    if( str.find( ' ' ) == std::string_view::npos ) return std::string{ str };

    // 前
    std::size_t i = 0;
    while( i < str.size() ) {

        // 半角
        if( str[ i ] == ' ' ) ++i;

        // 全角
        else if( str.compare( i, lng_space, str_space ) == 0 ) i += lng_space;

        else break;
    }
    if( i >= str.size() ) return std::string{};

    // 後
    std::size_t i2 = str.size() -1;
    while( 1 ){

        // 半角
        if( str[ i2 ] == ' ' ) --i2;

        // 全角
        else if( i2 +1 >= lng_space &&
                str.compare( i2 - lng_space + 1, lng_space, str_space ) == 0 ) i2 -= lng_space;

        else break;
    }

    return std::string{ str.begin() + i, str.begin() + i2 + 1 };
}


/** @brief str前後の改行(\\r, \\n)、タブ(\\t)、スペース(U+0020)を削除
 *
 * @param[in] str トリミングする文字列
 * @return トリミングした結果
 */
std::string MISC::ascii_trim( const std::string& str )
{
    if( str.empty() ) return std::string();

    constexpr std::string_view space_chars = " \n\t\r";
    const auto start = str.find_first_not_of( space_chars );
    if( start == std::string::npos ) return std::string();

    const auto end = str.find_last_not_of( space_chars ) + 1;
    return str.substr( start, end - start );
}



/** @brief strからpatternで示された文字列を除く
 *
 * @param[in] str 処理する文字列
 * @param[in] pattern 取り除く文字列
 * @return 取り除いた結果
 */
std::string MISC::remove_str( std::string_view str, std::string_view pattern )
{
    return MISC::replace_str( str, pattern, "" );
}


/// @brief start 〜 end の範囲をstrから取り除く ( /* コメント */ など )
/**
 * @param[in] str 処理する文字列
 * @param[in] start 取り除く範囲の先頭
 * @param[in] end 取り除く範囲の末尾
 * @return 取り除いた結果。
 */
std::string MISC::remove_str( std::string_view str, std::string_view start, std::string_view end )
{
    std::string str_out{ str };
    if( str_out.empty() || start.empty() || end.empty() ) return str_out;

    size_t l_pos = 0, r_pos = 0;
    const size_t start_length = start.length();
    const size_t end_length = end.length();

    while( ( l_pos = str_out.find( start, l_pos ) ) != std::string::npos &&
            ( r_pos = str_out.find( end, l_pos + start_length ) ) != std::string::npos )
    {
        str_out.erase( l_pos, r_pos - l_pos + end_length );
    }

    return str_out;
}



//
// 正規表現を使ってstr1からqueryで示された文字列を除く
//
std::string MISC::remove_str_regex( const std::string& str1, const std::string& query )
{
    JDLIB::Regex regex;
    const size_t offset = 0;
    const bool icase = false;
    const bool newline = true;
    const bool usemigemo = false;
    const bool wchar = false;
    if( ! regex.exec( query, str1, offset, icase, newline, usemigemo, wchar ) ) return std::string();
    return MISC::remove_str( str1, regex.str( 0 ) );
}


/** @brief front_sep, back_sep に囲まれた文字列を切り出す
 *
 * @param[in] str 処理する文字列
 * @param[in] front_sep 前の区切り
 * @param[in] back_sep 後の区切り
 * @return 切り出した結果。引数が空文字列または区切りが見つからないときは空文字列を返す。
 */
std::string MISC::cut_str( const std::string& str, std::string_view front_sep, std::string_view back_sep )
{
    if( str.empty() || front_sep.empty() || back_sep.empty() ) return std::string{};

    std::size_t i = str.find( front_sep );
    if( i == std::string::npos ) return std::string();
    i += front_sep.size();
    const std::size_t i2 = str.find( back_sep, i );
    if( i2 == std::string::npos ) return std::string();
    
    return str.substr( i, i2 - i );
}


/** @brief pattern を replacement に置き換える
 *
 * @param[in] str 置き換えを実行する文字列
 * @param[in] pattern 置き換える文字列のパターン
 * @param[in] replacement マッチした文字列を置き換える内容
 * @return 置き換えを実行した結果。str や pattern が空文字列のときは str をそのまま返す。
 */
std::string MISC::replace_str( std::string_view str, std::string_view pattern, std::string_view replacement )
{
    if( str.empty() || pattern.empty() ) return std::string( str );

    size_t i, pos = 0;
    if( ( i = str.find( pattern ) ) == std::string_view::npos ) return std::string( str );

    std::string str_out;
    str_out.reserve( str.length() );

    do {
        str_out.append( str, pos, ( i - pos ) );
        str_out.append( replacement );
        pos = i + pattern.length();
    }
    while( ( i = str.find( pattern, pos ) ) != std::string_view::npos );

    str_out.append( str, pos, str.length() );
    return str_out;
}


/** @brief 文字列をコピーし部分文字列 old を new_ に置換して返す (ASCIIだけignore case)
 *
 * @param[in] str コピーする文字列
 * @param[in] old 置き換え対象 (ASCIIだけ大文字小文字を無視)
 * @param[in] new_ 置き換える内容
 * @return 置換処理した結果
 * - old が空文字列のときは str を返す
 * - old が見つからなかったときは str を返す
 */
std::string MISC::replace_casestr( const std::string& str, const std::string& old, const std::string& new_ )
{
    if( old.empty() ) return str;

    std::string str_out;

    const char head[3] = { g_ascii_toupper( old[0] ), g_ascii_tolower( old[0] ) };
    std::size_t p0 = 0;
    std::size_t p1 = 0;
    while( true ) {
        const std::size_t p2 = str.find_first_of( head, p1 );
        if( p2 == std::string::npos ) break;
        // ヌル終端文字列が要件なので注意
        if( g_ascii_strncasecmp( str.c_str() + p2, old.data(), old.size() ) == 0 ) {
            str_out.append( str, p0, p2 - p0 );
            str_out.append( new_ );
            p0 = p1 = p2 + old.size();
            continue;
        }
        p1 = p2 + 1;
    }
    if( p0 == 0 ) return str;

    str_out.append( str, p0 );

    return str_out;
}


/** @brief list_inから pattern を replacement に置き換えてリストを返す
 *
 * @param[in] list_in 置き換えを実行する文字列のリスト
 * @param[in] pattern 置き換える文字列のパターン
 * @param[in] replacement マッチした文字列を置き換える内容
 * @return 置き換えを実行したリスト。
 */
std::list<std::string> MISC::replace_str_list( const std::list<std::string>& list_in,
                                               std::string_view pattern, std::string_view replacement )
{
    std::list< std::string > list_out;
    std::transform( list_in.cbegin(), list_in.cend(), std::back_inserter( list_out ),
                    [&]( const std::string& s ) { return replace_str( s, pattern, replacement ); } );
    return list_out;
}


/** @brief str に含まれる改行文字(\\r\\n)を replace に置き換え
 *
 * @param[in] str 処理する文字列
 * @param[in] replace マッチした改行文字と置き換える内容
 * @return 置き換えを実行した結果。str や replace が空文字列のときは str をそのまま返す。
 */
std::string MISC::replace_newlines_to_str( const std::string& str, std::string_view replace )
{
    if( str.empty() || replace.empty() ) return str;

    std::string str_out;
    str_out.reserve( str.size() );

    size_t pos = 0, found = 0;
    while( ( found = str.find_first_of( "\r\n", pos ) ) != std::string::npos )
    {
        str_out.append( str, pos, ( found - pos ) );
        str_out.append( replace );

        pos = found + 1;

        if( str[ found ] == '\r' && str[ found + 1 ] == '\n' ) ++pos;
    }

    str_out.append( str, pos );

    return str_out;
}


//
// " を \" に置き換え
//
std::string MISC::replace_quot( const std::string& str )
{
    return MISC::replace_str( str, "\"", "\\\"" );
}


//
// \" を " に置き換え
//
std::string MISC::recover_quot( const std::string& str )
{
    return MISC::replace_str( str, "\\\"", "\"" );
}


//
// 文字列(utf-8も) -> 整数変換
//
// (例) "12３" -> 123
//
// 入力:
// str
//
// 出力:
// dig: 桁数、0なら失敗
// n : str から何バイト読み取ったか
//
// 戻り値: 数値
//
int MISC::str_to_uint( const char* str, size_t& dig, size_t& n )
{
    int out = 0;
    dig = 0;
    n = 0;
    while( *str != '\0' ){

        const unsigned char in = (*str);
        
        if( '0' <=  in && in <= '9' ){

            out = out*10 + ( in - '0' );
            ++dig;
            ++str;
            ++n;
        }

        else{

            const auto in2 = static_cast<unsigned char>( in == 0xEF ? *( str + 1 ) : 0 );
            const auto in3 = static_cast<unsigned char>( in2 == 0xBC ? *( str + 2 ) : 0 );

            // utf-8
            if( 0x90 <= in3 && in3 <= 0x99 ){
                out = out*10 + ( in3 - 0x90 );
                ++dig;
                str += 3;
                n += 3;
            }

            else break;
        }
    }

    return out;
}


//
// listで指定した数字を文字に変換
//
std::string MISC::intlisttostr( const std::list< int >& list_num )
{
    assert( ! list_num.empty() );

    std::ostringstream comment;

    std::list < int >::const_iterator it = list_num.begin();

    bool comma = false;
    int num_from = *it;
    int num_to = -1; // -1 は番兵
    int i = 0; // 連番判定に使う
    for(;;){

        ++i;
        ++it;
        const bool loop_end{ it == list_num.end() };
        const int num{ loop_end ? -1 : *it };
        if( num_from + i != num || loop_end ) {

            if( comma ) comment << ",";
            comment << num_from;
            if( num_to != -1 ) comment << "-" << num_to;
            num_from = num;
            num_to = -1;
            i = 0;
            comma = true;

            if( loop_end ) break;
        }
        // 数字が連番のときは記録しておく
        else num_to = num;
    }

    return comment.str();
}



/** @brief 16進数表記文字列をバイト列に変換する( 例 "E38182" -> "\xE3\x81\x82" )
 *
 * @param[in]  chr_in 入力
 * @param[out] chr_out 出力 (not null)
 * @return 変換に成功した chr_in のバイト数
 */
std::size_t MISC::chrtobin( std::string_view chr_in, char* chr_out )
{
    assert( chr_out );
    if( chr_in.empty() ) return 0;

    const std::size_t chr_in_length = chr_in.length();

    std::size_t n = 0;
    for( ; n < chr_in_length; ++n )
    {
        const unsigned int chr = static_cast<unsigned char>( chr_in[n] );

        *chr_out <<= 4;

        // 0(0x30)〜9(0x39)
        if( ( chr - 0x30 ) < 10 ) *chr_out |= chr - 0x30;
        // A(0x41)〜F(0x46)
        else if( ( chr - 0x41 ) < 6 ) *chr_out |= chr - 0x37;
        // a(0x61)〜f(0x66)
        else if( ( chr - 0x61 ) < 6 ) *chr_out |= chr - 0x57;
        // その他
        else break;

        if( n % 2 != 0 ) ++chr_out;
    }

    return n;
}


//
// strが半角でmaxsize文字を超えたらカットして後ろに...を付ける
//
std::string MISC::cut_str( const std::string& str, const unsigned int maxsize )
{
    std::string outstr = str;
    unsigned int pos, lng_str;
    int byte = 0;
    const size_t outstr_length = outstr.length();

    for( pos = 0, lng_str = 0; pos < outstr_length; pos += byte ){
        byte = MISC::utf8bytes( outstr.c_str() + pos );
        if( byte > 1 ) lng_str += 2;
        else ++lng_str;
        if( lng_str >= maxsize ) break;
    }

    // カットしたら"..."をつける
    if( pos != outstr_length ) {
        outstr.resize( pos );
        outstr.append( "..." );
    }

    return outstr;
}


//
// 正規表現のメタ文字が含まれているか
//
// escape == true ならエスケープを考慮 (例)  escape == true なら \+ → \+ のまま、falseなら \+ → \\\+
//

#define REGEX_METACHARS ".+*?^$|{}[]()\\"

bool MISC::has_regex_metachar( const std::string& str, const bool escape )
{
    const char metachars[] = REGEX_METACHARS;
    const size_t str_length = str.length();

    for( size_t pos = 0; pos < str_length; ++pos ){

        if( escape && str[ pos ] == '\\' ){

            int i = 0;
            while( metachars[ i ] != '\0' ){

                if( str[ pos + 1 ] == metachars[ i ]  ) break;
                ++i;
            }

            if( metachars[ i ] == '\0' ) return true;

            ++pos;
        }
        else{

            int i = 0;
            while( metachars[ i ] != '\0' ){

                if( str[ pos ] == metachars[ i ] ) return true;
                ++i;
            }
        }
    }

    return false;
}


//
// 正規表現のメタ文字をエスケープ
//
// escape == true ならエスケープを考慮 (例)  escape == true なら \+ → \+ のまま、falseなら \+ → \\\+
//
std::string MISC::regex_escape( const std::string& str, const bool escape )
{
    if( ! has_regex_metachar( str, escape ) ) return str;

#ifdef _DEBUG
    std::cout << "MISC::regex_escape" << std::endl;
#endif

    std::string str_out;

    const char metachars[] = REGEX_METACHARS;
    const size_t str_length = str.length();

    for( size_t pos = 0; pos < str_length; ++pos ){

        if( escape && str[ pos ] == '\\' ){

            int i = 0;
            while( metachars[ i ] != '\0' ){

                if( str[ pos + 1 ] == metachars[ i ]  ) break;
                ++i;
            }

            if( metachars[ i ] == '\0' ) str_out += '\\';
            else{
                str_out += str[ pos ];
                ++pos;
            }
        }
        else{

            int i = 0;
            while( metachars[ i ] != '\0' ){

                if( str[ pos ] == metachars[ i ] ){
                    str_out += '\\';
                    break;
                }                    
                ++i;
            }
        }

        str_out += str[ pos ];
    }

#ifdef _DEBUG
    std::cout << str << " -> " << str_out << std::endl;
#endif

    return str_out;
}


//
// 正規表現のメタ文字をアンエスケープ
//
std::string MISC::regex_unescape( const std::string& str )
{
#ifdef _DEBUG
    std::cout << "MISC::regex_unescape" << std::endl;
#endif

    std::string str_out;

    const char metachars[] = REGEX_METACHARS;
    const size_t str_length = str.length();

    for( size_t pos = 0; pos < str_length; ++pos ){

        if( str[ pos ] == '\\' ){

            int i = 0;
            while( metachars[ i ] != '\0' ){

                if( str[ pos + 1 ] == metachars[ i ] ){
                    ++pos;
                    break;
                }
                ++i;
            }
        }

        str_out += str[ pos ];
    }

#ifdef _DEBUG
    std::cout << str << " -> " << str_out << std::endl;
#endif

    return str_out;
}


/** @brief HTMLで特別な意味を持つ記号(& " < >)を文字実体参照へエスケープする
 *
 * @param[in] str        エスケープする入力
 * @param[in] completely URL中でもエスケープする( デフォルト = true )
 * @return エスケープした結果
 */
std::string MISC::html_escape( const std::string& str, const bool completely )
{
    if( str.empty() ) return str;

    bool is_url = false;
    int scheme = SCHEME_NONE;
    std::string str_out;
    const size_t str_length = str.length();

    for( size_t pos = 0; pos < str_length; ++pos )
    {
        char tmpchar = str.c_str()[ pos ];

        // URL中はエスケープしない場合
        if( ! completely )
        {
            // URLとして扱うかどうか
            // エスケープには影響がないので loose_url としておく
            if( scheme != SCHEME_NONE ) is_url = is_url_char( str.c_str() + pos, true );

            // URLスキームが含まれているか判別
            int len = 0;
            if( ! is_url ) scheme = is_url_scheme( str.c_str() + pos, &len );

            // URLスキームが含まれていた場合は文字数分進めてループに戻る
            if( len > 0 )
            {
                str_out += str.substr( pos, len );
                pos += len - 1; // あとで ++pos される分を引く
                continue;
            }
        }

        // completely = false でURL中ならエスケープしない
        if( is_url ) str_out += tmpchar;
        else if( tmpchar == '&' ) str_out += "&amp;";
        else if( tmpchar == '\"' ) str_out += "&quot;";
        else if( tmpchar == '<' ) str_out += "&lt;";
        else if( tmpchar == '>' ) str_out += "&gt;";
        else str_out += tmpchar;
    }

#ifdef _DEBUG
    if( str != str_out ){
        std::cout << "MISC::html_escape\nstr = " << str << std::endl
                  << "out = " << str_out << std::endl;
    }
#endif

    return str_out;
}


/** @brief HTMLで特別な意味を持つ記号の文字実体参照(\&quot; \&amp; \&lt; \&gt;)をアンエスケープする
 *
 * @param[in] str アンエスケープする入力
 * @return アンエスケープした結果
 */
std::string MISC::html_unescape( const std::string& str )
{
    if( str.empty() ) return str;
    if( str.find( '&' ) == std::string::npos ) return str;

    std::string str_out;
    str_out.reserve( str.size() );
    const char* pos = str.c_str();
    const char* pos_end = pos + str.size();

    while( pos < pos_end ){

        // '&' までコピーする
        while( *pos != '&' && *pos != '\0' ) str_out.push_back( *pos++ );
        if( pos >= pos_end ) break;

        // エスケープ用の文字参照をデコード
        if( std::strncmp( pos, "&quot;", 6 ) == 0 ){ str_out.push_back( '"' ); pos += 6; }
        else if( std::strncmp( pos, "&amp;", 5 ) == 0 ){ str_out.push_back( '&' ); pos += 5; }
        else if( std::strncmp( pos, "&lt;", 4 ) == 0 ){ str_out.push_back( '<' ); pos += 4; }
        else if( std::strncmp( pos, "&gt;", 4 ) == 0 ){ str_out.push_back( '>' ); pos += 4; }
        else str_out.push_back( *pos++ );
    }

#ifdef _DEBUG
    if( str != str_out ){
        std::cout << "MISC::html_unescape\nstr = " << str << std::endl
                  << "out = " << str_out << std::endl;
    }
#endif

    return str_out;
}



//
// 文字参照のデコード内部処理
//
// strは'&'で始まる文字列を指定すること
// completely = true の時は'"' '&' '<' '>'も含めて変換する
//
static std::string chref_decode_one( const char* str, int& n_in, const char pre_char, const bool completely )
{
    std::string out_char( 15u, '\0' );
    int n_out;
    const int type = DBTREE::decode_char( str, n_in, out_char, n_out );
    out_char.resize( n_out );

    // 改行、タブ、スペースの処理
    if( type == DBTREE::NODE_SP && pre_char != ' ' ) {
        out_char.assign( 1u, ' ' );
    }
    // 変換できない文字
    else if( type == DBTREE::NODE_NONE ) {
        out_char.assign( 1u, *str );
        n_in = 1;
    }
    // エスケープする文字の場合は元に戻す
    else if( ! completely && n_out == 1 ) {
        switch( out_char[0] ) {
            case '"':
                out_char.assign( "&quot;" );
                break;
            case '&':
                out_char.assign( "&amp;" );
                break;
            case '<':
                out_char.assign( "&lt;" );
                break;
            case '>':
                out_char.assign( "&gt;" );
                break;
            default:
                break;
        }
    }

    return out_char;
}


/** @brief HTMLをプレーンテキストに変換する
 *
 * @details HTMLタグを取り除き文字参照をデコードして返す。
 * @param[in] html プレーンテキストに変換する入力
 * @return 変換した結果
 */
std::string MISC::to_plain( const std::string& html )
{
    if( html.empty() ) return html;
    if( html.find_first_of( "<&" ) == std::string::npos ) return html;

    std::string str_out;
    const char* pos = html.c_str();
    const char* pos_end = pos + html.size();

    while( pos < pos_end ){

        // '<' か '&' までコピーする
        while( *pos != '<' && *pos != '&' && *pos != '\0' ) str_out.push_back( *pos++ );
        if( pos >= pos_end ) break;

        // タグを取り除く
        if( *pos == '<' ){
            while( *pos != '>' && *pos != '\0' ) pos++;
            if( *pos == '>' ) ++pos;
            continue;
        }

        // 文字参照を処理する
        if( *pos == '&' ){
            int n_in;
            const char pre = str_out.empty() ? '\0' : str_out.back();
            str_out += chref_decode_one( pos, n_in, pre, true );
            pos += n_in;
        }
    }

    return str_out;
}


/** @brief HTMLをPango markupテキストに変換する
 *
 * @details @c @<mark@> と @c @<span@> タグの色を設定して文字参照をデコードして返す。
 * @param[in] html Pango markupテキストに変換する入力
 * @return 変換した結果
 */
std::string MISC::to_markup( const std::string& html )
{
    if( html.empty() ) return html;
    if( html.find_first_of( "<&" ) == std::string::npos ) return html;

    std::string markuptxt;
    const char* pos = html.c_str();
    const char* pos_end = pos + html.size();

    while( pos < pos_end ){

        // '<' か '&' までコピーする
        while( *pos != '<' && *pos != '&' && *pos != '\0' ) markuptxt.push_back( *pos++ );
        if( pos >= pos_end ) break;

        // タグを処理する
        if( *pos == '<' ) {
            ++pos;

            // <mark>と<span>タグは色を変える
            if( std::strncmp( pos, "mark", 4 ) == 0 || std::strncmp( pos, "span", 4 ) == 0 ) {
                std::string classname = ( ( *pos ) == 'm' ) ? "mark" : "";
                pos += 4;
                if( std::strncmp( pos, " class=\"", 8 ) == 0 ) {
                    pos += 8;
                    const char* pos_name = pos;
                    while( *pos != '"' && *pos != '\0' ) ++pos;
                    classname.assign( pos_name, pos - pos_name );
                    if( *pos != '\0' ) ++pos;
                }

                markuptxt += "<span";

                if( classname.size() ) {
                    const CORE::Css_Manager* const mgr = CORE::get_css_manager();
                    const int classid = mgr->get_classid( classname );
                    if( classid != -1 ) {
                        const CORE::CSS_PROPERTY& css = mgr->get_property( classid );
                        if( css.color != -1 ) markuptxt += " color=\"" + mgr->get_color( css.color ) + "\"";
                        if( css.bg_color != -1 ) markuptxt += " background=\"" + mgr->get_color( css.bg_color ) + "\"";
                    }
                }

                while( *pos != '>' && *pos != '\0' ) markuptxt.push_back( *pos++ );
                markuptxt.push_back( '>' );
                if( *pos != '\0' ) ++pos;
                continue;
            }

            // </mark> は </span> に置換する
            if( std::strncmp( pos, "/mark>", 6 ) == 0 || std::strncmp( pos, "/span>", 6 ) == 0 ) {
                pos += 6;
                markuptxt += "</span>";
                continue;
            }

            // XXX: その他のタグは取り除く
            while( *pos != '>' && *pos != '\0' ) ++pos;
            if( *pos == '>' ) ++pos;
            continue;
        }

        // 文字参照を処理する
        if( *pos == '&' ) {
            if( std::strncmp( pos + 1, "quot;", 5 ) == 0 ) {
                markuptxt.push_back( '"' );
                pos += 6;
            }
            else {
                int n_in;
                const char pre{ markuptxt.empty() ? '\0' : markuptxt.back() };
                markuptxt += chref_decode_one( pos, n_in, pre, false );
                if( n_in == 1 && markuptxt.back() == '&' ){
                    markuptxt += "amp;";
                }
                pos += n_in;
            }
        }
    }

    return markuptxt;
}


/** @brief 文字列をコピーしてHTML文字参照をデコードする
 *
 * @param[in] str        デコード処理する文字列
 * @param[in] completely `true` の時は`"` `&` `<` `>` もデコードする
 * @return デコード処理した結果
 */
std::string MISC::chref_decode( std::string_view str, const bool completely )
{
    std::string str_out;

    if( str.empty() ) return str_out;
    if( str.find( '&' ) == std::string_view::npos ) {
        str_out.assign( str );
        return str_out;
    }

    const char* pos = str.data();
    const char* pos_end = str.data() + str.size();

    while( pos < pos_end ) {

        // '&' までコピーする
        while( *pos != '&' && pos < pos_end ) str_out.push_back( *pos++ );
        if( pos >= pos_end ) break;

        // 文字参照のデコード
        int n_in;
        str_out.append( chref_decode_one( pos, n_in, '\0', completely ) );
        pos += n_in;
    }

    return str_out;
}


//
// URL中のスキームを判別する
//
// 戻り値 : スキームタイプ
// length    : "http://"等の文字数
//
int MISC::is_url_scheme_impl( const char* str_in, int* length )
{
    int scheme = SCHEME_NONE;
    int len = 0;

    // http https
    if( MISC::starts_with( str_in, "http" ) )
    {
        scheme = SCHEME_HTTP;
        len = 4;
        if( *( str_in + len ) == 's' ) ++len;
    }
    // ftp
    else if( MISC::starts_with( str_in, "ftp" ) )
    {
        scheme = SCHEME_FTP;
        len = 3;
    }
    // ttp ttps
    else if( MISC::starts_with( str_in, "ttp" ) )
    {
        scheme = SCHEME_TTP;
        len = 3;
        if( *( str_in + len ) == 's' ) ++len;
    }
    // tp tps
    else if( MISC::starts_with( str_in, "tp" ) )
    {
        scheme = SCHEME_TP;
        len = 2;
        if( *( str_in + len ) == 's' ) ++len;
    }
    // sssp
    else if( MISC::starts_with( str_in, "sssp" ) ) {
        if( MISC::starts_with( str_in + 7, "img.5ch" ) || MISC::starts_with( str_in + 7, "img.2ch" ) ) {
            scheme = SCHEME_SSSP;
        }
        else{
            // XXX img.[25]ch以外のアドレスはHTTPスキームにする
            scheme = SCHEME_HTTP;
        }
        len = 4;
    }

    // 各スキーム後に続く共通の"://"
    if( MISC::starts_with( str_in + len, "://" ) )
    {
        len += 3;
        if( length ) *length = len;
    }
    else scheme = SCHEME_NONE;

    return scheme;
}


//
// URLとして扱う文字かどうか判別する
//
// 基本 : 「!#$%&'()*+,-./0-9:;=?@A-Z_a-z~」
// 拡張 : 「[]^|」
//
// "RFC 3986" : http://www.ietf.org/rfc/rfc3986.txt
// "RFC 2396" : http://www.ietf.org/rfc/rfc2396.txt
//
static const char s_url_char[ 128 ] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//         !  "  #  $  %  &  '  (  )  *  +  ,  -  .  /
        0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
//      0  1  2  3  4  5  6  7  8  9  :  ;  <  =  >  ?
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1,
//      @  A  B  C  D  E  F  G  H  I  J  K  L  M  N  O
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
//      P  Q  R  S  T  U  V  W  X  Y  Z  [  \  ]  ^  _
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 0, 2, 2, 1,
//      `  a  b  c  d  e  f  g  h  i  j  k  l  m  n  o
        0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
//      p  q  r  s  t  u  v  w  x  y  z  {  |  }  ~
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 2, 0, 1, 0,
};
bool MISC::is_url_char( const char* str_in, const bool loose_url )
{
    const unsigned char c = static_cast<unsigned char>(*str_in);

    // 128以上のテーブルはないので先に判定
    if( c & 0x80 ) return false;

    // 基本
    if( s_url_char[ c ] == 1 ) return true;

    // 拡張
    // RFC 3986(2.2.)では"[]"が予約文字として定義されているが
    // RFC 2396(2.4.3.)では除外されていて、普通にURLとして扱う
    // と問題がありそうなので"loose_url"の扱いにしておく。
    if( loose_url && s_url_char[ c ] == 2 ) return true;

    return false;
}



/** @brief URLに含まれるパーセントエンコーディングをバイト列にデコードする
 *
 * @param[in] url デコードするURL
 * @return デコードした結果
 */
std::string MISC::url_decode( std::string_view url )
{
    std::string decoded;
    if( url.empty() ) return decoded;

    const std::size_t size = url.size();
    for( std::size_t n = 0; n < size; ++n )
    {
        if( url[n] == '%' && ( n + 2 ) < size )
        {
            char src[3] = { url[ n + 1 ], url[ n + 2 ], '\0' };
            char tmp[3] = { '\0', '\0', '\0' };

            if( chrtobin( src, tmp ) == 2 )
            {
                // 改行はLFにする
                if( *tmp == '\n' && ! decoded.empty() && decoded.back() == '\r' )
                {
                    decoded.pop_back();
                }
                // '%4A' など、2文字が変換できていること
                decoded.push_back( *tmp );
                n += 2;
            }
            else
            {
                // 変換失敗は、単なる '%' 文字として扱う
                decoded.push_back( url[n] );
            }
        }
        else if( url[n] == '+' )
        {
            decoded.push_back( ' ' );
        }
        else
        {
            decoded.push_back( url[n] );
        }
    }

    return decoded;
}


/** @brief 文字列(バイト列)をパーセント符号化して返す
 *
 * @details URL Living Standard が定めるpercent-encode setを参考にバイトを符号化する。
 * - 半角英数字(0-9 A-Z a-z)と一部記号(* - . _)は変換しない
 * - それ以外はパーセント記号ではじまる16進表記 \%XX に変換 (A-Fは大文字)
 *
 * @param[in] str 入力文字列 (文字エンコーディングは任意)
 * @return パーセント符号化された文字列
 * @see MISC::url_encode( const std::string& utf8str, const Encoding encoding )
*/
std::string MISC::url_encode( std::string_view str )
{
    std::string str_encoded;
    constexpr std::size_t tmplng = 8;
    char str_tmp[tmplng];

    for( const char c : str ) {
        if( g_ascii_isalnum( c )  || c == '*' || c == '-' || c == '.' || c == '_' ) {
            str_encoded.push_back( c );
        }
        else {
            std::snprintf( str_tmp, tmplng, "%%%02X", static_cast<unsigned char>( c ) );
            str_encoded.append( str_tmp );
        }
    }

    return str_encoded;
}


/** @brief UTF-8文字列をエンコーディング変換してからパーセント符号化して返す
 *
 * @details `utf8str` を `encoding` で指定した文字エンコーディングに変換してから符号化する。
 * @param[in] utf8str 入力文字列 (文字エンコーディングはUTF-8)
 * @param[in] encoding 変換先の文字エンコーディング
 * @return パーセント符号化された文字列
 * @see MISC::url_encode( std::string_view str )
*/
std::string MISC::url_encode( const std::string& utf8str, const Encoding encoding )
{
    if( encoding == Encoding::utf8 ) return MISC::url_encode( utf8str );

    const std::string str_enc = MISC::Iconv( utf8str, encoding, Encoding::utf8 );
    return MISC::url_encode( str_enc );
}


/** @brief application/x-www-form-urlencoded の形式でパーセント符号化する
 *
 * webブラウザのform要素の挙動に合わせてURLの規格と異なる変換処理がある。
 * - 半角英数字(0-9 A-Z a-z)と一部記号(* - . _)は変換しない
 * - 改行LF(U+000A) は \%0D\%0A に変換する
 * - 改行CR(U+000D) は読み飛ばして無視する
 * - 半角空白(U+0020)は + プラス記号(U+002B)に置換する
 * - それ以外はパーセント記号ではじまる16進表記 \%XX に変換 (A-Fは大文字)
 *
 * @param[in] str 入力文字列 (文字エンコーディングは任意)
 * @return パーセント符号化された文字列
 * @see MISC::url_encode_plus( const std::string& utf8str, const Encoding encoding )
*/
std::string MISC::url_encode_plus( std::string_view str )
{
    std::string str_encoded;
    constexpr std::size_t tmplng = 8;
    char str_tmp[tmplng];

    for( const char c : str ) {
        if( g_ascii_isalnum( c ) || c == '-' || c == '.' || c == '_' || c == '*' ) {
            str_encoded.push_back( c );
        }
        else if( c == ' ' ) {
            str_encoded.push_back( '+' );
        }
        else if( c == '\n' ) {
            str_encoded.append( "%0D%0A" );
        }
        else if( c != '\r' ) {
            std::snprintf( str_tmp, tmplng, "%%%02X", static_cast<unsigned char>( c ) );
            str_encoded.append( str_tmp );
        }
    }

    return str_encoded;
}


/** @brief UTF-8文字列をエンコーディング変換してから application/x-www-form-urlencoded の形式でパーセント符号化する
 *
 * @details `utf8str` を `encoding` で指定した文字エンコーディングに変換してから符号化する。
 * @param[in] utf8str 入力文字列 (文字エンコーディングはUTF-8)
 * @param[in] encoding 変換先の文字エンコーディング名
 * @return パーセント符号化された文字列
 * @see MISC::url_encode_plus( std::string_view str )
*/
std::string MISC::url_encode_plus( const std::string& utf8str, const Encoding encoding )
{
    if( encoding == Encoding::utf8 ) {
        return MISC::url_encode_plus( utf8str );
    }

    const std::string str_enc = MISC::Iconv( utf8str, encoding, Encoding::utf8 );
    return MISC::url_encode_plus( str_enc );
}


//
// BASE64
//
std::string MISC::base64( const std::string& str )
{
    constexpr const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    int lng = str.length();

    std::string out;
    out.reserve( lng * 2 );

    std::string data = str + "\0\0\0\0";

    for( int i = 0; i < lng; i += 3 ){

        const auto cstr = reinterpret_cast<const unsigned char*>( data.c_str() + i );
        unsigned char key[ 4 ];

        key[ 0 ] = (*cstr) >> 2;
        key[ 1 ] = static_cast<unsigned char>( ( (*cstr) << 4 ) + ( (*(cstr+1)) >> 4 ) );
        key[ 2 ] = static_cast<unsigned char>( ( (*(cstr+1)) << 2 ) + ( (*(cstr+2)) >> 6 ) );
        key[ 3 ] = *(cstr+2);

        for( int j = 0; j < 4; ++j ){
            key[ j ] &= 0x3f;
            out += table[ key[ j ] ];
        }
    }

    if( lng % 3 == 1 ){
        out[ out.length()-2 ] = '=';
        out[ out.length()-1 ] = '=';
    }
    else if( lng % 3 == 2 ){
        out[ out.length()-1 ] = '=';
    }

#ifdef _DEBUG
    std::cout << "MISC::base64 " << str << " -> " << out << std::endl;
#endif 

    return out;
}



//
// 「&#数字;」形式の数字参照文字列の中の「数字」部分の文字列長
//
// in_char: 入力文字列、in_char[0] == "&" && in_char[1] == "#" であること
// offset : 開始位置が返る
//
// 戻り値 : 「&#数字;」の中の数字の文字列の長さ、変換出来ないときは -1
//
// 例 : &#9999; なら 戻り値 = 4、 offset = 2
//
int MISC::spchar_number_ln( const char* in_char, int& offset )
{
    int lng = 0;
    offset = 2;

    // offset == 2 なら 10 進数、3 なら16進数
    if( in_char[ offset ] == 'x' || in_char[ offset ] == 'X' ) ++offset;

    // UTF-32の範囲でデコードするので最大1114111
    // デコードするとき「;」で終端されていなくてもよい

    // デコード可能かチェック
    // 10 進数
    if( offset == 2 ){

        // 最大7桁 (&#1114111;)
        for( lng = 0; lng <= 7; lng++ ){
            if( in_char[ offset + lng ] < '0' || in_char[ offset + lng ] > '9' ) break;
        }
        
        // 桁数チェック
        if( lng == 0 || lng == 8 ) return -1;
    }

    // 16 進数
    else{

        // 最大6桁 (&#x10FFFF;)
        for( lng = 0; lng <= 6; lng++ ){
            if(
                ! (
                    ( in_char[ offset + lng ] >= '0' && in_char[ offset + lng ] <= '9' )
                    || ( in_char[ offset + lng ] >= 'a' && in_char[ offset + lng ] <= 'f' )
                    || ( in_char[ offset + lng ] >= 'A' && in_char[ offset + lng ] <= 'F' )
                    )
                ) break;
        }
        
        // 桁数チェック
        if( lng == 0 || lng == 7 ) return -1;
    }

    return lng;
}


// 特定の変換が必要なコードポイントをチェックする
static char32_t transform_7f_9f( char32_t raw_point )
{
    switch( raw_point ) {
        case 0x80: return 0x20AC; // EURO SIGN (€)
        case 0x82: return 0x201A; // SINGLE LOW-9 QUOTATION MARK (‚)
        case 0x83: return 0x0192; // LATIN SMALL LETTER F WITH HOOK (ƒ)
        case 0x84: return 0x201E; // DOUBLE LOW-9 QUOTATION MARK („)
        case 0x85: return 0x2026; // HORIZONTAL ELLIPSIS (…)
        case 0x86: return 0x2020; // DAGGER (†)
        case 0x87: return 0x2021; // DOUBLE DAGGER (‡)
        case 0x88: return 0x02C6; // MODIFIER LETTER CIRCUMFLEX ACCENT (ˆ)
        case 0x89: return 0x2030; // PER MILLE SIGN (‰)
        case 0x8A: return 0x0160; // LATIN CAPITAL LETTER S WITH CARON (Š)
        case 0x8B: return 0x2039; // SINGLE LEFT-POINTING ANGLE QUOTATION MARK (‹)
        case 0x8C: return 0x0152; // LATIN CAPITAL LIGATURE OE (Œ)
        case 0x8E: return 0x017D; // LATIN CAPITAL LETTER Z WITH CARON (Ž)
        case 0x91: return 0x2018; // LEFT SINGLE QUOTATION MARK (‘)
        case 0x92: return 0x2019; // RIGHT SINGLE QUOTATION MARK (’)
        case 0x93: return 0x201C; // LEFT DOUBLE QUOTATION MARK (“)
        case 0x94: return 0x201D; // RIGHT DOUBLE QUOTATION MARK (”)
        case 0x95: return 0x2022; // BULLET (•)
        case 0x96: return 0x2013; // EN DASH (–)
        case 0x97: return 0x2014; // EM DASH (—)
        case 0x98: return 0x02DC; // SMALL TILDE (˜)
        case 0x99: return 0x2122; // TRADE MARK SIGN (™)
        case 0x9A: return 0x0161; // LATIN SMALL LETTER S WITH CARON (š)
        case 0x9B: return 0x203A; // SINGLE RIGHT-POINTING ANGLE QUOTATION MARK (›)
        case 0x9C: return 0x0153; // LATIN SMALL LIGATURE OE (œ)
        case 0x9E: return 0x017E; // LATIN SMALL LETTER Z WITH CARON (ž)
        case 0x9F: return 0x0178; // LATIN CAPITAL LETTER Y WITH DIAERESIS (Ÿ)
        default:
            return 0xFFFD; // REPLACEMENT CHARACTER
    }
}


/** @brief 「`&#数字;`」形式の数値文字参照をコードポイント(char32_t)に変換する
 *
 * @details 数値文字参照の解析エラーとなる値もそのまま返す
 * (Unicodeの範囲外、サロゲート、非文字など)
 * @param[in] in_char 入力文字列、 `in_char[0] == "&" && in_char[1] == "#"` であること (not null)
 * @param[in] offset  spchar_number_ln() の戻り値
 * @param[in] lng     spchar_number_ln() の戻り値
 * @return 「`&#数字;`」の中の数字(char32_t型)
 * @remarks 最初に MISC::spchar_number_ln() を呼び出して `offset` と `lng` を取得すること
 */
char32_t MISC::decode_spchar_number_raw( const char* in_char, const int offset, const int lng )
{
    char str_num[ 16 ];

    std::memcpy( str_num, in_char + offset, lng );
    str_num[ lng ] = '\0';

#ifdef _DEBUG
    std::cout << "MISC::decode_spchar_number_raw offset = " << offset << " lng = " << lng
              << " str = " << str_num << std::endl;
#endif

    const int base{ offset == 2 ? 10 : 16 };
    return static_cast<char32_t>( std::strtoul( str_num, nullptr, base ) );
}


/** @brief 「`&#数字;`」形式の数値文字参照をコードポイント(char32_t)に変換する
 *
 * @details 数値文字参照の解析エラーとなる値は規定の値に変換して返す
 * @param[in] in_char 入力文字列、 `in_char[0] == "&" && in_char[1] == "#"` であること (not null)
 * @param[in] offset  spchar_number_ln() の戻り値
 * @param[in] lng     spchar_number_ln() の戻り値
 * @return 「`&#数字;`」の中の数字(char32_t型)
 * @remarks 最初に MISC::spchar_number_ln() を呼び出して `offset` と `lng` を取得すること
 */
char32_t MISC::decode_spchar_number( const char* in_char, const int offset, const int lng )
{
    const char32_t uch = MISC::decode_spchar_number_raw( in_char, offset, lng );
    return MISC::sanitize_numeric_charref( uch );
}


/** @brief コードポイントが数値文字参照の無効・解析エラーなら規定の値へ変換する
 *
 * @details WHATWG の仕様と異なり
 * C0/C1 controls のうち変換表にリストされていないものは U+FFFD へ変換する
 * @param[in]  uch            数値文字参照を解析して得た値
 * @param[out] high_surrogate `uch` が上位サロゲート(U+D800 - U+DBFF)のときは代入して返す (nullable)
 * @return 変換した結果
 *
 * @remarks 参考文献 : Numeric character reference end state (WHATWG) @n
 * https://html.spec.whatwg.org/multipage/parsing.html#numeric-character-reference-end-state
 */
char32_t MISC::sanitize_numeric_charref( const char32_t uch, char32_t* high_surrogate )
{
    constexpr char32_t replace = 0xFFFD; // REPLACEMENT CHARACTER
    if( uch >= 0xD800 ) {
        // 特定のbitパターンの非文字をチェック
        if( ( uch & 0xFFFE ) == 0xFFFE ) return replace;
        // 上位サロゲート (U+D800 - U+DBFF)
        else if( uch < 0xDC00 ) {
            if( high_surrogate ) *high_surrogate = uch;
            return replace;
        }
        // 下位サロゲート (U+DC00 - U+DFFF)
        else if( uch < 0xE000 ) return replace;
        else if( uch < 0xFDD0 ) return uch;
        // 非文字 (noncharacters, U+FDD0 - U+FDEF)
        else if( uch < 0xFDF0 ) return replace;
        else if( uch < 0x110000 ) return uch;
        // Unicodeの範囲外
        return replace;
    }
    else if( uch < 0x00A0 ) {
        // C1 Controls
        if( 0x007F <= uch ) return transform_7f_9f( uch );
        // ASCII printable characters
        else if( 0x0020 <= uch ) return uch;
        else if( 0x000D <= uch ) return replace;
        // FORM FEED (FF)
        else if( 0x000C == uch ) return uch;
        // LINE TABULATION (VT)
        else if( 0x000B == uch ) return replace;
        // CHARACTER TABULATION (HT) and LINE FEED (LF)
        else if( 0x0009 <= uch ) return uch;
        return replace;
    }
    return uch;
}


//
// str に含まれる「&#数字;」形式の数字参照文字列を全てユニーコード文字に変換する
//
std::string MISC::decode_spchar_number( const std::string& str )
{
    std::string str_out;
    const size_t str_length = str.length();

    for( size_t i = 0; i < str_length ; ++i ){

        if( str[ i ] == '&' && str[ i + 1 ] == '#' ){

            int offset;
            const int lng = MISC::spchar_number_ln( str.c_str()+i, offset );
            if( lng == -1 ){
                str_out += str[ i ];
                continue;
            }

            const char32_t uch = MISC::decode_spchar_number( str.c_str()+i, offset, lng );

            char out_char[8];
            const int n_out = MISC::utf32toutf8( uch, out_char );
            if( ! n_out ){
                str_out += str[ i ];
                continue;
            }

            str_out.append( out_char, n_out );
            i += offset + lng;
        }
        else str_out += str[ i ];
    }

    return str_out;
}


//
// str を大文字化
//
std::string MISC::toupper_str( const std::string& str )
{
    std::string str_out;
    std::transform( str.cbegin(), str.cend(), std::back_inserter( str_out ),
                    []( unsigned char c ) { return std::toupper( c ); } );

    return str_out;
}


//
// list 内のアイテムを全部大文字化
//
std::list< std::string > MISC::toupper_list( const std::list< std::string >& list_str )
{
    std::list< std::string > list_out;
    std::transform( list_str.cbegin(), list_str.cend(), std::back_inserter( list_out ),
                    []( const std::string& s ) { return MISC::toupper_str( s ); } );

    return list_out;
}



//
// str を小文字化
//
std::string MISC::tolower_str( const std::string& str )
{
    std::string str_out;
    std::transform( str.cbegin(), str.cend(), std::back_inserter( str_out ),
                    []( unsigned char c ) { return std::tolower( c ); } );

    return str_out;
}



/** @brief path からホスト名だけ取り出してコピーを返す
 *
 * @param[in] path     ホスト名を取り出す文字列
 * @param[in] protocol false のときはプロトコルを除く
 * @return ホスト名の文字列 (protocolがtrueのときは先頭にプロトコルが付く)
 */
std::string MISC::get_hostname( std::string_view path, bool protocol )
{
    std::size_t lng = 0;
    if( path.rfind( "http://", 0 ) == 0 ) lng = strlen( "http://" );
    else if( path.rfind( "https://", 0 ) == 0 ) lng = strlen( "https://" );
    else if( path.rfind( "ftp://", 0 ) == 0 ) lng = strlen( "ftp://" );
    if( !lng ) return std::string();

    std::size_t pos = 0;
    if( ! protocol ) pos = lng;

    size_t i = path.find( '/', lng );

    if( i == std::string_view::npos ) return std::string{ path.substr( pos ) };

    return std::string{ path.substr( pos, i - pos ) };
}



//
// path からファイル名だけ取り出す
//
std::string MISC::get_filename( const std::string& path )
{
    if( path.empty() ) return std::string();

    size_t i = path.rfind( '/' );
    if( i == std::string::npos ) return path;

    return path.substr( i+1 );
}



//
// path からファイル名を除いてディレクトリだけ取り出す
//
std::string MISC::get_dir( const std::string& path )
{
    if( path.empty() ) return std::string();

    size_t i = path.rfind( '/' );
    if( i == std::string::npos ) return std::string();

    return path.substr( 0, i+1 );
}



//
// 文字数を限定して環境変数の値を返す
//
std::string MISC::getenv_limited( const char *name, const size_t size )
{
    if( ! name ) return {};
    char* p = getenv( name );
    if( ! p ) return {};

    std::string env{ p };
    if( env.size() > size ) env.resize( size );
    return env;
}


//
// pathセパレータを / に置き換える
//
std::string MISC::recover_path( const std::string& str )
{
    return str;
}

std::vector<std::string> MISC::recover_path( const std::vector<std::string>& list_path )
{
    return list_path;
}



//
// 文字列(utf-8)に全角英数字が含まれるか判定する
//
bool MISC::has_widechar( const char* str )
{
    while( *str != '\0' ){

        const unsigned char in = *str;

        if( ( in & 0xf0 ) == 0xe0 ){

            if( in == 0xef ){

                const auto in2 = static_cast<unsigned char>( *( str + 1 ) );
                const auto in3 = static_cast<unsigned char>( in2 != '\0' ? *( str + 2 ) : 0 );

                if( in2 == 0xbc ){

                    // 全角数字
                    if( 0x90 <= in3 && in3 <= 0x99 ) return true;

                    // 全角大文字
                    else if( 0xa1 <= in3 && in3 <= 0xba ) return true;
                }

                //  全角小文字
                else if( in2 == 0xbd && ( 0x81 <= in3 && in3 <= 0x9a ) ) return true;

                // 半角かな
                else if( ( in2 == 0xbd && ( 0xa1 <= in3 && in3 <= 0xbf ) )
                         || ( in2 == 0xbe && ( 0x80 <= in3 && in3 <= 0x9f ) ) ) return true;
            }

            str += 3;
        }

        else if( ( in & 0xe0 ) == 0xc0 ) str += 2;

        else if( ( in & 0xf8 ) == 0xf0 ) str += 4;

        else ++str;
    }

    return false;
}


//
// 全角英数字(str1) -> 半角英数字(str2)
//
// table_pos : 置き換えた文字列の位置
//
void MISC::asc( const char* str1, std::string& str2, std::vector< int >& table_pos )
{
    int pos = 0;
    while( str1[ pos ] != '\0' ) {
        assert( pos >= 0 );
        assert( table_pos.max_size() > table_pos.size() );
        const auto in1 = static_cast< unsigned char >( str1[ pos ] );

        if( in1 == 0xef ) {
            const auto in2 = static_cast< unsigned char >( str1[ pos + 1 ] );
            unsigned char in3 = 0;
            if ( in2 ) in3 = static_cast< unsigned char >( str1[ pos + 2 ] );

            if( in2 == 0xbc ){
                //  全角数字 (U+FF10 - U+FF19)
                if( 0x90 <= in3 && in3 <= 0x99 ){
                    str2.push_back( '0' + in3 - 0x90 );
                    table_pos.push_back( pos );
                    pos += 3;
                    continue;
                }
                //  全角大文字 (U+FF21 - U+FF3A)
                else if( 0xa1 <= in3 && in3 <= 0xba ){
                    str2.push_back( 'A' + in3 - 0xa1 );
                    table_pos.push_back( pos );
                    pos += 3;
                    continue;
                }
            }

            //  全角小文字 (U+FF41 - U+FF5A)
            else if( in2 == 0xbd && ( 0x81 <= in3 && in3 <= 0x9a ) ){
                str2.push_back( 'a' + in3 - 0x81 );
                table_pos.push_back( pos );
                pos += 3;
                continue;
            }

            // 半角かな (U+FF61 - U+FF9F)
            else if( ( in2 == 0xbd && ( 0xa1 <= in3 && in3 <= 0xbf ) )
                     || ( in2 == 0xbe && ( 0x80 <= in3 && in3 <= 0x9f ) ) ){

                bool flag_hkana = false;
                bool dakuten = false;
                size_t i = 0;

                // 濁点、半濁点
                unsigned char in4 = 0;
                unsigned char in5 = 0;
                if ( in3 ) in4 = static_cast< unsigned char >( str1[ pos + 3 ] );
                if ( in4 ) in5 = static_cast< unsigned char >( str1[ pos + 4 ] );
                if( in4 == 0xef && in5 == 0xbe ){
                    const auto in6 = static_cast< unsigned char >( str1[ pos + 5 ] );

                    // 濁点
                    if( in6 == 0x9e ){
                        dakuten = true;
                        i = 61;
                    }
                    // 半濁点
                    else if( in6 == 0x9f ){
                        dakuten = true;
                        i = 61 + 21;
                    }
                }

                while( hkana_table1[ i ][ 0 ][ 0 ] != '\0' ){

                    if( in1 == hkana_table1[ i ][ 0 ][ 0 ]
                            && in2 == hkana_table1[ i ][ 0 ][ 1 ]
                            && in3 == hkana_table1[ i ][ 0 ][ 2 ] ) {

                        std::copy_n( hkana_table1[ i ][ 1 ], 3, std::back_inserter( str2 ) );
                        std::generate_n( std::back_inserter( table_pos ), 3, [&pos]{ return pos++; } );

                        if( dakuten ) pos += 3;
                        flag_hkana = true;
                        break;
                    }
                    ++i;
                }
                if( flag_hkana ) continue;
            }
        }

        str2.push_back( str1[ pos ] );
        table_pos.push_back( pos );
        ++pos;
    }
    // 文字列の終端（ヌル文字）の位置を追加する。
    // ヌル文字の位置がないと検索対象の末尾にマッチングしたとき範囲外アクセスが発生する。
    table_pos.push_back( pos );
}


/** @brief UTF-8文字列の正規化(NFKD)
 *
 * @param[in] str1 変換する文字列(ヌル終端)
 * @param[out] str2 出力先
 * @param[out] table_pos 置き換えた文字列の位置(オプション)
 */
void MISC::norm( const char* str1, std::string& str2, std::vector<int>* table_pos )
{
    std::size_t pos = 0;
    char ustr[16];

    while( str1[ pos ] != '\0' ) {

        std::ptrdiff_t bytes = MISC::utf8bytes( str1 + pos );
        if( bytes <= 1 ) {
            str2.push_back( str1[pos] );
            if( table_pos ) table_pos->push_back( pos );
            pos++;
            continue;
        }
        std::strncpy( ustr, str1 + pos, bytes );

        // 異字体は纏める
        constexpr gssize nul_terminated = -1;
        const char32_t next = g_utf8_get_char_validated( str1 + pos + bytes, nul_terminated );
        if( ( 0x180B <= next && next <= 0x180D ) ||
            ( 0xFE00 <= next && next <= 0xFE0F ) ||
            ( 0xE0100 <= next && next <= 0xE01EF ) ) {
            bytes += MISC::utf32toutf8( next, ustr + bytes );
        }

        const std::size_t lng_before = str2.size();
        // 新たに割り当てられた文字列を返すためメモリの開放が必要
        gchar* normalized = g_utf8_normalize( ustr, bytes, G_NORMALIZE_NFKD );
        str2.append( normalized );
        g_free( normalized );
        if( table_pos ) {
            std::size_t n = pos;
            std::generate_n( std::back_inserter( *table_pos ), str2.size() - lng_before, [&n] { return n++; } );
        }

        pos += bytes;
    }
    // 文字列の終端（ヌル文字）の位置を追加する。
    // ヌル文字の位置がないと検索対象の末尾にマッチングしたとき範囲外アクセスが発生する。
    if( table_pos ) {
        table_pos->push_back( pos );
    }
}


//
// selfの先頭部分がstartsと等しいか（ヌル終端文字列バージョン）
// Unicode正規化は行わなずバイト列として比較する
//
// self : 対象の文字列
// starts : 先頭部分
//
bool MISC::starts_with( const char* self, const char* starts )
{
    for( std::size_t i = 0; starts[i] != '\0'; ++i ) {
        if( self[i] == '\0' || self[i] != starts[i] ) return false;
    }
    return true;
}


//
// HTMLからform要素を解析してinput,textarea要素の名前と値を返す
//
std::vector<MISC::FormDatum> MISC::parse_html_form_data( const std::string& html )
{
    JDLIB::Regex regex;
    JDLIB::RegexPattern pat;
    constexpr bool icase = true; // 大文字小文字区別しない
    constexpr bool newline = false;  // . に改行をマッチさせる
    constexpr bool usemigemo = false;
    constexpr bool wchar = false;

    // <input type=(hidden|submit)> or <textarea> のタグを解析して name と value を取得
    const std::string pattern = R"((<input +type=("hidden"|hidden|"submit"|submit) +(name=([^ ]*) +value=([^>]*)|value=([^ ]*) +name=([^>]*))>|<textarea +name=([^ >]*)[^>]*>(.*?)</textarea>))";
    pat.set( pattern, icase, newline, usemigemo, wchar );

    std::vector<MISC::FormDatum> data;
    for( std::size_t offset = 0; ; ++offset){
        std::string name;
        std::string value;

        if( regex.match( pat, html, offset ) ) {
            const std::string name_value = MISC::tolower_str( regex.str( 3 ) );
            if( name_value.rfind( "name=", 0 ) == 0 ) {
                name = MISC::utf8_trim( regex.str( 4 ) );
                value = MISC::utf8_trim( regex.str( 5 ) );
            }
            else if( name_value.rfind( "value=", 0 ) == 0 ) {
                name = MISC::utf8_trim( regex.str( 7 ) );
                value = MISC::utf8_trim( regex.str( 6 ) );
            }
            else {
                name = MISC::utf8_trim( regex.str( 8 ) );
                value = MISC::utf8_trim( regex.str( 9 ) );
            }
        }

        if( name.empty() ) break;

        offset = regex.pos( 0 );

        if( name[ 0 ] == '\"' ) name = MISC::cut_str( name, "\"", "\"" );

        if( value[ 0 ] == '\"' ) value = MISC::cut_str( value, "\"", "\"" );

#ifdef _DEBUG
        std::cout << "offset = " << offset << " "
                  << regex.str( 0 ) << std::endl
                  << "name = " << name << " value = " << value << std::endl;
#endif

        data.push_back( MISC::FormDatum{ std::move( name ), std::move( value ) } );
    }
    return data;
}


// HTMLのform要素から action属性(送信先URLのパス) を取得する
// 2ch互換板に特化して実装しているため他の掲示板で期待した結果を返す保証はない
// 詳細は実装やテストコードを参照
//
std::string MISC::parse_html_form_action( const std::string& html )
{
    const char pattern[] = R"(<form +method=("POST"|POST)[^>]* action="(\.\.)?(/test/(sub)?bbs\.cgi(\?[^"]*)?))";
    const char pattern_same_hierarchy[] = R"(<form +method=("POST"|POST)[^>]* action="\.(/(sub)?bbs\.cgi(\?[^"]*)?))";
    JDLIB::Regex regex;
    constexpr std::size_t offset = 0;
    constexpr bool icase = true; // 大文字小文字区別しない
    constexpr bool newline = false;  // . に改行をマッチさせる
    constexpr bool usemigemo = false;
    constexpr bool wchar = false;

    std::string path;
    if( regex.exec( pattern, html, offset, icase, newline, usemigemo, wchar ) ) {
        path = regex.str( 3 );
    }
    else if( regex.exec( pattern_same_hierarchy, html, offset, icase, newline, usemigemo, wchar ) ) {
        path = "/test" + regex.str( 2 );
    }
    return path;
}


/** @brief HTMLのmeta要素からテキストのエンコーディング(charset)を取得する
 *
 * @param[in] html HTMLの文字列
 * @return 文字エンコーディングを表す文字列、見つからないときは空文字列を返す
 */
std::string MISC::parse_charset_from_html_meta( const std::string& html )
{
    constexpr const char* pattern = R"(<meta\s+(?:http-equiv=["']content-type["']\s+content=["'][^"']*)"
                                    R"(charset=\s*([^"'\s]+)|charset=["']?\s*([^"'>\s]+)))";
    JDLIB::Regex regex;
    constexpr std::size_t offset = 0;
    constexpr bool icase = true;   // 大文字小文字区別しない
    constexpr bool newline = true; // . に改行をマッチさせる
    constexpr bool usemigemo = false;
    constexpr bool wchar = false;

    if( regex.exec( pattern, html, offset, icase, newline, usemigemo, wchar ) ) {
        if( regex.length( 1 ) > 0 ) return regex.str( 1 );
        if( regex.length( 2 ) > 0 ) return regex.str( 2 );
    }
    return {};
}


/** @brief haystack の pos 以降から最初に needle と一致する位置を返す (ASCIIだけignore case)
 *
 * @param[in] haystack 検索する文字列
 * @param[in] needle 探す文字列 (ASCIIだけ大文字小文字を無視)
 * @param[in] pos 検索を開始する位置
 * @return
 * - 最初に needle と一致した位置を返す
 * - 見つからない場合は std::string::npos を返す
 * - needle が空文字列なら pos を返す
 */
std::size_t MISC::ascii_ignore_case_find( const std::string& haystack, std::string_view needle, std::size_t pos )
{
    if( haystack.size() < pos ) return std::string::npos;
    if( needle.empty() ) return pos;

    const char head[3] = { g_ascii_toupper( needle[0] ), g_ascii_tolower( needle[0] ) };
    std::size_t i = pos;
    while( true ) {
        i = haystack.find_first_of( head, i );
        if( i == std::string::npos ) break;
        // ヌル終端文字列が要件なので注意
        if( g_ascii_strncasecmp( haystack.c_str() + i, needle.data(), needle.size() ) == 0 ) break;
        ++i;
    }
    return i;
}
