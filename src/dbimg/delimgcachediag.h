// ライセンス: GPL2

// 画像キャッシュ削除ダイアログ
//
// キャンセルボタンを押すとキャッシュの削除を中止する

#ifndef _DELIMGCACHEDIAG_H
#define _DELIMGCACHEDIAG_H

#include "jdlib/jdthread.h"

#include <gtkmm.h>

namespace DBIMG
{
    class DelImgCacheDiag : public Gtk::Dialog
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

        virtual bool on_expose_event( GdkEventExpose* event );

      private:

        void wait();
        void slot_cancel_clicked();
        time_t get_days( const std::string& path );
    };
}

#endif
