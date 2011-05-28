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
    virtual ~JDWinMain();

    // 通常のセッション保存
    void save_session();

    void hide();

  protected:

    virtual const int get_x_win();
    virtual const int get_y_win();
    virtual void set_x_win( const int x );
    virtual void set_y_win( const int y );

    virtual const int get_width_win();
    virtual const int get_height_win();
    virtual void set_width_win( const int width );
    virtual void set_height_win( const int height );

    virtual const bool is_focus_win();
    virtual void set_focus_win( const bool set );

    virtual const bool is_maximized_win();
    virtual void set_maximized_win( const bool set );

    virtual const bool is_iconified_win();
    virtual void set_iconified_win( const bool set );

    virtual const bool is_full_win();
    virtual void set_full_win( const bool set );

    virtual const bool is_shown_win();
    virtual void set_shown_win( const bool set );

    virtual bool on_delete_event( GdkEventAny* event );
    virtual bool on_window_state_event( GdkEventWindowState* event );
    virtual bool on_configure_event( GdkEventConfigure* event );
    virtual bool on_button_press_event( GdkEventButton* event );
    virtual bool on_button_release_event( GdkEventButton* event );
    virtual bool on_motion_notify_event( GdkEventMotion* event );

  private:
    const bool operate_win( const int control );
};


#endif
