// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "livepref.h"

#include "config/globalconf.h"
#include "config/defaultconf.h"

#include "global.h"

using namespace CORE;

LivePref::LivePref( Gtk::Window* parent, const std::string& url )
    : SKELETON::PrefDiag( parent, url ),
      m_label_inst( "実況を行うには始めに板のプロパティで更新間隔を設定して下さい。\n速度を0にするとスクロールしません。" ),
      m_mode1( m_radiogroup, "速度可変、速度がしきい値を越えると行単位でスクロール(_1)", true ),
      m_mode2( m_radiogroup, "速度一定、遅れがしきい値を越えると行単位でスクロール(_2)", true ),
      m_bt_reset( "設定を全てデフォルトに戻す(_F)", true )
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
    m_label_speed.set_text_with_mnemonic( "可変モードでの最低速度/一定モードでの速度(_S):" );
    m_label_speed.set_mnemonic_widget( m_spin_speed );
    m_label_speed.set_halign( Gtk::ALIGN_START );

    set_activate_entry( m_spin_speed );

    // しきい値
    m_spin_th.set_range( 1, 50 );
    m_spin_th.set_increments( 1, 1 );
    m_spin_th.set_value( CONFIG::get_live_threshold() );
    m_label_th.set_text_with_mnemonic( "しきい値(_T):" );
    m_label_th.set_mnemonic_widget( m_spin_th );
    m_label_th.set_halign( Gtk::ALIGN_START );

    set_activate_entry( m_spin_th );

    // デフォルトに戻す
    m_bt_reset.signal_clicked().connect( sigc::mem_fun( *this, &LivePref::slot_reset ) );

    m_grid.property_margin() = 10;
    m_grid.set_column_spacing( 10 );
    m_grid.set_row_spacing( 8 );

    m_grid.attach( m_label_inst, 0, 0, 4, 1 );
    m_grid.attach( m_frame_mode, 0, 1, 4, 1 );

    m_grid.attach( m_label_speed, 0, 2, 3, 1 );
    m_grid.attach( m_spin_speed, 3, 2, 1, 1 );

    m_grid.attach( m_label_th, 0, 3, 3, 1 );
    m_grid.attach( m_spin_th, 3, 3, 1, 1 );

    m_grid.attach( m_bt_reset, 0, 4, 4, 1 );

    get_content_area()->pack_start( m_grid, Gtk::PACK_SHRINK );

    set_title( "実況設定" );
    show_all_children();
}


// OK押した
void LivePref::slot_ok_clicked()
{
    if( m_mode1.get_active() ) CONFIG::set_live_mode( LIVE_SCRMODE_VARIABLE );
    else CONFIG::set_live_mode( LIVE_SCRMODE_STEADY );

    CONFIG::set_live_speed( m_spin_speed.get_value_as_int() );
    CONFIG::set_live_threshode( m_spin_th.get_value_as_int() );
}


void LivePref::slot_reset()
{
    m_mode1.set_active( true );

    m_spin_speed.set_value( CONFIG::CONF_LIVE_SPEED );
    m_spin_th.set_value( CONFIG::CONF_LIVE_THRESHOLD );
}
