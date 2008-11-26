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

        FavoriteListView( const std::string& url, const std::string& arg1 = std::string() , const std::string& arg2 = std::string() );
        virtual ~FavoriteListView();

        virtual void show_view();

      protected:

        // xml保存
        virtual void save_xml( const bool backup );

        virtual Gtk::Menu* get_popupmenu( const std::string& url );
    };
}

#endif
