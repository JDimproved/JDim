// ライセンス: GPL2
//
// DragableNoteBookを構成するツールバー表示用の Notebook
//

#ifndef _TOOLBARNOTE_H
#define _TOOLBARNOTE_H

#include <gtkmm.h>

namespace SKELETON
{
    class DragableNoteBook;

    class ToolBarNotebook : public Gtk::Notebook
    {
#if !GTKMM_CHECK_VERSION(3,0,0)
        DragableNoteBook* m_parent;
#endif

      public:

        ToolBarNotebook( DragableNoteBook* parent );

#if !GTKMM_CHECK_VERSION(3,0,0)
      protected:

        bool on_expose_event( GdkEventExpose* event ) override;
#endif
    };
}

#endif
