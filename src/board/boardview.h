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

        explicit BoardView( const std::string& url );
        ~BoardView() override;

        // SKELETON::View の関数のオーバロード

        void save_session() override;

        bool is_updated() const override;
        bool is_check_update() const override;

        void reload() override;
        void show_view() override;
        void update_view() override;
        void update_boardname() override;
    };
}

#endif
