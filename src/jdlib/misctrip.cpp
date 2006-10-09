// ライセンス: 最新のGPL

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


namespace MISC
{
    //
    // HTMLエスケープ
    //
    // ["<>] を &quot; &lt; &gt; に変換
    //
    bool html_escape( const char *input, char *key, unsigned int bufsize )
    {
        const unsigned int margin = 16;

        unsigned int i, n;
        for( i=0, n=0; i<=strlen(input); i++, n++ )
        {
            if( n >= bufsize - margin ) return false;

            switch(input[i])
            {
                case '"' :
                    key[n] = '&'; n++;
                    key[n] = 'q'; n++;
                    key[n] = 'u'; n++;
                    key[n] = 'o'; n++;
                    key[n] = 't'; n++;
                    key[n] = ';'; 
                    break;
                case '<' :
                    key[n] = '&'; n++;
                    key[n] = 'l'; n++;
                    key[n] = 't'; n++;
                    key[n] = ';';
                    break;
                case '>' :
                    key[n] = '&'; n++;
                    key[n] = 'g'; n++;
                    key[n] = 't'; n++;
                    key[n] = ';';
                    break;
                default :
                    key[n] = input[i];
                    break;
            }
        }

        return true;
    }


    //
    // salt の変換
    //
    void convert_salt( char *key, char *salt )
    {
        unsigned int i;
        for (i=0; i<strlen(salt); i++)
        {
            // 0x2e〜0x7aの範囲にないものは '.'(0x2e)
            if ( salt[i] < 0x2e || salt[i] > 0x7a )
            {
                salt[i] = 0x2e;
            }
            // :;<=>?@ (0x3a〜0x40) は A〜G (0x41〜0x47)
            else if ( salt[i] >= 0x3a && salt[i] <= 0x40 )
            {
                salt[i] += 7;
            }
            // [\]^_` (0x5b〜0x60) は a〜f (0x61〜0x66)
            else if ( salt[i] >= 0x5b && salt[i] <= 0x60 )
            {
                salt[i] += 6;
            }
        }
    }


    //
    // トリップ生成
    //
    void create_trip( const char *key, const char *salt, char *trip )
    {
        // とりあえず crypt()
        char *crypted = crypt(key, salt);

        // crypted の末尾10文字を trip にコピー
        strcpy( trip, &crypted[ strlen(crypted) - 10 ] );
    }

    
};



//
// トリップ取得
//
// str は UTF-8 であること
//
std::string MISC::get_trip( const std::string& str, const std::string& charset )
{
    const int chrbuf = 256;

    // key 用の変数
    char key[ chrbuf ];

    std::string str_enc = MISC::iconv( str, "UTF-8", charset );

    // input を元にHTMLエスケープ処理した値を key に入れる
    if( ! MISC::html_escape( str_enc.c_str(), key, chrbuf ) ) return std::string();

    // key の2,3文字目を salt とする
    char salt[ chrbuf ] = { key[1], key[2], '\0' };

    // salt の末尾に"H."を足しておく
    strncat(salt, "H.", 2);

    // salt を仕様に合わせて変換する
    MISC::convert_salt(key, salt);

    // トリップ生成
    char trip[ chrbuf ];
    MISC::create_trip( key, salt, trip );

    std::string str_trip( trip );

#ifdef _DEBUG
    std::cout << "MISC::get_trip : " << str << " -> " << str_trip << std::endl;
#endif

    return str_trip;
}

