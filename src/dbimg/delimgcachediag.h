// ライセンス: GPL2

// 画像キャッシュ削除ダイアログ
//
// キャンセルボタンを押すとキャッシュの削除を中止する

#ifndef _DELIMGCACHEDIAG_H
#define _DELIMGCACHEDIAG_H

#include "skeleton/dispatchable.h"

#include <gtkmm.h>
#include <ctime>
#include <thread>


namespace DBIMG
{
    class DelImgCacheDiag : public Gtk::Dialog, SKELETON::Dispatchable
    {
        Gtk::Label m_label;

        bool m_stop; // = true にするとスレッド停止
        std::thread m_thread;

      public:

        DelImgCacheDiag();
        ~DelImgCacheDiag() override;

        // 画像キャッシュ削除スレッド
        void main_thread();

      protected:

        bool on_draw( const Cairo::RefPtr< Cairo::Context >& cr ) override;

      private:

        void callback_dispatch() override;
        void wait();
        void slot_cancel_clicked();
        static int get_days( const std::string& path );
        void launch_thread();
    };
}

#endif
