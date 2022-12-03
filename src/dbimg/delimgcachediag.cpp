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

#include <glib/gi18n.h>

#include <ctime>
#include <system_error>


using namespace DBIMG;


DelImgCacheDiag::DelImgCacheDiag()
    : m_label( "画像キャッシュ削除中・・・\n\nしばらくお待ち下さい" )
    , m_stop{ true }
{
    add_button( g_dgettext( GTK_DOMAIN, "_Cancel" ), Gtk::RESPONSE_CANCEL )
    ->signal_clicked().connect( sigc::mem_fun(*this, &DelImgCacheDiag::slot_cancel_clicked ) );

    set_title( "JDim 画像キャッシュ削除中" );

    const int mrg = 8;
    get_content_area()->set_spacing( mrg );
    set_border_width( mrg );
    get_content_area()->pack_start( m_label, Gtk::PACK_SHRINK );
    show_all_children();
}


DelImgCacheDiag::~DelImgCacheDiag()
{
#ifdef _DEBUG
    std::cout << "DelImgCacheDiag::~DelImgCacheDiag\n";
#endif

    // m_thread.joinable() == true のときスレッドを破棄すると強制終了するため待機処理を入れる
    slot_cancel_clicked();
}


bool DelImgCacheDiag::on_draw( const Cairo::RefPtr< Cairo::Context >& cr )
{
#ifdef _DEBUG
    std::cout << "DelImgCacheDiag::on_draw\n";
#endif

    launch_thread();
    return Gtk::Dialog::on_draw( cr );
}


void DelImgCacheDiag::launch_thread()
{
    // スレッドを起動してキャッシュ削除
    if( ! m_thread.joinable() ) {
        m_stop = false;
        try {
            m_thread = std::thread( &DelImgCacheDiag::main_thread, this );
        }
        catch( std::system_error& ) {
            MISC::ERRMSG( "DelImgCacheDiag::launch_thread : could not start thread" );
        }
    }
}


// 画像キャッシュ削除スレッド
void DelImgCacheDiag::main_thread()
{
#ifdef _DEBUG
    std::cout << "DelImgCacheDiag::main_thread start\n";
#endif

    int days = 0;

    // info と 画像キャッシュ
    std::string path_info_root = CACHE::path_img_info_root();
    std::list<std::string> list_infofile = CACHE::get_filelist( path_info_root );

    for( const std::string& infofile : list_infofile ) {
        if( m_stop ) break;

        const std::string path_info = path_info_root + infofile;

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

    for( const std::string& infofile : list_infofile ) {
        if( m_stop ) break;

        const std::string path_info = path_info_root + infofile;

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
//static member
int DelImgCacheDiag::get_days( const std::string& path )
{
    const std::time_t current = std::time( nullptr );
    const std::time_t mtime = CACHE::get_filemtime( path );
    if( ! mtime ) return -1;

    return static_cast<int>( ( current - mtime ) / ( 60 * 60 * 24 ) );
}


void DelImgCacheDiag::wait()
{
#ifdef _DEBUG
    std::cout << "DelImgCacheDiag::wait\n";
#endif
    if( m_thread.joinable() ) m_thread.join();
}


void DelImgCacheDiag::slot_cancel_clicked()
{
#ifdef _DEBUG
    std::cout << "DelImgCacheDiag::slot_cancel_clicked\n";
#endif

    m_stop = true;
    wait();
}
