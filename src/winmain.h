// ライセンス: GPL2
//
// メインウィンドウとmain関数
//

#ifndef _MAINWIN_H
#define _MAINWIN_H

#include "skeleton/window.h"

namespace CORE
{
    class Core;
}

class JDWinMain : public SKELETON::JDWindow
{
    CORE::Core* m_core;
    bool m_cancel_state_event;
    
  public:

    JDWinMain( const bool init, const bool skip_setupdiag );
    virtual ~JDWinMain();

    // 緊急シャットダウン
    void shutdown();

    // 通常のセッション保存
    void save_session();

    void hide();

  protected:

    virtual const int get_x_win();
    virtual const int get_y_win();
    virtual void set_x_win( int x );
    virtual void set_y_win( int y );

    virtual const int get_width_win();
    virtual const int get_height_win();
    virtual void set_width_win( int width );
    virtual void set_height_win( int height );

    virtual const bool is_focus_win();
    virtual void set_focus_win( bool set );

    virtual const bool is_maximized_win();
    virtual void set_maximized_win( bool set );

    virtual const bool is_iconified_win();
    virtual void set_iconified_win( bool set );

    virtual const bool is_shown_win();
    virtual void set_shown_win( bool set );

    virtual bool on_delete_event( GdkEventAny* event );
    virtual bool on_window_state_event( GdkEventWindowState* event );
    virtual bool on_configure_event( GdkEventConfigure* event );
};


#endif
