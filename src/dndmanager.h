// ライセンス: GPL2

//
// ドラッグ&ドロップの管理クラス
//

#ifndef _DNDMANAGER_H
#define _DNDMANAGER_H

#include <gtkmm.h>
#include <string>

// ターゲット

#define DNDTARGET_FAVORITE "dnd/favorite"
#define DNDTARGET_TAB "dnd/tab"
#define DNDTARGET_IMAGETAB "dnd/imagetab"
#define DNDTARGET_USRCMD "dnd/usrcmd"

namespace CORE
{
    class DND_Manager
    {
        // true ならd&d中
        bool m_dnd;

      public:

        DND_Manager():m_dnd( false ){}
        virtual ~DND_Manager() noexcept {}

        bool now_dnd() const { return m_dnd; }

        // DnD 開始
        void begin(){ m_dnd = true; }

        // DnD終了
        void end(){ m_dnd = false; }
    };

    ///////////////////////////////////////
    // インターフェース

    DND_Manager* get_dnd_manager();
    void delete_dnd_manager();

    void DND_Begin();
    void DND_End();
    bool DND_Now_dnd();
}


#endif
