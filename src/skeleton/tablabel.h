// ライセンス: 最新のGPL

#ifndef _TABLABEL_H
#define _TABLABEL_H

#include <gtkmm.h>
#include <string>

#include "control.h"

namespace SKELETON
{
    // マウス
    typedef sigc::signal< void > SIG_TAB_MOTION_EVENT;
    typedef sigc::signal< void > SIG_TAB_LEAVE_EVENT;

    // D&D
    typedef sigc::signal< void > SIG_TAB_DRAG_BEGIN;
    typedef sigc::signal< void > SIG_TAB_DRAG_DROP;
    typedef sigc::signal< void > SIG_TAB_DRAG_END;

    class TabLabel : public Gtk::EventBox
    {
        SIG_TAB_MOTION_EVENT m_sig_tab_motion_event;
        SIG_TAB_LEAVE_EVENT m_sig_tab_leave_event;

        SIG_TAB_DRAG_BEGIN m_sig_tab_drag_begin;
        SIG_TAB_DRAG_DROP m_sig_tab_drag_drop;
        SIG_TAB_DRAG_END m_sig_tab_drag_end;

        std::string m_url;
        Gtk::HBox m_hbox;
        int m_id_icon;

        Gtk::Label m_label;
        Gtk::Image* m_image;

        // ラベルに表示する文字列の全体
        std::string m_fulltext;

        bool m_under_mouse;

      public:

        TabLabel( const std::string& url );
        ~TabLabel();

        SIG_TAB_MOTION_EVENT sig_tab_motion_event(){ return  m_sig_tab_motion_event; }
        SIG_TAB_LEAVE_EVENT sig_tab_leave_event(){ return m_sig_tab_leave_event; }

        SIG_TAB_DRAG_BEGIN sig_tab_drag_begin() { return m_sig_tab_drag_begin; }
        SIG_TAB_DRAG_DROP sig_tab_drag_drop() { return m_sig_tab_drag_drop; }
        SIG_TAB_DRAG_END sig_tab_drag_end() { return m_sig_tab_drag_end; }

        const std::string& get_url(){ return m_url; }

        const bool is_under_mouse() const { return m_under_mouse; }

        // カットしていない全体の文字列
        const std::string& get_fulltext() const { return m_fulltext; }
        void set_fulltext( const std::string& label );

        // 実際にラベルに表示している文字列
        const Glib::ustring get_text() const { return m_label.get_text(); }

        // アイコンセット
        void set_id_icon( int id );
        const int get_id_icon() const { return m_id_icon; }

        // タブの幅(ピクセル)
        const int get_tabwidth();

        // 伸縮
        bool dec();
        bool inc();

      private:

        // タブの文字列の文字数がlngになるようにリサイズする
        void resize_tab( int lng );

        void set_dragable( bool dragable, int button );

        virtual bool on_enter_notify_event( GdkEventCrossing* event );
        virtual bool on_motion_notify_event( GdkEventMotion* event );
        virtual bool on_leave_notify_event( GdkEventCrossing* event );

        virtual void on_drag_begin( const Glib::RefPtr< Gdk::DragContext>& context );
        virtual bool on_drag_drop( const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time );
        virtual void on_drag_end( const Glib::RefPtr< Gdk::DragContext>& context );
    }; 
}

#endif
