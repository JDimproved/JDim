// ライセンス: GPL2

//
// 外部板ビュー
//

#ifndef _ETCLISTVIEW_H
#define _ETCLISTVIEW_H

#include "bbslistviewbase.h"

namespace BBSLIST
{
    class EtcListView : public BBSListViewBase
    {
      public:
        EtcListView( const std::string& url, const std::string& arg1 = std::string() , const std::string& arg2 = std::string() );
        virtual ~EtcListView();

        virtual void show_view();

      protected:
        virtual Gtk::Menu* get_popupmenu( const std::string& url );
    };
};


#endif
