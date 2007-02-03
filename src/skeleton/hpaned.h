// ライセンス: GPL2
//
// HPandeクラス
//
// 仕切りをクリックすると閉じるhpaned
//

#ifndef _HPANED_H
#define _HPANED_H

#include <gtkmm.h>

namespace SKELETON
{
    enum
    {
        HPANED_MODE_NORMAL, // 折り畳まない
        HPANED_MODE_LEFT // 仕切りをクリックすると左ページを閉じる
    };

    // 左ペーン表示/表示切り替え時にemit
    typedef sigc::signal< void, bool > SIG_SHOW_HIDE_LEFTPANE;

    class JDHPaned : public Gtk::HPaned
    {
        SIG_SHOW_HIDE_LEFTPANE m_sig_show_hide_leftpane;

        int m_mode;

        bool m_clicked;
        bool m_drag;
        int m_pos;

      public:

        JDHPaned();
        ~JDHPaned();

        SIG_SHOW_HIDE_LEFTPANE sig_show_hide_leftpane() { return m_sig_show_hide_leftpane; }

        void set_mode( int mode ){ m_mode = mode; }

        // unpack = true の時取り除く
        void add_remove1( bool unpack, Gtk::Widget& child );
        void add_remove2( bool unpack, Gtk::Widget& child );

        int get_position();
        void set_position( int position );

        // 左ペーン表示/表示切り替え
        void show_hide_leftpane();

      protected:
        virtual bool on_button_press_event( GdkEventButton* event );
        virtual bool on_button_release_event( GdkEventButton* event );
        virtual bool on_motion_notify_event( GdkEventMotion* event );
    };
}

#endif
