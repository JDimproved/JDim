// ライセンス: GPL2

//
// メインビュー
//

#ifndef _BBSLISTVIEW_H
#define _BBSLISTVIEW_H

#include "bbslistviewbase.h"

namespace BBSLIST
{
    // メインビュー
    class BBSListViewMain : public BBSListViewBase
    {
      public:
        BBSListViewMain( const std::string& url, const std::string& arg1 = std::string() , const std::string& arg2 = std::string() );
        virtual ~BBSListViewMain();

        virtual void show_view();
        virtual void update_view();
        virtual void delete_view();
        virtual void show_preference();

      protected:

        // xml保存
        virtual void save_xml();

        virtual Gtk::Menu* get_popupmenu( const std::string& url );

      private:

        virtual void delete_view_impl();
    };
}


#endif
