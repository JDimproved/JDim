// ライセンス: GPL2

// 設定ダイアログの基底クラス

#ifndef _PREFDIAG_H
#define _PREFDIAG_H

#include <gtkmm.h>

#include "command.h"

namespace SKELETON
{
    class PrefDiag : public Gtk::Dialog
    {
        std::string m_url;
        virtual void slot_ok_clicked(){}
        virtual void slot_cancel_clicked(){}

      public:

        // parent == NULL のときはメインウィンドウをparentにする
        PrefDiag( Gtk::Window* parent, const std::string& url, bool add_cancel = true )
        : Gtk::Dialog(), m_url( url )
        {
            if( add_cancel ){
                add_button( Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL )
                ->signal_clicked().connect( sigc::mem_fun(*this, &PrefDiag::slot_cancel_clicked ) );
            }

            add_button( Gtk::Stock::OK, Gtk::RESPONSE_OK )
            ->signal_clicked().connect( sigc::mem_fun(*this, &PrefDiag::slot_ok_clicked ) );

            if( parent ) set_transient_for( *parent );
            else set_transient_for( *CORE::get_mainwindow() );
       }

        virtual ~PrefDiag(){}

        const std::string& get_url() const { return m_url; }

        virtual int run(){

            // run() 前後でフォーカスが外れて画像ウィンドウの開け閉めをしないように
            // Image admin にコマンドを送る
            CORE::core_set_command( "disable_fold_image_win" );
            int ret = Gtk::Dialog::run();
            CORE::core_set_command( "enable_fold_image_win" );

            return ret;
        }
    };
}

#endif
