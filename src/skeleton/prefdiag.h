// ライセンス: GPL2

// 設定ダイアログの基底クラス

#ifndef _PREFDIAG_H
#define _PREFDIAG_H

#include <gtkmm.h>

namespace SKELETON
{
    class PrefDiag : public Gtk::Dialog
    {
        std::string m_url;

      public:

        // parent == NULL のときはメインウィンドウをparentにする
        PrefDiag( Gtk::Window* parent, const std::string& url, bool add_cancel = true );

        virtual ~PrefDiag(){}

        const std::string& get_url() const { return m_url; }

        virtual int run();

      private:

        virtual void slot_ok_clicked(){}
        virtual void slot_cancel_clicked(){}

        // タイマーのslot関数
        // コードが繁雑になるのでcoreとは別にする
        virtual bool slot_timeout( int timer_number ){ return true; }
    };
}

#endif
