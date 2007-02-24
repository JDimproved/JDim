// ライセンス: GPL2

// スピンボタンのの基底クラス
//
// gtkのバージョンが2.4以下の時はスピンが回ったときに
// 明示的に値をセットする必要があるらしい


#ifndef _SPINBUTTON_H
#define _SPINBUTTON_H

#include <gtkmm.h>

namespace SKELETON
{
    class SpinButton : public Gtk::SpinButton
    {

      public:

        SpinButton() : Gtk::SpinButton(){}
        ~SpinButton(){}

      protected:

#if GTKMMVER <= 240
        virtual void on_spinbutton_digits_changed(){
            const size_t size = 256;
            char str[ size ];
            snprintf( str, size, "%d", (int)get_value() );
            set_text( str );
        }
#endif

    };
}

#endif
