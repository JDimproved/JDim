// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "miscgtk.h"

#include <errno.h> // ERANGE

// char の初期化用
#define CHAR_BUF 256


// 16進数表記の文字列 -> Gdk:Color
Gdk::Color MISC::str_to_color( const std::string& color_in )
{
    Gdk::Color color;

    // 13文字かどうか (#0123456789AB)
    if( color_in.length() < 13 ) return color;

    // R,G,Bに分割 ( #ABCD01234567 -> ABCD, 0123, 4567 )
    std::string rgb[3];
    rgb[0] = color_in.substr( 1, 4 );
    rgb[1] = color_in.substr( 5, 4 );
    rgb[2] = color_in.substr( 9, 4 );

    // 各色 int に変換
    gushort result;
    gushort l_rgb[3];
    for( int i = 0; i < 3; i++ )
    {
        result = strtoul( rgb[i].c_str(), NULL, 16 );

        if( result != ERANGE ) l_rgb[i] = result;
    }

#ifdef _DEBUG
    std::cout << "MISC::str_to_color" << std::endl;
    std::cout << "[" << color_in << "] -> "
              << std::hex << l_rgb[0] << ", " << l_rgb[1] << ", " << l_rgb[2] << std::endl;
#endif

    // 各色セット
    color.set_rgb( l_rgb[0], l_rgb[1], l_rgb[2] );

    return color;
}


// Gdk::Color -> 16進数表記の文字列
std::string MISC::color_to_str( const Gdk::Color& color )
{
    // R,G,Bを取得
    int l_rgb[3];
    l_rgb[0] = color.get_red();
    l_rgb[1] = color.get_green();
    l_rgb[2] = color.get_blue();

    return color_to_str( l_rgb );
}

// int[3] -> 16進数表記の文字列
std::string MISC::color_to_str( const int* l_rgb )
{
    // 16進数表記で各色の文字列を連結して格納
    char str_value[ CHAR_BUF ];
    snprintf( str_value, CHAR_BUF, "#%04x%04x%04x", l_rgb[ 0 ], l_rgb[ 1 ], l_rgb[ 2 ] );

#ifdef _DEBUG
    std::cout << "MISC::color_to_str" << std::endl;
    std::cout << l_rgb[0] << ", " << l_rgb[1] << ", " << l_rgb[2] << "->"
              << str_value << std::endl;
#endif

    return str_value;
}
