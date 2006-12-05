// ライセンス: GPL2

//
// お気に入り追加の時の選択ビュー
//

#ifndef _SELECTLISTVIEW_H
#define _SELECTLISTVIEW_H

#include "bbslistviewbase.h"

namespace BBSLIST
{
    class SelectListView : public BBSListViewBase
    {
      public:

        SelectListView( const std::string& url, const std::string& arg1 = std::string() , const std::string& arg2 = std::string() );
        virtual ~SelectListView(){}

        virtual void delete_view(){ delete_selected_rows(); }

      private:
        virtual bool open_row( Gtk::TreePath& path, bool tab );
        virtual void switch_rightview(){} // boardに移動しないようにキャンセル
        virtual Gtk::Menu* get_popupmenu( const std::string& url );
    };

};


#endif
