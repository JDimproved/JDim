// ライセンス: GPL2

// お気に入りビュー

#ifndef _FAVORITEVIEW_H
#define _FAVORITEVIEW_H

#include "bbslistviewbase.h"

namespace BBSLIST
{
    // お気に入りビュー
    class FavoriteListView : public BBSListViewBase
    {
      public:

        explicit FavoriteListView( const std::string& url, const std::string& arg1 = {}, const std::string& arg2 = {} );
        ~FavoriteListView();

        void show_view() override;

      protected:

        // xml保存
        void save_xml() override;

        Gtk::Menu* get_popupmenu( const std::string& url ) override;
    };
}

#endif
