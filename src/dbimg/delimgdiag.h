// ライセンス: GPL2

// 画像削除時の確認ダイアログ

#ifndef _DELIMGDIAG_H
#define _DELIMGDIAG_H

#include "skeleton/prefdiag.h"
#include "skeleton/spinbutton.h"

#include "config/globalconf.h"

#include "cache.h"

#include <sstream>

namespace DBIMG
{
    class ImgCacheFrame : public Gtk::Frame
    {
        Gtk::VBox m_vbox;
        Gtk::HBox m_hbox;

        Gtk::Label m_label;
        Gtk::Label m_spinlabel;

        SKELETON::SpinButton m_spin;

      public:

        SKELETON::SpinButton& get_spin(){ return m_spin; }

        ImgCacheFrame()
        {
            std::stringstream ss;
            ss << "現在の画像キャッシュサイズ : " << ( CACHE::get_dirsize( CACHE::path_img_root() ) / 1024 / 1024 ) << "M";
            m_label.set_text( ss.str() );

            m_spinlabel.set_text_with_mnemonic( "日より以前のキャッシュファイルを消去(_H)" );
            m_spinlabel.set_mnemonic_widget( m_spin );
            m_spin.set_range( 0, 360 );
            m_spin.set_increments( 1, 1 );
            m_spin.set_value( CONFIG::get_del_img_day() );
            
            m_hbox.set_border_width( 8 );
            m_hbox.set_spacing( 4 );
            m_hbox.pack_start( m_spin, Gtk::PACK_SHRINK );
            m_hbox.pack_start( m_spinlabel, Gtk::PACK_SHRINK );

            m_vbox.set_spacing( 16 );
            m_vbox.set_border_width( 8 );
            m_vbox.pack_start( m_label, Gtk::PACK_SHRINK );
            m_vbox.pack_start( m_hbox, Gtk::PACK_SHRINK );

            set_border_width( 8 );
            set_label( "画像キャッシュ" );
            add( m_vbox );
        }
    };

    class ImgAboneFrame : public Gtk::Frame
    {
        Gtk::VBox m_vbox;
        Gtk::HBox m_hbox;

        Gtk::Label m_spinlabel;

        SKELETON::SpinButton m_spin;

      public:

        SKELETON::SpinButton& get_spin(){ return m_spin; }

        ImgAboneFrame()
        {
            m_spinlabel.set_text_with_mnemonic( "日より以前のあぼ〜ん情報を消去(_A)" );
            m_spinlabel.set_mnemonic_widget( m_spin );
            m_spin.set_range( 0, 360 );
            m_spin.set_increments( 1, 1 );
            m_spin.set_value( CONFIG::get_del_imgabone_day()  );
            
            m_hbox.set_border_width( 16 );
            m_hbox.set_spacing( 4 );
            m_hbox.pack_start( m_spin, Gtk::PACK_SHRINK );
            m_hbox.pack_start( m_spinlabel, Gtk::PACK_SHRINK );

            set_border_width( 8 );
            set_label( "画像あぼ〜ん" );
            add( m_hbox );
        }
    };

    class DelImgDiag : public SKELETON::PrefDiag
    {
        ImgCacheFrame m_frame_cache;
        ImgAboneFrame m_frame_abone;

        // OK押した
        void slot_ok_clicked() override
        {
            CONFIG::set_del_img_day( m_frame_cache.get_spin().get_value_as_int() );
            CONFIG::set_del_imgabone_day( m_frame_abone.get_spin().get_value_as_int() );
        }

      public:

        DelImgDiag( Gtk::Window* parent, const std::string& url ):
        SKELETON::PrefDiag( parent, url )
        {

            get_vbox()->set_spacing( 8 );
            get_vbox()->pack_start( m_frame_cache );
            get_vbox()->pack_start( m_frame_abone );

            set_activate_entry( m_frame_cache.get_spin() );
            set_activate_entry( m_frame_abone.get_spin() );

            set_title( "画像キャッシュの消去" );
            show_all_children();
        }

        ~DelImgDiag() noexcept {}
    };

}

#endif
