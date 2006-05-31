// ライセンス: 最新のGPL

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

        virtual void shutdown();

        virtual void reload();
        virtual void show_view();
        virtual void update_view();

      protected:
        virtual void show_popupmenu( const Gtk::TreePath& path );

        void save_xml( const std::string& file );
    };
};


#endif
