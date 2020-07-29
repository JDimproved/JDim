// ライセンス: GPL2
//
// DragableNoteBookを構成するツールバー表示用の Notebook
//
// TODO: 使われなくなったコンストラクタの引数を整理する

#ifndef _TOOLBARNOTE_H
#define _TOOLBARNOTE_H

#include <gtkmm.h>

namespace SKELETON
{
    class DragableNoteBook;

    class ToolBarNotebook : public Gtk::Notebook
    {
      public:

        explicit ToolBarNotebook( DragableNoteBook* );
        ~ToolBarNotebook() noexcept;
    };
}

#endif
