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

        virtual bool set_command( const std::string& command, const std::string& arg = std::string() );

        virtual void shutdown();

        virtual void show_view();
        virtual void update_view();
        virtual void delete_view(){ delete_selected_rows(); }

      protected:

        virtual Gtk::Menu* get_popupmenu( const std::string& url );

      private:

        // お気に入りにアイテム追加
        // あらかじめ共有バッファに追加するデータをセットしておくこと
        void append_favorite();

        void save_xml( const std::string& file );
    };
}

#endif
