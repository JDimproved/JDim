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
        explicit BBSListViewMain( const std::string& url, const std::string& arg1 = {}, const std::string& arg2 = {} );
        ~BBSListViewMain() override;

        void show_view() override;
        void update_view() override;
        void delete_view() override;
        void show_preference() override;

      protected:

        // xml保存
        void save_xml() override;

        Gtk::Menu* get_popupmenu( const std::string& url ) override;

      private:

        void delete_view_impl() override;
    };
}


#endif
