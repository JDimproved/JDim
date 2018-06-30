// ライセンス: GPL2

// gtk, gdk 関係の関数

#ifndef _MISCGTK_H
#define _MISCGTK_H

#include "gtkmmversion.h"

#include <gtkmm.h>

#include <set>

namespace MISC
{
    // Gdk::Color -> 16進数表記の文字列
    std::string color_to_str( const Gdk::Color& color );

    // int[3] -> 16進数表記の文字列
    std::string color_to_str( const int* l_rgb );

#if GTKMM_CHECK_VERSION(3,0,0)
    // Gdk::RGBA -> 16進数表記の文字列
    std::string color_to_str( const Gdk::RGBA& rgba );
#endif

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
}

#endif
