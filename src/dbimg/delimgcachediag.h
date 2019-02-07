// ライセンス: GPL2

// 画像キャッシュ削除ダイアログ
//
// キャンセルボタンを押すとキャッシュの削除を中止する

#ifndef _DELIMGCACHEDIAG_H
#define _DELIMGCACHEDIAG_H

#include "jdlib/jdthread.h"

#include "skeleton/dispatchable.h"

#include <gtkmm.h>
#include <ctime>

namespace DBIMG
{
    class DelImgCacheDiag : public Gtk::Dialog, SKELETON::Dispatchable
    {
        Gtk::Label m_label;
       
        bool m_stop; // = true にするとスレッド停止
        JDLIB::Thread m_thread;

      public:

        DelImgCacheDiag();
        ~DelImgCacheDiag();

        // 画像キャッシュ削除スレッド
        void main_thread();

        static void* launcher( void* dat );

      protected:

#if GTKMM_CHECK_VERSION(3,0,0)
        bool on_draw( const Cairo::RefPtr< Cairo::Context >& cr ) override;
#else
        bool on_expose_event( GdkEventExpose* event ) override;
#endif

      private:

        void callback_dispatch() override;
        void wait();
        void slot_cancel_clicked();
        time_t get_days( const std::string& path );
        void launch_thread();
    };
}

#endif
