// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "miscutil.h"
#include "miscmsg.h"
#include "jdiconv.h"

#include <sstream>


//
// str を "\n" ごとに区切ってlistにして出力
//
std::list< std::string > MISC::get_lines( const std::string& str, bool rm_space ){
        
    std::list< std::string > lines;
    unsigned int i = 0,i2 = 0, r = 0;
    while ( ( i2 = str.find( "\n", i ) ) != std::string::npos ){
        r = 0;
        if( str[ i2 - 1 ] == '\r' ) r = 1;
        if( i2 - i > 0 ){
            std::string str_tmp = str.substr( i, i2 - i - r );

            if( rm_space ) lines.push_back( remove_space( str_tmp ) );
            else lines.push_back( str_tmp );
        }
        i = i2 + 1;
    }

    // 最後の行
    if( i != str.length() +1 ){
        if( rm_space ) lines.push_back( remove_space( str.substr( i ) ) );
        else lines.push_back( str.substr( i ) );
    }
    
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
    int pos = 0, length = 0;
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
    int lng_space = str_space.length();
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
        int lng_tmp = 1;
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
// strの前後の空白削除
//
std::string MISC::remove_space( const std::string& str )
{
    std::string str_space = "　";
    int lng_space = str_space.length();

    std::string str_out;
    int lng = str.length();
    
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
    int i2 = lng -1;
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
// str1, str2 に囲まれた文字列を切り出す
//
std::string MISC::cut_str( const std::string& str, const std::string& str1, const std::string& str2 )
{
    unsigned int i = str.find( str1 );
    if( i == std::string::npos ) return std::string();
    i += str1.length();
    unsigned int i2 = str.find( str2, i );
    if( i2 == std::string::npos ) return std::string();
    
    return str.substr( i, i2 - i );
}


// str1 を str2 に置き換え
std::string MISC::replace_str( const std::string& str, const std::string& str1, const std::string& str2 )
{
    std::string str_out;
    unsigned int i, pos = 0;
    while( ( i = str.find( str1 , pos ) ) != std::string::npos ){

        str_out += str.substr( pos, ( i - pos ) ) + str2;
        pos += ( i - pos ) + str1.length();
    }

    str_out += str.substr( pos );
    return str_out;
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
std::string MISC::charset_url_encode( const std::string& str, const std::string& charset )
{
    if( charset.empty() ) return MISC::url_encode( str.c_str(), str.length() );

    char* str_bk = ( char* ) malloc( str.length() + 64 );
    strcpy( str_bk, str.c_str() );

    JDLIB::Iconv* libiconv = new JDLIB::Iconv( "UTF-8", charset.c_str() );
    int byte_out;
    const char* str_enc = libiconv->convert( str_bk, strlen( str_bk ), byte_out );
    std::string str_encoded = MISC::url_encode( str_enc, strlen( str_enc ) );
    delete libiconv;
    free( str_bk );

    return str_encoded;
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
int MISC::ucs2utf8( int ucs2, char* utfstr )
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
    for( unsigned int i = 0; i < str.length() ; ++i ) str_out += toupper( str[ i ] );

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

    for( unsigned int i = 0; i < str.length() ; ++i ) str_out += tolower( str[ i ] );

    return str_out;
}



//
// path からホスト名だけ取り出す
//
std::string MISC::get_hostname( const std::string& path )
{
    if( path.find( "http://" ) == std::string::npos ) return std::string();
    unsigned int i = path.find( "/", strlen( "http://" ) ); 
    if( i == std::string::npos ) return std::string();

    return path.substr( 0, i );
}



//
// path からファイル名だけ取り出す
//
std::string MISC::get_filename( const std::string& path )
{
    if( path.empty() ) return std::string();

    unsigned int i = path.rfind( "/" );
    if( i == std::string::npos ) return path;

    return path.substr( i+1 );
}



//
// path からファイル名を除いてディレクトリだけ取り出す
//
std::string MISC::get_dir( const std::string& path )
{
    if( path.empty() ) return std::string();

    unsigned int i = path.rfind( "/" );
    if( i == std::string::npos ) return std::string();

    return path.substr( 0, i+1 );
}
