// ライセンス: GPL2
//
// 自前のツールチップクラス
//

#ifndef _TOOLTIP_H
#define _TOOLTIP_H

#include "popupwinbase.h"

namespace SKELETON
{
    class Tooltip : public PopupWinBase
    {
        Gtk::Label m_label;

        int m_counter;

        // 横幅が m_min_width より大きければ表示
        int m_min_width;

      public:

        Tooltip();
        void clock_in();
        void modify_font_label( const std::string& fontname );

        void set_text( const std::string& text );
        void set_min_width( const int min_width ){ m_min_width = min_width; }

        void show_tooltip();
        void hide_tooltip();
    };
}


#endif
