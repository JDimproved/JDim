// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "jdtoolbar.h"


using namespace SKELETON;


JDToolbar::JDToolbar()
    : Gtk::Toolbar()
{
    // 子ウィジェットの配色がGTKテーマと違うことがある。
    // ツールバーのcssクラスを削除し配色を修正する。
    get_style_context()->remove_class( GTK_STYLE_CLASS_TOOLBAR );
}


JDToolbar::~JDToolbar() noexcept = default;
