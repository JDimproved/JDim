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
        ~BoardToolBar() noexcept {}

        // ツールバー表示切り替え時に検索関係の wiget の位置を変更する
        void unpack_pack();

      protected:

        void pack_buttons() override;

      private:

        void pack_toolbar();
        void pack_search_toolbar();
    };
}


#endif
