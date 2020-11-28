// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "miscutil.h"
#include "miscmsg.h"
#include "jdiconv.h"
#include "jdregex.h"
#include "hkana.h"

#include "dbtree/spchar_decoder.h"
#include "dbtree/node.h"

#include <sstream>
#include <cstring>
#include <cstdio>
#include <cstdlib>


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
    std::string str2 = remove_space( str );
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
    std::list < std::string >::iterator it;
    for( it = lists.begin(); it != lists.end(); ++it ) std::cout << "[" << *it << "]" << std::endl;
#endif
    
    return lists;
}

//
// strを空白または "" 単位で区切って list で出力
//
std::list< std::string > MISC::split_line( const std::string& str )
{
    constexpr const char* str_space = u8"\u3000"; // "\xE3\x80\x80" 全角スペース
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
    std::list< std::string >::const_iterator it;    
    for( it = list_in.begin(); it != list_in.end(); ++it ){
        std::string tmp_str = MISC::remove_space( (*it) );
        if( ! tmp_str.empty() ) list_ret.push_back( *it );
    }

    return list_ret;
}


//
// list_inの各行から前後の空白を除いてリストを返す
//
std::list< std::string > MISC::remove_space_from_list( const std::list< std::string >& list_in )
{
    std::list< std::string > list_ret;
    std::list< std::string >::const_iterator it;    
    for( it = list_in.begin(); it != list_in.end(); ++it ){
        std::string tmp_str = MISC::remove_space( (*it) );
        list_ret.push_back( tmp_str );
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
    std::list< std::string >::const_iterator it;    
    for( it = list_in.begin(); it != list_in.end(); ++it ){
        std::string tmp_str = MISC::remove_space( (*it) );
        if( tmp_str.c_str()[ 0 ] != commentchr ) list_ret.push_back( *it );
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
    std::list< std::string > list_tmp;
    std::list< std::string > list_ret;

    list_tmp = MISC::split_line( str_in );
    std::list< std::string >::iterator it = list_tmp.begin();
    for( ; it != list_tmp.end(); ++it ){
        if( !( *it ).empty() ) list_ret.push_back( MISC::recover_quot( ( *it ) ) );
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
    std::list< std::string >::const_iterator it = list_in.begin();
    for( ; it != list_in.end(); ++it ){
        if( ! ( *it ).empty() ) str_out += " \"" + MISC::replace_quot( ( *it ) )  + "\"";
    }

    return str_out;
}



//
// strの前後の空白削除
//
std::string MISC::remove_space( const std::string& str )
{
    constexpr const char* str_space = u8"\u3000"; // "\xE3\x80\x80" 全角スペース
    constexpr size_t lng_space = 3;

    size_t lng = str.length();
    
    if( lng == 0 ) return str;
    if( str.find( ' ' ) == std::string::npos ) return str;

    // 前
    size_t i = 0;
    while( i < lng ){

        // 半角
        if( str[ i ] == ' ' ) ++i;

        // 全角
        else if( str[ i ] == str_space[ 0 ] &&
                 str[ i +1 ] == str_space[ 1 ] &&
                 str[ i +2 ] == str_space[ 2 ] ) i += lng_space;
        else break;
    }
    if (i >= lng) return "";

    // 後
    size_t i2 = lng -1;
    while( 1 ){

        // 半角
        if( str[ i2 ] == ' ' ) --i2;

        // 全角
        else if( i2 +1 >= lng_space &&
                 str[ i2 - lng_space +1 ] == str_space[ 0 ] &&
                 str[ i2 - lng_space +2 ] == str_space[ 1 ] &&
                 str[ i2 - lng_space +3 ] == str_space[ 2 ] ) i2 -= lng_space;
        else break;
    }

    return str.substr( i, i2 - i + 1 );
}


//
// str前後の改行、タブ、スペースを削除
//
std::string MISC::remove_spaces( const std::string& str )
{
    if( str.empty() ) return std::string();

    size_t l = 0, r = str.length();

    while( l < r
         && ( str[l] == '\n'
           || str[l] == '\r'
           || str[l] == '\t'
           || str[l] == ' ' ) ) ++l;

    // 最後の文字の位置は文字数より1少ない
    size_t p = r - 1;
    while( p > 0
         && ( str[p] == '\n'
           || str[p] == '\r'
           || str[p] == '\t'
           || str[p] == ' ' ) ){ --p; --r; }

    return str.substr( l, r - l );
}



//
// str1からstr2で示された文字列を除く
//
std::string MISC::remove_str( const std::string& str1, const std::string& str2 )
{
    return MISC::replace_str( str1, str2, "" );
}


//
// start 〜 end の範囲をstrから取り除く ( /* コメント */ など )
//
std::string MISC::remove_str( const std::string& str, const std::string& start, const std::string& end )
{
    std::string str_out = str;

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


//
// str1, str2 に囲まれた文字列を切り出す
//
std::string MISC::cut_str( const std::string& str, const std::string& str1, const std::string& str2 )
{
    size_t i = str.find( str1 );
    if( i == std::string::npos ) return std::string();
    i += str1.length();
    size_t i2 = str.find( str2, i );
    if( i2 == std::string::npos ) return std::string();
    
    return str.substr( i, i2 - i );
}


//
// str1 を str2 に置き換え
//
std::string MISC::replace_str( const std::string& str, const std::string& str1, const std::string& str2 )
{
    size_t i, pos = 0;
    if( ( i = str.find( str1 , pos ) ) == std::string::npos ) return str;

    std::string str_out;
    str_out.reserve( str.length() );

    do {
        str_out.append( str, pos, ( i - pos ) );
        str_out.append( str2 );
        pos = i + str1.length();
    }
    while( ( i = str.find( str1 , pos ) ) != std::string::npos );

    str_out.append( str, pos, str.length() );
    return str_out;
}


//
// list_inから str1 を str2 に置き換えてリストを返す
//
std::list< std::string > MISC::replace_str_list( const std::list< std::string >& list_in,
                                                 const std::string& str1, const std::string& str2 )
{
    std::list< std::string > list_out;
    std::list< std::string >::const_iterator it = list_in.begin();
    for( ; it != list_in.end(); ++it ) list_out.push_back( replace_str( *it, str1, str2 ) );
    return list_out;
}


//
// str_in に含まれる改行文字を replace に置き換え
//
std::string MISC::replace_newlines_to_str( const std::string& str_in, const std::string& replace )
{
    if( str_in.empty() || replace.empty() ) return str_in;

    std::string str_out;
    str_out.reserve( str_in.length() );

    size_t pos = 0, found = 0;
    while( ( found = str_in.find_first_of( "\r\n", pos ) ) != std::string::npos )
    {
        str_out.append( str_in, pos, ( found - pos ) );
        str_out.append( replace );

        pos = found + 1;

        if( str_in[ found ] == '\r' && str_in[ found + 1 ] == '\n' ) ++pos;
    }

    str_out.append( str_in, pos, str_in.length() );

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
// str 中に含まれている str2 の 数を返す
//
int MISC::count_str( const std::string& str, const std::string& str2  )
{
    int count = 0;
    size_t found, pos = 0;

    while( ( found = str.find( str2, pos ) ) != std::string::npos )
    {
        ++count;
        pos = found + 1;
    }

    return count;
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

            const unsigned char in2 = (* ( str +1 ));
            const unsigned char in3 = (* ( str +2 ));

            // utf-8
            if( in == 0xef && in2 == 0xbc && ( 0x90 <= in3 && in3 <= 0x99 ) ){
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
    std::ostringstream comment;

    std::list < int >::const_iterator it = list_num.begin();

    bool comma = false;
    int num_from = *it;
    int num_to = -1;
    int i = 0;
    for(;;){

        ++i;
        ++it;
        const int num = *it;
        if( num_from + i != num || it == list_num.end() ){

            if( comma ) comment << ",";
            comment << num_from;
            if( num_to != -1 ) comment << "-" << num_to;
            num_from = num;
            num_to = -1;
            i = 0;
            comma = true;

            if( it == list_num.end() ) break;
        }
        else num_to = num;
    }

    return comment.str();
}



//
// 16進数表記文字をバイナリに変換する( 例 "E38182" -> 0xE38182 )
//
// 出力 : char_out 
// 戻り値: 変換に成功した chr_in のバイト数
//
size_t MISC::chrtobin( const char* chr_in, char* chr_out )
{
    if( ! chr_in ) return 0;

    const size_t chr_in_length = strlen( chr_in );

    size_t a, b;
    for( a = 0, b = a; a < chr_in_length; ++a )
    {
        unsigned char chr = chr_in[a];

        chr_out[b] <<= 4;

        // 0(0x30)〜9(0x39)
        if( (unsigned char)( chr - 0x30 ) < 10 ) chr_out[b] |= chr - 0x30;
        // A(0x41)〜F(0x46)
        else if( (unsigned char)( chr - 0x41 ) < 6 ) chr_out[b] |= chr - 0x37;
        // a(0x61)〜f(0x66)
        else if( (unsigned char)( chr - 0x61 ) < 6 ) chr_out[b] |= chr - 0x57;
        // その他
        else break;

        if( a % 2 != 0 ) ++b;
    }

    return a;
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
        MISC::utf8toucs2( outstr.c_str()+pos, byte );
        if( byte > 1 ) lng_str += 2;
        else ++lng_str;
        if( lng_str >= maxsize ) break;
    }

    // カットしたら"..."をつける
    if( pos != outstr_length ) outstr = outstr.substr( 0, pos ) + "...";

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


//
// HTMLエスケープ
//
// include_url : URL中でもエスケープする( デフォルト = true )
//
std::string MISC::html_escape( const std::string& str, const bool include_url )
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
        if( ! include_url )
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

        // include_url = false でURL中ならエスケープしない
        if( is_url ) str_out += tmpchar;
        else if( tmpchar == '&' )
        {
            const int bufsize = 64;
            char out_char[ bufsize ];
            int n_in, n_out;
            const int type = DBTREE::decode_char( str.c_str() + pos, n_in, out_char, n_out, false );
            if( type == DBTREE::NODE_NONE ) str_out += "&amp;";
            else str_out += tmpchar;
        }
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


//
// HTMLアンエスケープ
//
std::string MISC::html_unescape( const std::string& str )
{
    if( str.empty() ) return str;
    if( str.find( '&' ) == std::string::npos ) return str;

    std::string str_out;
    const size_t str_length = str.length();

    for( size_t pos = 0; pos < str_length; ++pos ){

        const int bufsize = 64;
        char out_char[ bufsize ];
        int n_in, n_out;
        DBTREE::decode_char( str.c_str() + pos, n_in, out_char, n_out, false );

        if( n_out ){
            str_out += out_char;
            pos += n_in -1;
        }
        else str_out += str.c_str()[ pos ];
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
    unsigned char c = (unsigned char)(*str_in);

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



//
// URLデコード
//
std::string MISC::url_decode( const std::string& url )
{
    if( url.empty() ) return std::string();

    const size_t url_length = url.length();

    std::vector< char > decoded( url_length + 1, '\0' );

    unsigned int a, b;
    for( a = 0, b = a; a < url_length; ++a, ++b )
    {
        if( url[a] == '%' && ( a + 2 < url_length ) )
        {
            char src[3] = { url[ a + 1 ], url[ a + 2 ], '\0' };
            char tmp[3] = { '\0', '\0', '\0' };

            if( chrtobin( src, tmp ) == 2 )
            {
                // '%4A' など、2文字が変換できていること
                decoded[b] = *tmp;
                a += 2;
            }
            else
            {
                // 変換失敗は、単なる '%' 文字として扱う
                decoded[b] = url[a];
            }
        }
        else if( url[a] == '+' )
        {
            decoded[b] = ' ';
        }
        else
        {
            decoded[b] = url[a];
        }
    }

    return decoded.data();
}


//
// url エンコード
//
std::string MISC::url_encode( const char* str, const size_t n )
{
    if( str[ n ] != '\0' ){
        ERRMSG( "url_encode : invalid input." );
        return std::string();
    }

    std::string str_encoded;
    
    for( size_t i = 0; i < n; i++ ){
        
        unsigned char c = str[ i ];
        const int tmplng = 16;
        char str_tmp[ tmplng ];
        
        if( ! ( 'a' <= c && c <= 'z' ) &&
            ! ( 'A' <= c && c <= 'Z' ) &&
            ! ( '0' <= c && c <= '9' ) &&            
            ( c != '*' ) &&
            ( c != '-' ) &&
            ( c != '.' ) &&
            ( c != '@' ) &&
            ( c != '_' )){

            snprintf( str_tmp, tmplng , "%%%02x", c );
        }
        else {
            str_tmp[ 0 ] = c;
            str_tmp[ 1 ] = '\0';
        }

        str_encoded += str_tmp;
    }

    return str_encoded;
}


std::string MISC::url_encode( const std::string& str )
{
    return url_encode( str.c_str(), str.length() );
}


//
// 文字コード変換して url エンコード
//
// str は UTF-8 であること
//
std::string MISC::charset_url_encode( const std::string& str, const std::string& charset )
{
    if( charset.empty() || charset == "UTF-8" ) return MISC::url_encode( str.c_str(), str.length() );

    const std::string str_enc = MISC::Iconv( str, charset, "UTF-8" );
    return  MISC::url_encode( str_enc.c_str(), str_enc.length() );
}


//
// 文字コード変換して url エンコード
//
// ただし半角スペースのところを+に置き換えて区切る
//
std::string MISC::charset_url_encode_split( const std::string& str, const std::string& charset )
{
    std::list< std::string > list_str = MISC::split_line( str );
    std::list< std::string >::iterator it = list_str.begin();
    std::string str_out;
    for( ; it != list_str.end(); ++it ){

        if( it != list_str.begin() ) str_out += "+";
        str_out += MISC::charset_url_encode( *it, charset );
    }

    return str_out;
}


//
// BASE64
//
std::string MISC::base64( const std::string& str )
{
    char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    int lng = str.length();

    std::string out;
    out.reserve( lng * 2 );

    std::string data = str + "\0\0\0\0";

    for( int i = 0; i < lng; i += 3 ){

        unsigned char* cstr = (unsigned char*)( data.c_str() + i );
        unsigned char key[ 4 ];

        key[ 0 ] = (*cstr) >> 2;
        key[ 1 ] = ( ( (*cstr) << 4 ) + ( (*(cstr+1)) >> 4 ) );
        key[ 2 ] = ( ( (*(cstr+1)) << 2 ) + ( (*(cstr+2)) >> 6 ) );
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
// 文字コードを coding_from から coding_to に変換
//
// 遅いので連続的な処理が必要な時は使わないこと
//
std::string MISC::Iconv( const std::string& str, const std::string& coding_to, const std::string& coding_from )
{
    if( coding_from == coding_to ) return str;

    std::string str_bk = str;

    JDLIB::Iconv* libiconv = new JDLIB::Iconv( coding_to, coding_from );
    int byte_out;

    std::string str_enc = libiconv->convert( &*str_bk.begin(), str_bk.size(), byte_out );

    delete libiconv;

    return str_enc;
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

    // UCS2の2バイトの範囲でデコードするので最大65535
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
static int transform_7f_9f( int raw_point )
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

//
// コードポイントが数値文字参照としてはエラーなら規定の値へ変換する
// 例えばサロゲートペアは'REPLACEMENT CHARACTER' (U+FFFD)を返す
//
// 参考文献 : Numeric character reference end state (HTML 5.3)
//            https://www.w3.org/TR/html53/syntax.html#numeric-character-reference-end-state
//
static int sanitize_numeric_character_reference( int raw_point )
{
    // NOTE: 記号や絵文字を速やかに処理できるよう順番が組まれている

    bool parse_error = false;

    // 基本多言語面(BMP)をおおまかにチェック
    if( 0x009F < raw_point && raw_point < 0xD800 ) return raw_point;
    // 特定のbitパターンの非文字と符号空間の範囲をチェック
    else if( ( raw_point & 0xFFFE ) == 0xFFFE || raw_point > 0x10FFFF ) parse_error = true;
    // bitパターンを除いたらBMPの一部と追加多言語面(SMP)以降をチェック
    else if( 0xFDEF < raw_point ) return raw_point;
    // サロゲートペアはエラー
    else if( 0xD800 <= raw_point && raw_point <= 0xDFFF ) parse_error = true;

    // 基本ラテン文字をチェック
    else if( 0x001F < raw_point && raw_point < 0x007F ) return raw_point;
    // 特定の変換が必要なコードポイントをチェック
    else if( 0x007F <= raw_point && raw_point <= 0x009F ) return transform_7f_9f( raw_point );
    // 最後に制御文字と非文字をチェック
    // サロゲートペアは他の値より入力される可能性が高いので処理を優先している
    else if( raw_point <= 0x0008 // Control character
            || raw_point == 0x000B // Control character (Vertical tab)
            || ( 0x000D <= raw_point && raw_point <= 0x001F ) // Control character
            || ( 0xFDD0 <= raw_point // && raw_point <= 0xFDEF の境界は上でチェックしているので不要
                ) // Noncharacters
            ) {
        parse_error = true;
    }

    if( parse_error ) {
#ifdef _DEBUG
        std::cout << "Parse error for numeric character reference... " << raw_point << std::endl;
#endif
        return 0xFFFD; // REPLACEMENT CHARACTER
    }
    return raw_point;
}

//
// 「&#数字;」形式の数字参照文字列を数字(int)に変換する
//
// 最初に MISC::spchar_number_ln() を呼び出して offset と lng を取得すること
//
// in_char: 入力文字列、in_char[0] == "&" && in_char[1] == "#" であること
// offset : spchar_number_ln() の戻り値
// lng : spchar_number_ln() の戻り値
//
// 戻り値 : 「&#数字;」の中の数字(int型)
//
int MISC::decode_spchar_number( const char* in_char, const int offset, const int lng )
{
    char str_num[ 16 ];

    memcpy( str_num, in_char + offset, lng );
    str_num[ lng ] = '\0';

#ifdef _DEBUG
    std::cout << "MISC::decode_spchar_number offset = " << offset << " lng = " << lng << " str = " << str_num << std::endl;
#endif

    int num = 0;
    if( offset == 2 ) num = atoi( str_num );
    else num = strtol( str_num, nullptr, 16 );

    return sanitize_numeric_character_reference( num );
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

            const int num = MISC::decode_spchar_number( str.c_str()+i, offset, lng );

            char out_char[ 64 ];
            const int n_out = MISC::ucs2toutf8( num, out_char );
            if( ! n_out ){
                str_out += str[ i ];
                continue;
            }

            for( int j = 0; j < n_out; ++j ) str_out += out_char[ j ];
            i += offset + lng;
        }
        else str_out += str[ i ];
    }

    return str_out;
}


//
// utf-8 -> ucs2 変換
//
// 入力 : utfstr 入力文字 (UTF-8)
//
// 出力 :  byte  長さ(バイト) utfstr が ascii なら 1, UTF-8 なら 2 or 3 or 4 を入れて返す
//
// 戻り値 : ucs2
//
int MISC::utf8toucs2( const char* utfstr, int& byte )
{
    int ucs2 = 0;
    byte = 0;

    if( utfstr[ 0 ] == '\0' ) return '\0';
    
    else if( ( ( unsigned char ) utfstr[ 0 ] & 0xf0 ) == 0xe0 ){
        byte = 3;
        ucs2 = utfstr[ 0 ] & 0x0f;
        ucs2 = ( ucs2 << 6 ) + ( utfstr[ 1 ] & 0x3f );
        ucs2 = ( ucs2 << 6 ) + ( utfstr[ 2 ] & 0x3f );        
    }

    else if( ( ( unsigned char ) utfstr[ 0 ] & 0x80 ) == 0 ){ // ascii
        byte = 1;
        ucs2 =  utfstr[ 0 ];
    }

    else if( ( ( unsigned char ) utfstr[ 0 ] & 0xe0 ) == 0xc0 ){
        byte = 2;
        ucs2 = utfstr[ 0 ] & 0x1f;
        ucs2 = ( ucs2 << 6 ) + ( utfstr[ 1 ] & 0x3f );
    }

    else if( ( ( unsigned char ) utfstr[ 0 ] & 0xf8 ) == 0xf0 ){
        byte = 4;
        ucs2 = utfstr[ 0 ] & 0x07;
        ucs2 = ( ucs2 << 6 ) + ( utfstr[ 1 ] & 0x3f );
        ucs2 = ( ucs2 << 6 ) + ( utfstr[ 2 ] & 0x3f );
        ucs2 = ( ucs2 << 6 ) + ( utfstr[ 3 ] & 0x3f );        
    }

    // 不正なUTF8
    else {
        byte = 1;
        ucs2 =  utfstr[ 0 ];
        ERRMSG( "MISC::utf8toucs2 : invalid code = " + std::to_string( ucs2 ) );
    }

    return ucs2;
}




//
// ucs2 -> utf8 変換
//
// 出力 : utfstr 変換後の文字
//
// 戻り値 : バイト数
//
int MISC::ucs2toutf8( const int ucs2,  char* utfstr )
{
    int byte = 0;

    if( ucs2 <= 0x7f ){ // ascii
        byte = 1;
        utfstr[ 0 ] = ucs2;
    }

    else if( ucs2 <= 0x07ff ){
        byte = 2;
        utfstr[ 0 ] = ( 0xc0 ) + ( ucs2 >> 6 );
        utfstr[ 1 ] = ( 0x80 ) + ( ucs2 & 0x3f );
    }

    else if( ucs2 <= 0xffff){
        byte = 3;
        utfstr[ 0 ] = ( 0xe0 ) + ( ucs2 >> 12 );
        utfstr[ 1 ] = ( 0x80 ) + ( ( ucs2 >>6 ) & 0x3f );
        utfstr[ 2 ] = ( 0x80 ) + ( ucs2 & 0x3f );
    }

    else{
        byte = 4;
        utfstr[ 0 ] = ( 0xf0 ) + ( ucs2 >> 18 );
        utfstr[ 1 ] = ( 0x80 ) + ( ( ucs2 >>12 ) & 0x3f );
        utfstr[ 2 ] = ( 0x80 ) + ( ( ucs2 >>6 ) & 0x3f );
        utfstr[ 3 ] = ( 0x80 ) + ( ucs2 & 0x3f );
    }

    utfstr[ byte ] = 0;
    return byte;
}


//
// ucs2 の種類
//
int MISC::get_ucs2mode( const int ucs2 )
{
    if( ucs2 >= 0x0000 && ucs2 <= 0x007f ) return UCS2MODE_BASIC_LATIN;
    if( ucs2 >= 0x3040 && ucs2 <= 0x309f ) return UCS2MODE_HIRA;
    if( ucs2 >= 0x30a0 && ucs2 <= 0x30ff ) return UCS2MODE_KATA;

    return UCS2MODE_OTHER;
}

//
// WAVEDASHなどのWindows系UTF-8文字をUnix系文字と相互変換
//
std::string MISC::utf8_fix_wavedash( const std::string& str, const int mode )
{
    // WAVE DASH 問題
    const size_t size = 4;
    const unsigned char Win[size][4] = {
        { 0xef, 0xbd, 0x9e, '\0' }, // FULLWIDTH TILDE (U+FF5E)
        { 0xe2, 0x80, 0x95, '\0' }, // HORIZONTAL BAR (U+2015)
        { 0xe2, 0x88, 0xa5, '\0' }, // PARALLEL TO (U+2225)
        { 0xef, 0xbc, 0x8d, '\0' }  // FULLWIDTH HYPHEN-MINUS (U+FF0D)
    };
    const unsigned char Unix[size][4] = {
        { 0xe3, 0x80, 0x9c, '\0' }, // WAVE DASH (U+301C)
        { 0xe2, 0x80, 0x94, '\0' }, // EM DASH(U+2014)
        { 0xe2, 0x80, 0x96, '\0' }, // DOUBLE VERTICAL LINE (U+2016)
        { 0xe2, 0x88, 0x92, '\0' }  // MINUS SIGN (U+2212)
    };
    
    std::string ret(str);

    if( mode == WINtoUNIX ){

        for( size_t i = 0; i < ret.length(); i++ ) {
            for( size_t s = 0; s < size; s++ ) {
                if( ret[ i ] != (char)Win[ s ][ 0 ] || ret[ i+1 ] != (char)Win[ s ][ 1 ] || ret[ i+2 ] != (char)Win[ s ][ 2 ] )
                    continue;
                for( size_t t = 0; t < 3; t++ )
                    ret[ i+t ] = (char)Unix[ s ][ t ];
                i += 2;
                break;
            }
        }

    }else{
   
        for( size_t i = 0; i < ret.length(); i++ ) {
            for( size_t s = 0; s < size; s++ ) {
                if( ret[ i ] != (char)Unix[ s ][ 0 ] || ret[ i+1 ] != (char)Unix[ s ][ 1 ] || ret[ i+2 ] != (char)Unix[ s ][ 2 ] )
                    continue;
                for( size_t t = 0; t < 3; t++ )
                    ret[ i+t ] = (char)Win[ s ][ t ];
                i += 2;
                break;
            }
        }
    }

    return ret;
}


//
// str を大文字化
//
std::string MISC::toupper_str( const std::string& str )
{
    std::string str_out;
    const size_t str_length = str.length();

    for( size_t i = 0; i < str_length ; ++i ) str_out += toupper( str[ i ] );

    return str_out;
}


//
// list 内のアイテムを全部大文字化
//
std::list< std::string > MISC::toupper_list( const std::list< std::string >& list_str )
{
    std::list< std::string > list_out;
    std::list< std::string >::const_iterator it = list_str.begin();
    for( ; it != list_str.end() ; ++it ) list_out.push_back( MISC::toupper_str( *it ) );

    return list_out;
}



//
// str を小文字化
//
std::string MISC::tolower_str( const std::string& str )
{
    std::string str_out;
    const size_t str_length = str.length();

    for( size_t i = 0; i < str_length; ++i ) str_out += tolower( str[ i ] );

    return str_out;
}



//
// path からホスト名だけ取り出す
//
// protocol = false のときはプロトコルを除く
//
std::string MISC::get_hostname( const std::string& path, bool protocol )
{
    int lng = 0;
    if( path.find( "http://" ) == 0 ) lng = strlen( "http://" );
    else if( path.find( "https://" ) == 0 ) lng = strlen( "https://" );
    else if( path.find( "ftp://" ) == 0 ) lng = strlen( "ftp://" );
    if( !lng ) return std::string();

    int pos = 0;
    if( ! protocol ) pos = lng;

    size_t i = path.find( "/", lng ); 

    if( i == std::string::npos ) return path.substr( pos );

    return path.substr( pos, i - pos );
}



//
// path からファイル名だけ取り出す
//
std::string MISC::get_filename( const std::string& path )
{
    if( path.empty() ) return std::string();

    size_t i = path.rfind( "/" );
    if( i == std::string::npos ) return path;

    return path.substr( i+1 );
}



//
// path からファイル名を除いてディレクトリだけ取り出す
//
std::string MISC::get_dir( const std::string& path )
{
    if( path.empty() ) return std::string();

    size_t i = path.rfind( "/" );
    if( i == std::string::npos ) return std::string();

    return path.substr( 0, i+1 );
}



//
// 文字数を限定して環境変数の値を返す
//
std::string MISC::getenv_limited( const char *name, const size_t size )
{
    if( ! name || ! getenv( name ) ) return std::string();

    std::string env = getenv( name );
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

std::vector< std::string > MISC::recover_path( std::vector< std::string > list_str )
{
    return list_str;
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

                const unsigned char in2 = * ( str + 1 );
                const unsigned char in3 = * ( str + 2 );

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
            if( name_value.compare( 0, 5, "name=" ) == 0 ) {
                name = MISC::remove_space( regex.str( 4 ) );
                value = MISC::remove_space( regex.str( 5 ) );
            }
            else if( name_value.compare( 0, 6, "value=" ) == 0 ) {
                name = MISC::remove_space( regex.str( 7 ) );
                value = MISC::remove_space( regex.str( 6 ) );
            }
            else {
                name = MISC::remove_space( regex.str( 8 ) );
                value = MISC::remove_space( regex.str( 9 ) );
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
