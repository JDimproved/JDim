// ライセンス: GPL2

// ツールバーのクラス

#ifndef _BOARD_TOOLBAR_H
#define _BOARD_TOOLBAR_H

#include <gtkmm.h>

#include "skeleton/toolbar.h"

namespace BOARD
{
    class BoardToolBar : public SKELETON::ToolBar
    {
      public:

        BoardToolBar();
        virtual ~BoardToolBar(){}

      protected:

        virtual void pack_buttons();
    };
}


#endif
