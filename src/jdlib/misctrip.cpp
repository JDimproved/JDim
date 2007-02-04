// ライセンス: GPL2

//
// Thanks to 「パッチ投稿」スレの9氏
//
// http://jd4linux.sourceforge.jp/cgi-bin/bbs/test/read.cgi/support/1151836078/9
//

//#define _DEBUG
#include "jddebug.h"

#include "misctrip.h"
#include "miscutil.h"

#include <unistd.h> // crypt


//
// key から salt を取得
//
const std::string MISC::get_salt( const std::string& key )
{
    if( key.empty() ) return std::string();

    // key の2,3バイト目を salt として取り出す
    std::string salt = key.substr( 1, 2 );

    // salt の末尾に "H." を足す
    salt += "H.";

    // その他の仕様に合わせて salt を変換
    unsigned int i;
    for( i=0; i < salt.length(); i++ )
    {
        // 0x2e〜0x7aの範囲にないものは '.'(0x2e)
        if( salt[i] < 0x2e || salt[i] > 0x7a )
        {
            salt[i] = 0x2e;
        }
        // :;<=>?@ (0x3a〜0x40) は A〜G (0x41〜0x47)
        else if( salt[i] >= 0x3a && salt[i] <= 0x40 )
        {
            salt[i] += 7;
        }
        // [\]^_` (0x5b〜0x60) は a〜f (0x61〜0x66)
        else if( salt[i] >= 0x5b && salt[i] <= 0x60 )
        {
            salt[i] += 6;
        }
    }

    return salt;
}


//
// key と salt から trip を計算
//
const std::string MISC::create_trip( const std::string& key, const std::string& salt )
{
    if( key.empty() || salt.empty() ) return std::string();

    // crypt( key, salt )
    const char *temp = crypt( key.c_str(), salt.c_str() );

    const std::string crypted( temp );

    // crypted(必ず13文字) から末尾10文字を trip として取り出す
    // crypted.substr( crypted.length()-10, 10 );
    const std::string trip = crypted.substr( 3, 10 );

    return trip;
}


//
// トリップ取得
//
// str は UTF-8 であること
//
const std::string MISC::get_trip( const std::string& str, const std::string& charset )
{
    if( str.empty() ) return std::string();

    // str の文字コードを UTF-8 から charset に変更して key に代入する
    std::string key = MISC::Iconv( str, "UTF-8", charset );

    // "<> をHTMLエスケープ
    key = MISC::replace_str( key, "\"", "&quot;" );
    key = MISC::replace_str( key, "<", "&lt;" );
    key = MISC::replace_str( key, ">", "&gt;" );

    // key を元に salt を取得
    const std::string salt = get_salt( key );

    // trip を計算
    const std::string trip = create_trip( key, salt );

#ifdef _DEBUG
    std::cout << "MISC::get_trip : " << str << " -> " << trip << std::endl;
#endif

    return trip;
}
