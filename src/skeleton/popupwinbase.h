// ライセンス: GPL2

//
// ポップアップウィンドウの基底クラス
//

#ifndef POPUPWINBASE_H
#define POPUPWINBASE_H

#include "gtkmmversion.h"

#include <gtkmm.h>

namespace SKELETON
{
    constexpr bool POPUPWIN_DRAWFRAME = true;
    constexpr bool POPUPWIN_NOFRAME = false;

    //
    // Gtk::Windows は signal_configure_event()を発行しないようなので
    // 自前でconfigureイベントをフックしてシグナルを発行する
    //
    using SIG_CONFIGURED_POPUP = sigc::signal< void, int, int >;

    class PopupWinBase : public Gtk::Window
    {
        SIG_CONFIGURED_POPUP m_sig_configured;
#if !GTKMM_CHECK_VERSION(3,0,0)
        Glib::RefPtr< Gdk::GC > m_gc;
#endif

        const bool m_draw_frame;

      public:

        // draw_frame == true なら枠を描画する
        explicit PopupWinBase( bool draw_frame );
        ~PopupWinBase() noexcept;

        SIG_CONFIGURED_POPUP sig_configured(){ return m_sig_configured; }

      protected:

        void on_realize() override;
#if GTKMM_CHECK_VERSION(3,0,0)
        bool on_draw( const Cairo::RefPtr< Cairo::Context >& cr ) override;
#else
        bool on_expose_event( GdkEventExpose* event ) override;
#endif
        bool on_configure_event( GdkEventConfigure* event ) override;
    };
}

#endif
