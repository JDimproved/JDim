// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "miscutil.h"
#include "miscmsg.h"
#include "jdiconv.h"
#include "jdregex.h"

#include <sstream>


//
// str を "\n" ごとに区切ってlistにして出力
//
std::list< std::string > MISC::get_lines( const std::string& str ){
        
    std::list< std::string > lines;
    size_t i = 0, i2 = 0, r = 0;
    while ( ( i2 = str.find( "\n", i ) ) != std::string::npos ){
        r = 0;
        if( str[ i2 - 1 ] == '\r' ) r = 1;
        if( i2 - i > 0 ){
            std::string str_tmp = str.substr( i, i2 - i - r );
            lines.push_back( str_tmp );
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
    size_t pos = 0, length = 0;
    std::string str2 = remove_space( str );
    const char* data = str2.c_str();
    if( data[ 0 ] != '(' ) return lists;

    pos = 1;

    while( data[ pos ] != '\0' ){

        // 空白削除
        while( data[ pos ] == ' ' && data[ pos ] != '\0' ) ++pos;
        if( data[ pos ] == '\0' ) break;

        length = 1;

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
    std::string str_space = "　";
    size_t lng_space = str_space.length();
    bool dquote;

    std::list< std::string > list_str;

    size_t i = 0, i2 = 0, lng = str.length();
    for(;;){

        // 空白を取る
        while( 1 ){

            // 半角
            if( str[ i ] == ' ' ) ++i;

            // 全角
            else if( str[ i ] == str_space[ 0 ] &&
                     str[ i +1 ] == str_space[ 1 ] &&
                     ( lng_space == 2 || str[ i +2 ] == str_space[ 2 ] ) ) i += lng_space;

            else break;
        }

        // " から始まる ( \"は除く )
        dquote = false;
        if( str[ i ] == '\"' && str[ i -1 ] != '\\' ){
            dquote = true;
            ++i;
        }

        // 空白か " を探す
        i2 = i;
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
                         ( lng_space == 2 || str[ i2 +2 ] == str_space[ 2 ] ) ){
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
std::list< std::string > MISC::StringTokenizer( const std::string& str, char delim )
{
    std::list< std::string > list_str;

    size_t i = 0, i2 = 0, lng = str.length();
    for(;;){

        while( i2 < lng && str[ i2++ ] != delim );
        int tmp = ( str[ i2-1 ] == delim || str[ i2 -1 ] == '\n' ) ? 1 : 0;
        if( i2 - i ) list_str.push_back( str.substr( i, i2 - i - tmp ) );
        if( i2 >= lng ) break;
        i = i2;
    }
    
    return list_str;
}



//
// list_inから空白行を除いてリストを返す
//
std::list< std::string > MISC::remove_nullline_from_list( std::list< std::string >& list_in )
{
    std::list< std::string > list_ret;
    std::list< std::string >::iterator it;    
    for( it = list_in.begin(); it != list_in.end(); ++it ){
        std::string tmp_str = MISC::remove_space( (*it) );
        if( ! tmp_str.empty() ) list_ret.push_back( *it );
    }

    return list_ret;
}


//
// list_inの各行から前後の空白を除いてリストを返す
//
std::list< std::string > MISC::remove_space_from_list( std::list< std::string >& list_in )
{
    std::list< std::string > list_ret;
    std::list< std::string >::iterator it;    
    for( it = list_in.begin(); it != list_in.end(); ++it ){
        std::string tmp_str = MISC::remove_space( (*it) );
        list_ret.push_back( tmp_str );
    }

    return list_ret;
}


//
// list_inからコメント行(#)を除いてリストを返す
//
std::list< std::string > MISC::remove_commentline_from_list( std::list< std::string >& list_in )
{
    const char commentchr = '#';

    std::list< std::string > list_ret;
    std::list< std::string >::iterator it;    
    for( it = list_in.begin(); it != list_in.end(); ++it ){
        std::string tmp_str = MISC::remove_space( (*it) );
        if( tmp_str.c_str()[ 0 ] != commentchr ) list_ret.push_back( *it );
    }

    return list_ret;
}



//
// strからコメントの範囲を取り除く ( /* コメント */ など )
//
std::string MISC::remove_commentrange_from_str( std::string& str, const std::string& start, const std::string& end )
{
    size_t start_pos = 0, l_pos = 0, r_pos = 0;

    while( ( l_pos = str.find( start, start_pos ) ) != std::string::npos &&
            ( r_pos = str.find( end, l_pos + start.length() ) ) != std::string::npos )
    {
        str.erase( l_pos, r_pos - l_pos + end.length() );
        start_pos = r_pos + end.length();
    }

    return str;
}



//
// 空白とカンマで区切られた str_in の文字列をリストにして出力
//
// \"は " に置換される
//
// (例)  "aaa" "bbb" "\"ccc\""  → aaa と bbb と "ccc"
//
std::list< std::string > MISC::strtolist( std::string& str_in )
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
// list_in の文字列リストを空白とカンマで区切ってストリングにして出力
//
// "は \" に置換される
//
// (例)  "aaa" "bbb" "\"ccc\""
//
std::string MISC::listtostr( std::list< std::string >& list_in )
{
    std::string str_out;
    std::list< std::string >::iterator it = list_in.begin();
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
    std::string str_space = "　";
    size_t lng_space = str_space.length();

    std::string str_out;
    size_t lng = str.length();
    
    if( lng == 0 ) return str;
    if( str.find( " " ) == std::string::npos ) return str;

    // 前
    size_t i = 0;
    while( 1 ){

        // 半角
        if( str[ i ] == ' ' ) ++i;

        // 全角
        else if( str[ i ] == str_space[ 0 ] &&
                 str[ i +1 ] == str_space[ 1 ] &&
                 ( lng_space == 2 || str[ i +2 ] == str_space[ 2 ] ) ) i += lng_space;
        else break;
    }

    // 後
    size_t i2 = lng -1;
    while( 1 ){

        // 半角
        if( str[ i2 ] == ' ' ) --i2;

        // 全角
        else if( str[ i2 - lng_space +1 ] == str_space[ 0 ] &&
                 str[ i2 - lng_space +2 ] == str_space[ 1 ] &&
                 ( lng_space == 2 || str[ i2 - lng_space +3 ] == str_space[ 2 ] ) ) i2 -= lng_space;
        else break;
    }

    str_out = str.substr( i, i2 - i + 1 );
    
    return str_out;
}


//
// str1からstr2で示された文字列を除く
//
std::string MISC::remove_str( const std::string& str1, const std::string& str2 )
{
    return MISC::replace_str( str1, str2, "" );
}


//
// 正規表現を使ってstr1からqueryで示された文字列を除く
//
std::string MISC::remove_str_regex( const std::string& str1, const std::string& query )
{
    JDLIB::Regex regex;
    if( ! regex.exec( query, str1 ) ) return std::string();
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


// str1 を str2 に置き換え
std::string MISC::replace_str( const std::string& str, const std::string& str1, const std::string& str2 )
{
    std::string str_out;
    size_t i, pos = 0;
    while( ( i = str.find( str1 , pos ) ) != std::string::npos ){

        str_out += str.substr( pos, ( i - pos ) ) + str2;
        pos += ( i - pos ) + str1.length();
    }

    str_out += str.substr( pos );
    return str_out;
}


// list_inから str1 を str2 に置き換えてリストを返す
std::list< std::string > MISC::replace_str_list( std::list< std::string >& list_in,
                                                 const std::string& str1, const std::string& str2 )
{
    std::list< std::string > list_out;
    std::list< std::string >::iterator it = list_in.begin();
    for( ; it != list_in.end(); ++it ) list_out.push_back( replace_str( *it, str1, str2 ) );
    return list_out;
}



// " を \" に置き換え
std::string MISC::replace_quot( const std::string& str )
{
    return MISC::replace_str( str, "\"", "\\\"" );
}


// \" を " に置き換え
std::string MISC::recover_quot( const std::string& str )
{
    return MISC::replace_str( str, "\\\"", "\"" );
}


// str 中に含まれている str2 の 数を返す
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
int MISC::str_to_uint( const char* str, unsigned int& dig, unsigned int& n )
{
    int out = 0;
    dig = 0;
    n = 0;
    while( *str != '\0' ){

        unsigned char in = (*str);
        
        if( '0' <=  in && in <= '9' ){

            out = out*10 + ( in - '0' );
            ++dig;
            ++str;
            ++n;
        }

        else{
            // utf-8
            unsigned char in2 = (* ( str +1 ));
            unsigned char in3 = (* ( str +2 ));
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



// 数字　-> 文字変換
std::string MISC::itostr( int n )
{
    std::ostringstream ss;
    ss << n;

    return ss.str();
}



// listで指定した数字を文字に変換
std::string MISC::intlisttostr( std::list< int >& list_num )
{
    std::ostringstream comment;

    std::list < int >::iterator it = list_num.begin();

    bool comma = false;
    int num_from = *it;
    int num_to = -1;
    ++it;
    for(;;){

        int num = *it;
        if( num_from + 1 != num || it == list_num.end() ){
                
            if( comma ) comment << ",";
            comment << num_from;
            if( num_to != -1 ) comment << "-" << num_to;
            num_from = num;
            num_to = -1;
            comma = true;

            if( it == list_num.end() ) break;
        }
        else num_to = num;

        ++it;
    }

    return comment.str();
}



// strが半角でmaxsize文字を超えたらカットして後ろに...を付ける
std::string MISC::cut_str( const std::string& str, unsigned int maxsize )
{
    std::string outstr = str;
    unsigned int pos, lng_str;
    int byte = 0;

    for( pos = 0, lng_str = 0; pos < outstr.length(); pos += byte ){
        MISC::utf8toucs2( outstr.c_str()+pos, byte );
        if( byte > 1 ) lng_str += 2;
        else ++lng_str;
        if( lng_str >= maxsize ) break;
    }

    // カットしたら"..."をつける
    if( pos != outstr.length() ) outstr = outstr.substr( 0, pos ) + "...";

    return outstr;
}



//
// url エンコード
//
std::string MISC::url_encode( const char* str, size_t n )
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

            snprintf( str_tmp, tmplng , "\%%%02x", c );
        }
        else {
            str_tmp[ 0 ] = c;
            str_tmp[ 1 ] = '\0';
        }

        str_encoded += str_tmp;
    }

    return str_encoded;
}



//
// 文字コード変換して url エンコード
//
// str は UTF-8 であること
//
std::string MISC::charset_url_encode( const std::string& str, const std::string& charset )
{
    if( charset.empty() ) return MISC::url_encode( str.c_str(), str.length() );

    std::string str_enc = MISC::Iconv( str, "UTF-8", charset );
    std::string str_encoded = MISC::url_encode( str_enc.c_str(), strlen( str_enc.c_str() ) );

    return str_encoded;
}


//
// BASE64
//
std::string MISC::base64( const std::string& str )
{
    char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string out;
    int lng = str.length();
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
        out = out.substr( 0, out.length()-2 ) + "==";
    }
    else if( lng % 3 == 2 ){
        out = out.substr( 0, out.length()-1 ) + "=";
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
std::string MISC::Iconv( const std::string& str, const std::string& coding_from, const std::string& coding_to )
{
    if( coding_from == coding_to ) return str;

    char* str_bk = ( char* ) malloc( str.length() + 64 );
    strcpy( str_bk, str.c_str() );

    JDLIB::Iconv* libiconv = new JDLIB::Iconv( coding_from, coding_to );
    int byte_out;

    std::string str_enc = libiconv->convert( str_bk, strlen( str_bk ), byte_out );

    delete libiconv;
    free( str_bk );

    return str_enc;
}



//
// utf-8 -> ucs2 変換
//
// utfstr : 入力文字 (UTF-8)
// byte   : 長さ(バイト) utfstr が ascii なら 1, UTF-8 なら 2 or 3 を入れて返す
//
// 戻り値 : ucs2
//
int MISC::utf8toucs2( const char* utfstr, int& byte )
{
    int ucs2 = 0;
    byte = 0;

    if( utfstr[ 0 ] == '\0' ) return ucs2;
    
    if( ( ( unsigned char ) utfstr[ 0 ] & 0x80 ) == 0 ){ // ascii
        byte = 1;
        ucs2 =  utfstr[ 0 ];
    }

    else if( ( ( unsigned char ) utfstr[ 0 ] & 0x20 ) == 0 ){
        byte = 2;
        ucs2 = utfstr[ 0 ] & 0x1f;
        ucs2 = ( ucs2 << 6 ) + ( utfstr[ 1 ] & 0x3f );
    }

    else if( ( ( unsigned char ) utfstr[ 0 ] & 0x10 ) == 0 ){
        byte = 3;
        ucs2 = utfstr[ 0 ] & 0x0f;
        ucs2 = ( ucs2 << 6 ) + ( utfstr[ 1 ] & 0x3f );
        ucs2 = ( ucs2 << 6 ) + ( utfstr[ 2 ] & 0x3f );        
    }

    else{
        byte = 4;
        ucs2 = utfstr[ 0 ] & 0x07;
        ucs2 = ( ucs2 << 6 ) + ( utfstr[ 1 ] & 0x3f );
        ucs2 = ( ucs2 << 6 ) + ( utfstr[ 2 ] & 0x3f );
        ucs2 = ( ucs2 << 6 ) + ( utfstr[ 3 ] & 0x3f );        
    }

    return ucs2;
}




//
// ucs2 -> utf8 変換
//
// 戻り値 : バイト数
//
int MISC::ucs2toutf8( int ucs2, char* utfstr )
{
    int byte = 0;

    if( ucs2 < 0x7f ){ // ascii
        byte = 1;
        utfstr[ 0 ] = ucs2;
    }

    else if( ucs2 < 0x07ff ){
        byte = 2;
        utfstr[ 0 ] = ( 0xc0 ) + ( ucs2 >> 6 );
        utfstr[ 1 ] = ( 0x80 ) + ( ucs2 & 0x3f );
    }

    else if( ucs2 < 0xffff){
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
// str を大文字化
//
std::string MISC::toupper_str( const std::string& str )
{
    std::string str_out;
    for( size_t i = 0; i < str.length() ; ++i ) str_out += toupper( str[ i ] );

    return str_out;
}


//
// list 内のアイテムを全部大文字化
//
std::list< std::string > MISC::toupper_list( std::list< std::string >& list_str )
{
    std::list< std::string > list_out;
    std::list< std::string >::iterator it = list_str.begin();
    for( ; it != list_str.end() ; ++it ) list_out.push_back( MISC::toupper_str( *it ) );

    return list_out;
}



//
// str を小文字化
//
std::string MISC::tolower_str( const std::string& str )
{
    std::string str_out;

    for( size_t i = 0; i < str.length() ; ++i ) str_out += tolower( str[ i ] );

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

    if( i == std::string::npos ) path.substr( pos );

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
