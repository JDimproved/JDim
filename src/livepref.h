// ライセンス: GPL2

// 実況設定ダイアログ

#ifndef _LIVEPREF_H
#define _LIVEPREF_H

#include "skeleton/prefdiag.h"

#include "config/globalconf.h"

#include "global.h"

namespace CORE
{
    class LivePref : public SKELETON::PrefDiag
    {
        Gtk::VBox m_vbox;

        Gtk::Label m_label_inst;

        Gtk::Frame m_frame_mode;
        Gtk::VBox m_vbox_mode;
        Gtk::RadioButtonGroup m_radiogroup;
        Gtk::RadioButton m_mode1;
        Gtk::RadioButton m_mode2;

        Gtk::HBox m_hbox_speed;
        SKELETON::SpinButton m_spin_speed;
        Gtk::Label m_label_speed;

        Gtk::HBox m_hbox_th;
        SKELETON::SpinButton m_spin_th;
        Gtk::Label m_label_th;

        // OK押した
        virtual void slot_ok_clicked()
        {
            if( m_mode1.get_active() ) CONFIG::set_live_mode( LIVE_SCRMODE_VARIABLE );
            else CONFIG::set_live_mode( LIVE_SCRMODE_STEADY );

            CONFIG::set_live_speed( m_spin_speed.get_value_as_int() );
            CONFIG::set_live_threshode( m_spin_th.get_value_as_int() );
        }

      public:

        LivePref( Gtk::Window* parent, const std::string& url )
        : SKELETON::PrefDiag( parent, url ),
        m_label_inst( "実況を行うには始めに板のプロパティで更新間隔を設定して下さい。\n速度を0にするとスクロールしません。" ),
        m_mode1( m_radiogroup, "速度可変、速度がしきい値を越えると行単位でスクロール(_1)", true ),
        m_mode2( m_radiogroup, "速度一定、遅れがしきい値を越えると行単位でスクロール(_2)", true )
        {
            const int mrg = 8;

            // スクロールモード
            m_vbox_mode.set_spacing( mrg );
            m_vbox_mode.set_border_width( mrg );
            m_vbox_mode.pack_start( m_mode1 );
            m_vbox_mode.pack_start( m_mode2 );
            m_frame_mode.set_label( "オートスクロールモード" );
            m_frame_mode.add( m_vbox_mode );
            if( CONFIG::get_live_mode() == LIVE_SCRMODE_VARIABLE ) m_mode1.set_active( true );
            else m_mode2.set_active( true );

            // 速度
            m_spin_speed.set_range( 0, 50 );
            m_spin_speed.set_increments( 1, 1 );
            m_spin_speed.set_value( CONFIG::get_live_speed() );
            m_label_speed.set_text_with_mnemonic( "可変モードでの最低速度/一定モードでの速度(_S)：" );
            m_label_speed.set_mnemonic_widget( m_spin_speed );
            m_hbox_speed.set_spacing( mrg );
            m_hbox_speed.pack_start( m_label_speed, Gtk::PACK_SHRINK );
            m_hbox_speed.pack_start( m_spin_speed, Gtk::PACK_SHRINK );

            // しきい値
            m_spin_th.set_range( 1, 50 );
            m_spin_th.set_increments( 1, 1 );
            m_spin_th.set_value( CONFIG::get_live_threshold() );
            m_label_th.set_text_with_mnemonic( "しきい値(_T)：" );
            m_label_th.set_mnemonic_widget( m_spin_th );
            m_hbox_th.set_spacing( mrg );
            m_hbox_th.pack_start( m_label_th, Gtk::PACK_SHRINK );
            m_hbox_th.pack_start( m_spin_th, Gtk::PACK_SHRINK );

            m_vbox.pack_start( m_frame_mode, Gtk::PACK_SHRINK );
            m_vbox.pack_start( m_hbox_speed, Gtk::PACK_SHRINK );
            m_vbox.pack_start( m_hbox_th,  Gtk::PACK_SHRINK );
            m_vbox.set_border_width( mrg );

            get_vbox()->set_spacing( mrg );
            get_vbox()->pack_start( m_label_inst, Gtk::PACK_SHRINK );
            get_vbox()->pack_start( m_vbox, Gtk::PACK_SHRINK );

            set_title( "実況設定" );
            show_all_children();
        }

        virtual ~LivePref(){}
    };

}

#endif
