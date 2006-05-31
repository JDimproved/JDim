// ライセンス: 最新のGPL

//
// ドラッグ&ドロップの管理クラス
//

#ifndef _DNDMANAGER_H
#define _DNDMANAGER_H

#include <gtkmm.h>
#include <string>

namespace CORE
{
    class DND_Manager
    {
        // D&D 開始と終了時にシグナルを出す
        typedef sigc::signal< void > SIG_DND_BEGIN;
        typedef sigc::signal< void > SIG_DND_END;

        SIG_DND_END m_sig_dnd_begin;
        SIG_DND_END m_sig_dnd_end;

        // 開始したwidgetのurl
        std::string m_url_from;

        // true ならd&d中
        bool m_dnd;

      public:

        DND_Manager();
        virtual ~DND_Manager();

        SIG_DND_END sig_dnd_begin(){ return m_sig_dnd_begin; }
        SIG_DND_END sig_dnd_end(){ return m_sig_dnd_end; }

        const bool now_dnd() const{ return m_dnd; }
        const std::string& url_from() const { return m_url_from; }

        // DnD 開始
        void begin( const std::string& url_from );

        // DnD終了
        void end();
    };

    ///////////////////////////////////////
    // インターフェース

    DND_Manager* get_dnd_manager();
    void delete_dnd_manager();

    void DND_Begin( const std::string& url_from );
    void DND_End();

    const bool DND_Now_dnd();
    const std::string DND_Url_from();
}


#endif
