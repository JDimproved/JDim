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
#if !GTKMM_CHECK_VERSION(3,0,0)
        DragableNoteBook* m_parent;
#endif

      public:

        ViewNotebook( DragableNoteBook* parent );

        void redraw_scrollbar();

#if !GTKMM_CHECK_VERSION(3,0,0)
      protected:

        bool on_expose_event( GdkEventExpose* event ) override;
#endif
    };
}

#endif
