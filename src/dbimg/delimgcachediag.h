// ライセンス: GPL2

// 画像キャッシュ削除ダイアログ
//
// キャンセルボタンを押すとキャッシュの削除を中止する

#ifndef _DELIMGCACHEDIAG_H
#define _DELIMGCACHEDIAG_H

#include "cache.h"

#include "config/globalconf.h"

#include "jdlib/miscutil.h"
#include "jdlib/miscmsg.h"

#ifdef _DEBUG
#include "jdlib/misctime.h"
#endif

#include <sys/time.h>

typedef void* ( *FUNC )( void * );

namespace DBIMG
{

    class DelImgCacheDiag : public Gtk::Dialog
    {
        Gtk::Label m_label;
       
        bool m_stop; // = true にするとスレッド停止
        pthread_t m_thread;

        void wait()
        {
#ifdef _DEBUG
            std::cout << "DelImgCacheDiag::wait\n";
#endif
            if( m_thread ) pthread_join( m_thread, NULL );
            m_thread = 0;
        }

        void slot_cancel_clicked(){
#ifdef _DEBUG
            std::cout << "DelImgCacheDiag::slot_cancel_clicked\n";
#endif
            m_stop = true;
            wait();
        }

      public:

        // 画像キャッシュ削除
        void main_thread(){
#ifdef _DEBUG
            std::cout << "DelImgCacheDiag::main_thread\n";
#endif

            std::list< std::string >list_infofile;
            std::string path_info_root = CACHE::path_img_info_root();
            list_infofile = CACHE::get_filelist( path_info_root );

            std::list< std::string >::iterator it_info = list_infofile.begin();
            for(; ! m_stop && it_info != list_infofile.end(); ++it_info ){

                std::string path_info = CACHE::path_img_info_root() + ( *it_info );

                // 経過日数を計算
                struct timeval tv;
                struct timezone tz;
                gettimeofday( &tv, &tz );
                time_t mtime = CACHE::get_filemtime( path_info );
                if( ! mtime ) continue;

                time_t days = ( tv.tv_sec - mtime ) / ( 60 * 60 * 24 );
                if( days < CONFIG::get_del_img_day() ) continue;

                std::string filename = MISC::get_filename( path_info );
                std::string path_file = CACHE::path_img_root() + filename.substr( 0, filename.size() - 5 ); // ".info" を外す

#ifdef _DEBUG
                std::cout << "\ninfo  = " << path_info << std::endl;
                std::cout << "path = " << path_file << std::endl;
                std::cout << "mtime = " << MISC::timettostr( mtime ) << std::endl;
                std::cout << "days = " << days << std::endl;
#endif

                // キャッシュ削除
                if( CACHE::file_exists( path_file ) == CACHE::EXIST_FILE ) unlink( path_file.c_str() );

                // info 削除
                if( CACHE::file_exists( path_info ) == CACHE::EXIST_FILE ) unlink( path_info.c_str() );
            }

#ifdef _DEBUG
            std::cout << "DelImgCacheDiag::main_thread end\n";
#endif

            hide();
        }

        static void* launcher( void* dat ){
            DelImgCacheDiag* tt = ( DelImgCacheDiag * ) dat;
            tt->main_thread();
            return 0;
        }

        DelImgCacheDiag()
        : m_label( "画像キャッシュ削除中・・・\n\nしばらくお待ち下さい" ),
        m_thread( 0 ){

            add_button( Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL )
            ->signal_clicked().connect( sigc::mem_fun(*this, &DelImgCacheDiag::slot_cancel_clicked ) );

            set_title( "JD 画像キャッシュ削除中" );

            const int mrg = 8;
            get_vbox()->set_spacing( mrg );
            set_border_width( mrg );
            get_vbox()->pack_start( m_label, Gtk::PACK_SHRINK );
            show_all_children();
        }

        ~DelImgCacheDiag(){
#ifdef _DEBUG
            std::cout << "DelImgCacheDiag::~DelImgCacheDiag\n";
#endif
            slot_cancel_clicked();
        }

        virtual bool on_expose_event( GdkEventExpose* event ){
#ifdef _DEBUG
            std::cout << "DelImgCacheDiag::on_expose_event\n";
#endif

            // スレッドを起動してキャッシュ削除
            if( ! m_thread ){
                m_stop = false;
                int status;
                if( ( status = pthread_create( &m_thread, NULL,  ( FUNC ) launcher, ( void * ) this ) )){
                    MISC::ERRMSG( std::string( "pthread_create failed : " ) + strerror( status ) );
                }
            }

            return Gtk::Dialog::on_expose_event( event );
        }

    };

}

#endif
