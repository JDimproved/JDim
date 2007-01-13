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
        bool m_load_etc;

      public:
        BBSListViewMain( const std::string& url, const std::string& arg1 = std::string() , const std::string& arg2 = std::string() );
        virtual ~BBSListViewMain();

        virtual void shutdown();

        virtual void reload();
        virtual void show_view();
        virtual void update_view();

      protected:

        virtual Gtk::Menu* get_popupmenu( const std::string& url );

        void save_xml( const std::string& file );
    };
};


#endif
