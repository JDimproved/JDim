// ライセンス: GPL2
//
// メインウィンドウとmain関数
//

#ifndef _MAINWIN_H
#define _MAINWIN_H

#include "skeleton/window.h"

#include "control/control.h"

namespace CORE
{
    class Core;
}

class JDWinMain : public SKELETON::JDWindow
{
    CORE::Core* m_core;
    bool m_cancel_state_event;
    
    // 入力コントローラ
    CONTROL::Control m_control;

  public:

    JDWinMain( const bool init, const bool skip_setupdiag, const int init_w, const int init_h, const int init_x, const int init_y );
    ~JDWinMain();

    // 通常のセッション保存
    void save_session();

    void hide();

  protected:

    int get_x_win() override;
    int get_y_win() override;
    void set_x_win( const int x ) override;
    void set_y_win( const int y ) override;

    int get_width_win() override;
    int get_height_win() override;
    void set_width_win( const int width ) override;
    void set_height_win( const int height ) override;

    bool is_focus_win() override;
    void set_focus_win( const bool set ) override;

    bool is_maximized_win() override;
    void set_maximized_win( const bool set ) override;

    bool is_iconified_win() override;
    void set_iconified_win( const bool set ) override;

    bool is_full_win() override;
    void set_full_win( const bool set ) override;

    bool is_shown_win() override;
    void set_shown_win( const bool set ) override;

    bool on_delete_event( GdkEventAny* event ) override;
    bool on_window_state_event( GdkEventWindowState* event ) override;
    bool on_configure_event( GdkEventConfigure* event ) override;
    bool on_button_press_event( GdkEventButton* event ) override;
    bool on_button_release_event( GdkEventButton* event ) override;
    bool on_motion_notify_event( GdkEventMotion* event ) override;

  private:
    bool operate_win( const int control );
};


#endif
