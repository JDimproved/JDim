// ライセンス: GPL2
//
// メインウィンドウとmain関数
//

#ifndef _MAINWIN_H
#define _MAINWIN_H

#include <gtkmm.h>

namespace CORE
{
    class Core;
}

class WinMain : public Gtk::Window
{
    CORE::Core* m_core;
    bool m_maximized;
    
  public:
    WinMain( bool init );
    ~WinMain();

    // 緊急シャットダウン
    void shutdown();

    // 通常のセッション保存
    void save_session();

  protected:
    virtual bool on_window_state_event( GdkEventWindowState* event );
};


#endif
