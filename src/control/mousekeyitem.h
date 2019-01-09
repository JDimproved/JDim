// ライセンス: GPL2
//
// マウスやキー設定のデータ
//

#ifndef _MOUSEKEYITEM_H
#define _MOUSEKEYITEM_H

#include "controlid.h"

#include <gtkmm.h>
#include <string>

namespace CONTROL
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

    public:

        MouseKeyItem( const guint id, const int mode, const std::string& name, const std::string& str_motion,
                      const guint motion, const bool ctrl, const bool shift, const bool alt, const bool dblclick, const bool trpclick )
        : m_id( id ),
        m_mode( mode ),
        m_name( name ),
        m_str_motion( str_motion ),
        m_motion( motion ),
        m_ctrl( ctrl ),
        m_shift( shift ),
        m_alt( alt ),
        m_dblclick( dblclick ),
        m_trpclick( trpclick )
        {}

        int get_id() const { return m_id; }
        int get_mode() const { return m_mode; }
        const std::string& get_name() const { return m_name; }
        const std::string& get_str_motion() const { return m_str_motion; }
        gint get_motion() const { return m_motion; }
        bool get_ctrl() const { return m_ctrl; }
        bool get_shift() const { return m_shift; }
        bool get_alt() const { return m_alt; }
        bool get_dblclick() const { return m_dblclick; }
        bool get_trpclick() const { return m_trpclick; }

        // モード無視
        int equal( const std::string& str_motion )
        {
            if( str_motion == m_str_motion ) return m_id;
            return CONTROL::None;
        }

        // モード無視
        int equal( const guint motion, const bool ctrl, const bool shift, const bool alt, const bool dblclick, const bool trpclick )
        {
            if( motion == m_motion && ctrl == m_ctrl && shift == m_shift && alt == m_alt && dblclick == m_dblclick && trpclick == m_trpclick ) return m_id;
            return CONTROL::None;
        }

        int is_activated( const int mode, const std::string& str_motion )
        {
            if( mode == m_mode ) return equal( str_motion );
            return CONTROL::None;
        }

        int is_activated( const int mode, const guint motion, const bool ctrl, const bool shift, const bool alt,
                          const bool dblclick, const bool trpclick )
        {
            if( mode == m_mode ) return equal( motion, ctrl, shift, alt, dblclick, trpclick );
            return CONTROL::None;
        }
    };
}

#endif
