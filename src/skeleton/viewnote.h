// ライセンス: GPL2
//
// DragableNoteBookを構成するview表示用の Notebook
//

#ifndef _VIEWNOTE_H
#define _VIEWNOTE_H

#include <gtkmm.h>

namespace SKELETON
{
    class DragableNoteBook;

    class ViewNotebook : public Gtk::Notebook
    {
        DragableNoteBook* m_parent;

      public:

        ViewNotebook( DragableNoteBook* parent );

        void redraw_scrollbar();

      protected:

        bool on_expose_event( GdkEventExpose* event ) override;
    };
}

#endif
