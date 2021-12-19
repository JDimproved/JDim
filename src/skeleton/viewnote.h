// ライセンス: GPL2
//
// DragableNoteBookを構成するview表示用の Notebook
//
// TODO: 使われなくなったコンストラクタの引数を整理する

#ifndef _VIEWNOTE_H
#define _VIEWNOTE_H

#include <gtkmm.h>

namespace SKELETON
{
    class DragableNoteBook;

    class ViewNotebook : public Gtk::Notebook
    {
      public:

        explicit ViewNotebook( DragableNoteBook* );
        ~ViewNotebook() noexcept;
    };
}

#endif
