// ライセンス: GPL2

//
// ポップアップウィンドウの基底クラス
//

#ifndef POPUPWINBASE_H
#define POPUPWINBASE_H

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
        const bool m_draw_frame;

      public:

        // draw_frame == true なら枠を描画する
        explicit PopupWinBase( bool draw_frame );
        ~PopupWinBase() noexcept override = default;

        SIG_CONFIGURED_POPUP sig_configured(){ return m_sig_configured; }

      protected:

        bool on_configure_event( GdkEventConfigure* event ) override;
    };
}

#endif
