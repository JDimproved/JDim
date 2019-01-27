// ライセンス: GPL2

//
// ポップアップウィンドウの基底クラス
//

#ifndef _POPUPWINBASE_H
#define _POPUPWINBASE_H

#include <gtkmm.h>

namespace SKELETON
{
    enum
    {
        POPUPWIN_DRAWFRAME = true,
        POPUPWIN_NOFRAME = false
    };

    //
    // Gtk::Windows は signal_configure_event()を発行しないようなので
    // 自前でconfigureイベントをフックしてシグナルを発行する
    //
    typedef sigc::signal< void, int, int > SIG_CONFIGURED_POPUP;

    class PopupWinBase : public Gtk::Window
    {
        SIG_CONFIGURED_POPUP m_sig_configured;
        Glib::RefPtr< Gdk::GC > m_gc;

        bool m_draw_frame;

      public:

        // draw_frame == true なら枠を描画する
        PopupWinBase( bool draw_frame ) : Gtk::Window( Gtk::WINDOW_POPUP ), m_draw_frame( draw_frame ){
            if( m_draw_frame ) set_border_width( 1 );
        }
        ~PopupWinBase() noexcept {}

        SIG_CONFIGURED_POPUP sig_configured(){ return m_sig_configured; }

      protected:

        void on_realize() override
        {
            Gtk::Window::on_realize();

            Glib::RefPtr< Gdk::Window > window = get_window();
            m_gc = Gdk::GC::create( window );    
        }

        bool on_expose_event( GdkEventExpose* event ) override
        {
            bool ret = Gtk::Window::on_expose_event( event );

            // 枠の描画
            if( m_draw_frame ){
                m_gc->set_foreground( Gdk::Color( "black" ) );
                get_window()->draw_rectangle( m_gc, false, 0, 0, get_width()-1, get_height()-1 );
            }

            return ret;
        }

        bool on_configure_event( GdkEventConfigure* event ) override
        {
            bool ret = Gtk::Window::on_configure_event( event );
            m_sig_configured.emit( get_width(), get_height() );

            return ret;
        }
    };
}

#endif
