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

        PrefDiag( const std::string& url, bool add_cancel = true ) : m_url( url )
        {
            if( add_cancel ){
                add_button( Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL )
                ->signal_clicked().connect( sigc::mem_fun(*this, &PrefDiag::slot_cancel_clicked ) );
            }

            add_button( Gtk::Stock::OK, Gtk::RESPONSE_OK )
            ->signal_clicked().connect( sigc::mem_fun(*this, &PrefDiag::slot_ok_clicked ) );

            // ダイアログを消したときに画像ウィンドウにフォーカスが移ってしまうので
            // 画像ウィンドウをhideしておいてデストラクタでshowする
            CORE::core_set_command( "hide_imagewindow" );
       }

        virtual ~PrefDiag(){
            CORE::core_set_command( "show_imagewindow" );
        }

        const std::string& get_url() const { return m_url; }
    };
}

#endif
