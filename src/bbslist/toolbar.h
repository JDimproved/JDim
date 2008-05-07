// ライセンス: GPL2

// ツールバーのクラス

#ifndef _BBSLIST_TOOLBAR_H
#define _BBSLIST_TOOLBAR_H

#include <gtkmm.h>

#include "skeleton/toolbar.h"
#include "skeleton/jdtoolbar.h"
#include "skeleton/menubutton.h"
#include "skeleton/toolmenubutton.h"

namespace BBSLIST
{
    class BBSListToolBar : public SKELETON::ToolBar
    {
        // ラベルバー
        SKELETON::JDToolbar m_tool_label;

        Gtk::Label m_label;
        SKELETON::MenuButton m_button_toggle;
        SKELETON::ToolMenuButton m_tool_toggle;

      public:

        BBSListToolBar();
        virtual ~BBSListToolBar(){}

        // タブが切り替わった時にDragableNoteBookから呼び出される( Viewの情報を取得する )
        virtual void set_view( SKELETON::View * view );

      protected:

        virtual void pack_buttons();

      private:

        void slot_toggle( int i );
        bool slot_scroll_event( GdkEventScroll* event );
    };
}


#endif
