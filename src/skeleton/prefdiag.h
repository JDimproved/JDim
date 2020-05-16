// ライセンス: GPL2

// 設定ダイアログの基底クラス

#ifndef _PREFDIAG_H
#define _PREFDIAG_H

#include "gtkmmversion.h"

#include <gtkmm.h>

#include "jdlib/timeout.h"

namespace SKELETON
{
    class LabelEntry;

    class PrefDiag : public Gtk::Dialog
    {
        std::string m_url;

        Gtk::Button* m_bt_ok;
        Gtk::Button m_bt_apply;

        std::unique_ptr<JDLIB::Timeout> m_conn_timer;

      public:

        // parent == nullptr のときはメインウィンドウをparentにする
        PrefDiag( Gtk::Window* parent, const std::string& url, const bool add_cancel = true, const bool add_apply = false, const bool add_open = false );

        ~PrefDiag();

        const std::string& get_url() const { return m_url; }

        // okボタンをフォーカス
        void grab_ok();

        // Entry、LabelEntryがactiveになったときにOKでダイアログを終了させる
        void set_activate_entry( Gtk::Entry& entry );
        void set_activate_entry( LabelEntry& entry );

        virtual int run();

      protected:

        virtual void slot_ok_clicked(){}
        virtual void slot_cancel_clicked(){}
        virtual void slot_apply_clicked(){}

#if GTKMM_CHECK_VERSION(3,0,0)
        void set_default_size_ratio( double ratio );
#endif

      private:

        // タイマーのslot関数
        bool slot_timeout( int timer_number );

        // 各設定ダイアログ別のタイムアウト処理 ( slot_timeout()から呼び出される )
        virtual void timeout(){}

        void slot_activate_entry();
    };
}

#endif
