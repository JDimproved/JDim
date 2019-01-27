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
        DragableNoteBook* m_parent;

      public:

        ToolBarNotebook( DragableNoteBook* parent );

      protected:

        bool on_expose_event( GdkEventExpose* event ) override;
    };
}

#endif
