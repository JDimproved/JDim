// ライセンス: 最新のGPL

// ブラウザ設定ダイアログ

#ifndef _BROWSER_H
#define _BROWSER_H

#include "skeleton/prefdiag.h"

#include "config/globalconf.h"

#include "jdlib/miscutil.h"

namespace CORE
{
    class BrowserPref : public SKELETON::PrefDiag
    {
        Gtk::Label m_label_notice;
        Gtk::Entry m_entry_browser;

        // OK押した
        virtual void slot_ok_clicked(){
            CONFIG::set_command_openurl( MISC::remove_space( m_entry_browser.get_text() ) );
        }

      public:

        BrowserPref( const std::string& url )
            : SKELETON::PrefDiag( url )
            ,m_label_notice( "%s をURLに変換します" )
        {
            m_entry_browser.set_text( CONFIG::get_command_openurl() );

            get_vbox()->set_spacing( 8 );
            get_vbox()->pack_start( m_label_notice );
            get_vbox()->pack_start( m_entry_browser, Gtk::PACK_EXPAND_WIDGET );

            set_title( "ブラウザ設定" );
            show_all_children();
            resize( 400, 100 );
        }
    };

}

#endif
