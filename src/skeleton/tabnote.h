// ライセンス: GPL2
//
// タブ用の Notebook
//

#ifndef _TABNOTE_H
#define _TABNOTE_H

#include <gtkmm.h>

namespace SKELETON
{

    // タブのクリック
    typedef sigc::signal< bool, GdkEventButton* > SIG_BUTTON_PRESS;
    typedef sigc::signal< bool, GdkEventButton* > SIG_BUTTON_RELEASE;

    // タブ用の Notebook
    class TabNotebook : public Gtk::Notebook
    {
        SIG_BUTTON_PRESS m_sig_button_press;
        SIG_BUTTON_RELEASE m_sig_button_release;

      public:

        SIG_BUTTON_PRESS sig_button_press(){ return m_sig_button_press; }
        SIG_BUTTON_RELEASE sig_button_release(){ return m_sig_button_release; }

        TabNotebook();

        int append_tab( Widget& tab );
        int insert_tab( Widget& tab, int page );
        void remove_tab( int page );

      protected:

        // signal_button_press_event と signal_button_release_event は emit されない
        // ときがあるので自前でemitする
        virtual bool on_button_press_event( GdkEventButton* event );
        virtual bool on_button_release_event( GdkEventButton* event );
    };

}

#endif
