// ライセンス: 最新のGPL

// ブラウザ設定ダイアログ

#ifndef _BROWSER_H
#define _BROWSER_H

#include "skeleton/prefdiag.h"

#include "config/globalconf.h"

#include "jdlib/miscutil.h"

#include "browsers.h"

namespace CORE
{
    class BrowserPref : public SKELETON::PrefDiag
    {
        Gtk::Label m_label_notice;
        Gtk::ComboBoxText m_combo;
        Gtk::Entry m_entry_browser;

        // OK押した
        virtual void slot_ok_clicked(){
            CONFIG::set_brownsercombo_id( m_combo.get_active_row_number() );
            CONFIG::set_command_openurl( MISC::remove_space( m_entry_browser.get_text() ) );
        }

        // コンボボックスが変わった
        void slot_changed(){
            m_entry_browser.set_text( CORE::browsers[ m_combo.get_active_row_number() ][ 1 ] );
        }

      public:

        BrowserPref( const std::string& url )
        : SKELETON::PrefDiag( url ),
        m_label_notice( "コンボボックスの中から使用するWebブラウザを選択して下さい\nリンククリック時に %LINK をURLに変換します" )
        {
            int i = 0;
            for(;;){

                if( !strlen( CORE::browsers[ i ][ 0 ] ) ) break;
                m_combo.append_text( CORE::browsers[ i++ ][ 0 ] );
            }

            m_combo.set_active( CONFIG::get_brownsercombo_id() );
            m_combo.signal_changed().connect( sigc::mem_fun(*this, &BrowserPref::slot_changed ) );

            m_entry_browser.set_text( CONFIG::get_command_openurl() );

            m_label_notice.set_alignment( Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER );

            get_vbox()->set_spacing( 0 );
            get_vbox()->pack_start( m_label_notice, Gtk::PACK_EXPAND_WIDGET, 8 );
            get_vbox()->pack_start( m_combo, Gtk::PACK_EXPAND_WIDGET, 0 );
            get_vbox()->pack_start( m_entry_browser, Gtk::PACK_EXPAND_WIDGET, 8 );

            set_title( "Webブラウザ設定" );
            show_all_children();
            resize( 400, 100 );
        }
    };

}

#endif
