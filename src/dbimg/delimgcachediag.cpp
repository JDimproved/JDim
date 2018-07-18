// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "delimgcachediag.h"

#ifdef _DEBUG
#include "jdlib/misctime.h"
#endif

#include "cache.h"

#include "config/globalconf.h"

#include "jdlib/miscutil.h"
#include "jdlib/miscmsg.h"

#include <sys/time.h>


using namespace DBIMG;


DelImgCacheDiag::DelImgCacheDiag()
    : m_label( "画像キャッシュ削除中・・・\n\nしばらくお待ち下さい" )
{
    add_button( Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL )
    ->signal_clicked().connect( sigc::mem_fun(*this, &DelImgCacheDiag::slot_cancel_clicked ) );

    set_title( "JDim 画像キャッシュ削除中" );

    const int mrg = 8;
    get_vbox()->set_spacing( mrg );
    set_border_width( mrg );
    get_vbox()->pack_start( m_label, Gtk::PACK_SHRINK );
    show_all_children();
}


DelImgCacheDiag::~DelImgCacheDiag()
{
#ifdef _DEBUG
    std::cout << "DelImgCacheDiag::~DelImgCacheDiag\n";
#endif

    slot_cancel_clicked();
}


#if GTKMM_CHECK_VERSION(3,0,0)
bool DelImgCacheDiag::on_draw( const Cairo::RefPtr< Cairo::Context >& cr )
{
#ifdef _DEBUG
    std::cout << "DelImgCacheDiag::on_draw\n";
#endif

    launch_thread();
    return Gtk::Dialog::on_draw( cr );
}
#else
bool DelImgCacheDiag::on_expose_event( GdkEventExpose* event )
{
#ifdef _DEBUG
    std::cout << "DelImgCacheDiag::on_expose_event\n";
#endif

    launch_thread();
    return Gtk::Dialog::on_expose_event( event );
}
#endif // GTKMM_CHECK_VERSION(3,0,0)


void DelImgCacheDiag::launch_thread()
{
    // スレッドを起動してキャッシュ削除
    if( !m_thread.is_running() ) {
        m_stop = false;
        if( !m_thread.create( static_cast< STARTFUNC >( launcher ),
                              static_cast< void* >( this ),
                              JDLIB::NODETACH ) ) {
            MISC::ERRMSG( "DelImgCacheDiag::launch_thread : could not start thread" );
        }
    }
}


void* DelImgCacheDiag::launcher( void* dat )
{
    DelImgCacheDiag* tt = ( DelImgCacheDiag * ) dat;
    tt->main_thread();
    return 0;
}


// 画像キャッシュ削除スレッド
void DelImgCacheDiag::main_thread()
{
#ifdef _DEBUG
    std::cout << "DelImgCacheDiag::main_thread start\n";
#endif

    time_t days = 0;

    // info と 画像キャッシュ
    std::list< std::string >list_infofile;
    std::string path_info_root = CACHE::path_img_info_root();
    list_infofile = CACHE::get_filelist( path_info_root );

    std::list< std::string >::iterator it_info = list_infofile.begin();
    for(; ! m_stop && it_info != list_infofile.end(); ++it_info ){

        std::string path_info = path_info_root + ( *it_info );

        // info ファイルの作成日時を調べて、設定した日時よりも古かったら消す
        if( CONFIG::get_del_img_day() > 0 ){
            days = get_days( path_info );
            if( days < CONFIG::get_del_img_day() ) continue;
        }

        // 画像ファイル名取得
        std::string filename = MISC::get_filename( path_info );
        std::string path_file = CACHE::path_img_root() + filename.substr( 0, filename.size() - 5 ); // ".info" を外す

#ifdef _DEBUG
        std::cout << "\ninfo  = " << path_info << std::endl;
        std::cout << "path = " << path_file << std::endl;
        std::cout << "days = " << days << std::endl;
#endif

        // キャッシュ削除
        if( CACHE::file_exists( path_file ) == CACHE::EXIST_FILE ) unlink( to_locale_cstr( path_file ) );

        // info 削除
        if( CACHE::file_exists( path_info ) == CACHE::EXIST_FILE ) unlink( to_locale_cstr( path_info ) );
    }

    // あぼーん情報

#ifdef _DEBUG
    std::cout << "\ndelete abone info\n";
#endif

    path_info_root = CACHE::path_img_abone_root();
    list_infofile = CACHE::get_filelist( path_info_root );

    it_info = list_infofile.begin();
    for(; ! m_stop && it_info != list_infofile.end(); ++it_info ){

        std::string path_info = path_info_root + ( *it_info );

        // info ファイルの作成日時を調べて、設定した日時よりも古かったら消す
        if( CONFIG::get_del_imgabone_day() > 0 ){
            days = get_days( path_info );
            if( days < CONFIG::get_del_imgabone_day() ) continue;
        }

#ifdef _DEBUG
        std::cout << "\nabone info  = " << path_info << std::endl;
        std::cout << "days = " << days << std::endl;
#endif
        // info 削除
        if( CACHE::file_exists( path_info ) == CACHE::EXIST_FILE ) unlink( to_locale_cstr( path_info ) );
    }

#ifdef _DEBUG
    std::cout << "DelImgCacheDiag::main_thread end\n";
#endif

    // アクセシビリティをonにした時に別スレッドでhide()すると
    // フリーズするので、dispatchしてメインスレッドでhide()する
    dispatch();
}


void DelImgCacheDiag::callback_dispatch()
{
#ifdef _DEBUG
    std::cout << "DelImgCacheDiag::callback_dispatch\n";
#endif

    hide();
}


//
// ファイルの作成日からの経過日数を計算
//
// エラーの時ははマイナスの値が戻る
//
time_t DelImgCacheDiag::get_days( const std::string& path )
{
        struct timeval tv;
        struct timezone tz;
        gettimeofday( &tv, &tz );
        time_t mtime = CACHE::get_filemtime( path );
        if( ! mtime ) return -1;

        return ( tv.tv_sec - mtime ) / ( 60 * 60 * 24 );
}


void DelImgCacheDiag::wait()
{
#ifdef _DEBUG
    std::cout << "DelImgCacheDiag::wait\n";
#endif
    m_thread.join();
}


void DelImgCacheDiag::slot_cancel_clicked()
{
#ifdef _DEBUG
    std::cout << "DelImgCacheDiag::slot_cancel_clicked\n";
#endif

    m_stop = true;
    wait();
}
