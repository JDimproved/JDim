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

class WinMain : public SKELETON::JDWindow
{
    CORE::Core* m_core;
    
  public:

    WinMain( bool init );
    virtual ~WinMain();

    // 緊急シャットダウン
    void shutdown();

    // 通常のセッション保存
    void save_session();
};


#endif
