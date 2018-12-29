// ライセンス: GPL2
//
// カスタマイズした Gtk::Toolbar ( 背景を描画しない )
//
// (注) DragableNoteBookにappendするのは SKELETON::ToolBar
//

#ifndef JDTOOLBAR_H
#define JDTOOLBAR_H

#include <gtkmm.h>

namespace SKELETON
{
    class JDToolbar : public Gtk::Toolbar
    {
    public:
        JDToolbar() noexcept {}

    protected:
        bool on_expose_event( GdkEventExpose* event ) override;
    };
}

#endif
