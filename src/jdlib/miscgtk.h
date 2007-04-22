// ライセンス: GPL2

// gtk, gdk 関係の関数

#ifndef _MISCGTK_H
#define _MISCGTK_H

#include <gtkmm.h>

#include <set>

namespace MISC
{
    // Gdk::Color -> 16進数表記の文字列
    std::string color_to_str( const Gdk::Color& color );

    // int[3] -> 16進数表記の文字列
    std::string color_to_str( const int* l_rgb );

    // htmlカラー (#ffffffなど) -> 16進数表記の文字列
    std::string htmlcolor_to_str( const std::string& htmlcolor );

    // Gdk::Color -> int 変換
    guint32 color_to_int( const Gdk::Color& color );

    // 使用可能なフォントの一覧を取得
    std::set< std::string > get_font_families();


    // 画像の幅と高さを取得
    void get_img_size( const std::string& filename, int& width, int& height );

    class Img_Size
    {
        int m_width;
        int m_height;

      public:

        Img_Size( const std::string& file );
        ~Img_Size(){}

        const int get_width() const { return m_width; }
        const int get_height() const{ return m_height; }

      private:
        void slot_size_prepared( int w, int h );
    };


    // 画像ローダ取得
    // stop を trueにすると読み込みを停止する
    Glib::RefPtr< Gdk::PixbufLoader > get_ImageLoder( const std::string& file, int width, int height, bool& stop, std::string& errmsg );
}

#endif
