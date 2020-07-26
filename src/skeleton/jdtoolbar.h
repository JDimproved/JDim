// ライセンス: GPL2
//
// カスタマイズした Gtk::Toolbar ( 背景を描画しない )
//
// (注) DragableNoteBookにappendするのは SKELETON::ToolBar
//
// TODO: クライアント側で Gtk::Toolbar に置き換える
// TODO: CONFIG::(set|get)_draw_toolbarback() を削除する

#ifndef JDTOOLBAR_H
#define JDTOOLBAR_H

#include <gtkmm.h>

namespace SKELETON
{
    class JDToolbar : public Gtk::Toolbar
    {
    public:
        JDToolbar();
        ~JDToolbar() noexcept;
    };
}

#endif
