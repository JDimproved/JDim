// ライセンス: GPL2

// スレ一覧 メインビュー

#ifndef _BOARDVIEW_H
#define _BOARDVIEW_H

#include "boardviewbase.h"

namespace BOARD
{
    class BoardView : public BOARD::BoardViewBase
    {
      public:

        BoardView( const std::string& url );
        virtual ~BoardView();

        // SKELETON::View の関数のオーバロード

        virtual void save_session();

        virtual const bool is_updated();
        virtual const bool is_check_update();

        virtual void reload();
        virtual void show_view();
        virtual void update_view();
        virtual void update_boardname();
    };
}

#endif
