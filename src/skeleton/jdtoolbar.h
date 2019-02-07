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
        JDToolbar()
        {
#if GTKMM_CHECK_VERSION(3,0,0)
            // 子ウィジェットの配色がGTKテーマと違うことがある。
            // ツールバーのcssクラスを削除し配色を修正する。
            get_style_context()->remove_class( GTK_STYLE_CLASS_TOOLBAR );
#endif
        }

        // GTK+3ではデフォルトの描画処理に任せる
#if !GTKMM_CHECK_VERSION(3,0,0)
    protected:
        bool on_expose_event( GdkEventExpose* event ) override;
#endif
    };
}

#endif
