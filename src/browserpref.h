// ライセンス: GPL2

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
        Gtk::VBox m_vbox;
        Gtk::Label m_label_notice;
        Gtk::ComboBoxText m_combo;
        Gtk::Frame m_frame;
        Gtk::HBox m_hbox;
        Gtk::Entry m_entry_browser;

        // OK押した
        virtual void slot_ok_clicked(){
            CONFIG::set_browsercombo_id( m_combo.get_active_row_number() );
            CONFIG::set_command_openurl( MISC::remove_space( m_entry_browser.get_text() ) );
        }

        // コンボボックスが変わった
        void slot_changed(){
            m_entry_browser.set_text( CORE::get_browser_name( m_combo.get_active_row_number() ) );
        }

      public:

        BrowserPref( Gtk::Window* parent, const std::string& url )
        : SKELETON::PrefDiag( parent, url ),
        m_label_notice( "使用するWebブラウザを選択して下さい\nリンククリック時に %LINK をURLに置換します" )
        {
            const int mrg = 8;

            int i = 0;
            for(;;){
                std::string label = CORE::get_browser_label( i++ );
                if( label.empty() ) break;
                m_combo.append_text( label );
            }

            m_combo.set_active( CONFIG::get_browsercombo_id() );
            m_combo.signal_changed().connect( sigc::mem_fun(*this, &BrowserPref::slot_changed ) );

            m_entry_browser.set_text( CONFIG::get_command_openurl() );
            m_hbox.set_spacing( mrg );
            m_hbox.set_border_width( mrg );
            m_hbox.add( m_entry_browser );
            m_frame.set_label( "ブラウザ起動コマンド" );
            m_frame.add( m_hbox );
            m_label_notice.set_alignment( Gtk::ALIGN_START, Gtk::ALIGN_CENTER );

            m_vbox.set_border_width( mrg );
            m_vbox.pack_start( m_label_notice, Gtk::PACK_EXPAND_WIDGET, mrg );
            m_vbox.pack_start( m_combo, Gtk::PACK_EXPAND_WIDGET, 0 );
            m_vbox.pack_start( m_frame, Gtk::PACK_EXPAND_WIDGET, mrg );

            get_vbox()->set_spacing( 0 );
            get_vbox()->pack_start( m_vbox );

            set_activate_entry( m_entry_browser );

            set_title( "Webブラウザ設定" );
            show_all_children();
            resize( 400, 100 );
        }
    };

}

#endif
