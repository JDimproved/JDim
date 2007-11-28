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

    // gtk::entryのフォント名を取得
    std::string get_entry_font();

    // gtk::entryの文字色を16進数表記の文字列で取得
    std::string get_entry_color_text();

    // gtk::entryの背景色を16進数表記の文字列で取得
    std::string get_entry_color_base();

    // str をクリップボードにコピー
    void CopyClipboard( const std::string& str );

    // 画像の幅と高さを取得
    void get_img_size( const std::string& filename, int& width, int& height );

    // PixbufLoaderローダ取得
    // stop を trueにすると読み込みを停止する
    Glib::RefPtr< Gdk::PixbufLoader > get_ImageLoder( const std::string& file, bool& stop, bool pixbufonly, std::string& errmsg );

    class ImgLoader
    {
        Glib::RefPtr< Gdk::PixbufLoader > m_loader;

        std::string m_file;
        std::string m_errmsg;
        int m_width;
        int m_height;
        bool m_stop;

        bool m_pixbufonly;
        int m_y;

      public:

        ImgLoader( const std::string& file );

        virtual ~ImgLoader(){}

        Glib::RefPtr< Gdk::PixbufLoader > get_loader(){ return  m_loader; }
        const std::string& get_errmsg() const { return m_errmsg; }
        const int get_width() const { return m_width; }
        const int get_height() const{ return m_height; }

        bool get_size();
        bool load( bool& stop, bool pixbufonly, bool sizeonly );

      private:
        void slot_size_prepared( int w, int h );
        void slot_area_updated(int x, int y, int w, int h );
    };

}

#endif
