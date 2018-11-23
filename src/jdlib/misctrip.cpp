// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "misctrip.h"
#include "miscutil.h"

#include <sstream>
#include <cstring>
#include <ctype.h>
#include <unistd.h> // crypt

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef USE_OPENSSL
#include <openssl/sha.h>
#else // defined USE_GNUTLS
#include <gcrypt.h>
#endif

#include <crypt.h>


/*--------------------------------------------------------------------*/
// ローカル関数の宣言

// SHA1を計算
const std::string create_sha1( const std::string& key );

// トリップを計算(新方式)
const std::string create_trip_newtype( const std::string& key );

// トリップを計算(従来方式)
const std::string create_trip_conventional( const std::string& key );
/*--------------------------------------------------------------------*/



/*--------------------------------------------------------------------*/
// SHA1を計算
//
// param1: 元となる文字列
// return: SHA1文字列
/*--------------------------------------------------------------------*/
const std::string create_sha1( const std::string& key )
{
    if( key.empty() ) return std::string();

#ifdef USE_OPENSSL

    constexpr const unsigned int digest_length = SHA_DIGEST_LENGTH;

    std::array< unsigned char, digest_length > digest;

    // unsigned char *SHA1( const unsigned char *, size_t, unsigned char * );
    SHA1( (const unsigned char *)key.c_str(), key.length(), digest.data() );

#else // defined USE_GNUTLS

    const unsigned int digest_length = gcry_md_get_algo_dlen( GCRY_MD_SHA1 );

    std::vector< unsigned char > digest( digest_length );

    gcry_md_hash_buffer( GCRY_MD_SHA1, digest.data(), key.c_str(), key.length() );

#endif

    std::stringstream sha1;

#ifdef _DEBUG
    std::cout << "create_sha1 : SHA1 = ";
#endif

    unsigned int n;
    for( n = 0; n < digest_length; ++n )
    {
        sha1 << digest[n];

#ifdef _DEBUG
        std::cout << std::hex << (unsigned int)digest[n];
#endif
    }

#ifdef _DEBUG
    std::cout << std::endl;
#endif

    return sha1.str();
}


/*--------------------------------------------------------------------*/
// トリップを計算(新方式) 2009/06の仕様
// param1: 元となる文字列
// return: トリップ文字列
/*--------------------------------------------------------------------*/
const std::string create_trip_newtype( const std::string& key )
{
    if( key.empty() ) return std::string();

    const size_t key_length = key.length();

    // キーは1024文字以内に限定される
    if( key_length > 1024 ) return std::string();

    std::string trip = "???";

    // "#"で始まる
    if( key[0] == '#' )
    {
        // 全体が17〜19文字 ^#[0-9A-Fa-f]{16}[./0-9A-Za-z]{0,2}$
        if( key_length > 16 && key_length < 20 )
        {
            // 16進数文字列部分
            const std::string hex_part = key.substr( 1, 16 );

            char key_binary[17] = { 0 };

            // 16進数文字列を全てバイナリに変換出来た [0-9A-Za-z]{16}
            if( MISC::chrtobin( hex_part.c_str(), key_binary ) == 16 )
            {
                char salt[5] = { 0 };

                bool is_salt_suitable = true;

                size_t n;

                // salt 候補の17〜18文字目を検証
                for( n = 17; key[n] && n < 19; ++n )
                {
                    // [./0-9A-Za-z]
                    if( isalnum( key[n] ) != 0 || (unsigned char)( key[n] - 0x2e ) < 2 )
                    {
                        strncat( salt, &key[n], 1 );
                    }
                    else
                    {
                        is_salt_suitable = false;
                        break;
                    }
                }

                // salt が適切(空も含む)
                if( is_salt_suitable == true )
                {
                    // salt に".."を足す
                    strncat( salt, "..", 2 );

                    // crypt (key は先頭8文字しか使われない)
                    const char *crypted = crypt( key_binary, salt );

                    // 末尾から10文字(cryptの戻り値はNULLでなければ必ず13文字)
                    if( crypted ) trip = std::string( crypted + 3 );
                    else trip.clear();
                }
            }
        }
    }
    // "$"で始まる
    else if( key[0] == '$' )
    {
        // 現在は"???"を返す
    }
    // SHA1パターン
    else
    {
        const std::string sha1 = create_sha1( key );

        if( ! sha1.empty() )
        {
            // BASE64エンコード
            const std::string encoded = MISC::base64( sha1 );

            // 先頭から12文字
            trip = encoded.substr( 0, 12 );
        }
    }

    return trip;
}


/*--------------------------------------------------------------------*/
// トリップを計算(従来方式) 2003/11/15の仕様らしい
// 新方式導入に伴い、特殊文字の置換が不要になったようだ
//
// param1: 元となる文字列
// return: トリップ文字列
/*--------------------------------------------------------------------*/
const std::string create_trip_conventional( const std::string& key )
{
    if( key.empty() ) return std::string();

    // key の2,3バイト目を salt として取り出す
    std::string salt = key.substr( 1, 2 );

    // 仕様に合わせて salt を変換
    const size_t salt_length = salt.length();
    size_t n;
    for( n = 0; n < salt_length; n++ )
    {
        // 0x2e〜0x7aの範囲にないものは '.'(0x2e)
        if( (unsigned char)( salt[n] - 0x2E ) > 0x4C )
        {
            salt[n] = 0x2e;
        }
        // :;<=>?@ (0x3a〜0x40) は A〜G (0x41〜0x47)
        else if( (unsigned char)( salt[n] - 0x3A ) < 0x07 )
        {
            salt[n] += 7;
        }
        // [\]^_` (0x5b〜0x60) は a〜f (0x61〜0x66)
        else if( (unsigned char)( salt[n] - 0x5B ) < 0x06 )
        {
            salt[n] += 6;
        }
    }

    // salt の末尾に"H."を足す
    salt.append( "H." );

    // crypt (key は先頭8文字しか使われない)
    const char *crypted = crypt( key.c_str(), salt.c_str() );

    std::string trip;

    // 末尾10(cryptの戻り値はNULLでなければ必ず13文字)
    if( crypted ) trip = std::string( crypted + 3 );

    return trip;
}


/*--------------------------------------------------------------------*/
// トリップを取得
//
// param1: 元となる文字列(最初の"#"を含まない。UTF-8 であること)
// param2: 書き込む掲示板の文字コード
// return: トリップ文字列
/*--------------------------------------------------------------------*/
const std::string MISC::get_trip( const std::string& str, const std::string& charset )
{
    if( str.empty() ) return std::string();

    // str の文字コードを UTF-8 から charset に変更して key に代入する
    std::string key = MISC::Iconv( str, "UTF-8", charset );

    std::string trip;

    // key が12文字以上の場合は新方式
    if( key.length() > 11 )
    {
        trip = create_trip_newtype( key );
    }
    // 従来方式
    else
    {
        trip = create_trip_conventional( key );
    }

#ifdef _DEBUG
    std::cout << "MISC::get_trip : " << str << " -> " << trip << std::endl;
#endif

    return trip;
}
