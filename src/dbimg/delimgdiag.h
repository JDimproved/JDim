// ライセンス: GPL2

// 画像削除時の確認ダイアログ

#ifndef _DELIMGDIAG_H
#define _DELIMGDIAG_H

#include "skeleton/prefdiag.h"

#include "config/globalconf.h"

#include "cache.h"

#include <sstream>


#if GTKMMVER <= 240
// gtkのバージョンが2.4以下の時はスピンが回ったときに
// 明示的に値をセットする必要がある
class SpinButton24 : public Gtk::SpinButton
{

public:

    SpinButton24() : Gtk::SpinButton(){}

protected:

    virtual void on_spinbutton_digits_changed(){
        const size_t size = 256;
        char str[ size ];
        snprintf( str, size, "%d", (int)get_value() );
        set_text( str );
    }
};

#endif



namespace DBIMG
{
    class DelImgDiag : public SKELETON::PrefDiag
    {
        Gtk::VBox m_vbox;
        Gtk::Label m_label;
        Gtk::HBox m_hbox;
        Gtk::Label m_spinlabel;

#if GTKMMVER <= 240
        SpinButton24 m_spin;
#else
        Gtk::SpinButton m_spin;
#endif

        // OK押した
        virtual void slot_ok_clicked(){
            CONFIG::set_del_img_day( m_spin.get_value_as_int() );
        }

      public:

        DelImgDiag(): SKELETON::PrefDiag( "" )
        {
            std::stringstream ss;
            ss << "現在の画像キャッシュサイズ : " << ( CACHE::get_dirsize( CACHE::path_img_root() ) / 1024 / 1024 ) << "M";
            m_label.set_text( ss.str() );

            m_spinlabel.set_text( "日より以前のキャッシュファイルを削除しますか？" );
            m_spin.set_range( 0, 360 );
            m_spin.set_increments( 1, 1 );
            m_spin.set_value( CONFIG::get_del_img_day() );
            
            m_hbox.set_spacing( 4 );
            m_hbox.pack_start( m_spin, Gtk::PACK_SHRINK );
            m_hbox.pack_start( m_spinlabel, Gtk::PACK_SHRINK );

            m_vbox.set_spacing( 16 );
            m_vbox.pack_start( m_label, Gtk::PACK_SHRINK );
            m_vbox.pack_start( m_hbox, Gtk::PACK_SHRINK );

            get_vbox()->set_spacing( 8 );
            get_vbox()->pack_start( m_vbox );

            set_title( "画像キャッシュ削除" );
            show_all_children();
        }

        virtual ~DelImgDiag(){}
    };

}

#endif
