// ライセンス: GPL2
//
// マウスやキー設定のデータ
//

#ifndef _MOUSEKEYITEM_H
#define _MOUSEKEYITEM_H

#include "controlid.h"

namespace CONFIG
{
    class MouseKeyItem
    {
        int m_id;
        int m_mode;
        std::string m_name;
        std::string m_str_motion;

        guint m_motion;
        bool m_ctrl;
        bool m_shift;
        bool m_alt;
        bool m_dblclick;
        bool m_trpclick;
        bool m_save; // 設定ファイルに保存するか

    public:

        MouseKeyItem( const guint id, const int mode, const std::string& name, const std::string& str_motion,
                      const guint motion, const bool ctrl, const bool shift, const bool alt, const bool dblclick, const bool trpclick,
                      const bool save )
        : m_id( id ),
        m_mode( mode ),
        m_name( name ),
        m_str_motion( str_motion ),
        m_motion( motion ),
        m_ctrl( ctrl ),
        m_shift( shift ),
        m_alt( alt ),
        m_dblclick( dblclick ),
        m_trpclick( trpclick ),
        m_save( save )
        {}

        const int get_id() const { return m_id; }
        const int get_mode() const{ return m_mode; }
        const std::string& get_name() const { return m_name; }
        const std::string& get_str_motion() const { return m_str_motion; }
        const gint get_motion() const{ return m_motion; }
        const bool get_ctrl() const { return m_ctrl; }
        const bool get_shift() const { return m_shift; }
        const bool get_alt() const { return m_alt; }
        const bool get_dblclick() const { return m_dblclick; }
        const bool get_trpclick() const { return m_trpclick; }
        const bool get_save() const { return m_save; }

        // モード無視
        const int equal( const guint motion, const bool ctrl, const bool shift, const bool alt, const bool dblclick, const bool trpclick )
        {
            if( motion == m_motion && ctrl == m_ctrl && shift == m_shift && alt == m_alt && dblclick == m_dblclick && trpclick == m_trpclick ) return m_id;
            return CONTROL::None;
        }

        const int is_activated( const int& mode,
                          const guint& motion, const bool& ctrl, const bool& shift, const bool& alt, const bool& dblclick, const bool& trpclick )
        {
            if( mode == m_mode ) return equal( motion, ctrl, shift, alt, dblclick, trpclick );
            return CONTROL::None;
        }
    };
}

#endif
