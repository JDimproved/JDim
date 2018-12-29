// ライセンス: GPL2

// スピンボタンのの基底クラス
//
// gtkのバージョンが2.4以下の時はスピンが回ったときに
// 明示的に値をセットする必要があるらしい


#ifndef _SPINBUTTON_H
#define _SPINBUTTON_H

#include <gtkmm.h>
#include "gtkmmversion.h"

namespace SKELETON
{
    class SpinButton : public Gtk::SpinButton
    {

      public:

        SpinButton() : Gtk::SpinButton(){}

      protected:

#if !GTKMM_CHECK_VERSION(2,5,0)
        void on_spinbutton_digits_changed() override
        {
            const size_t size = 256;
            char str[ size ];
            snprintf( str, size, "%d", (int)get_value() );
            set_text( str );
        }
#endif

    };

    class SpinButtonDouble : public Gtk::SpinButton
    {
      public:

        SpinButtonDouble() : Gtk::SpinButton(){}

      protected:

#if !GTKMM_CHECK_VERSION(2,5,0)
        void on_spinbutton_digits_changed() override
        {
            const size_t size = 256;
            char str[ size ];
            snprintf( str, size, "%4.3lf", get_value() );
            set_text( str );
        }
#endif

    };
}

#endif
