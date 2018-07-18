// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"
#include "gtkmmversion.h"

#include "iconmanager.h"
#include "iconfiles.h"

#include "cache.h"

#include "jd16.h"
#include "jd32.h"
#include "jd48.h"
#include "jd96.h"

#include "bkmark_update.h"
#include "bkmark.h"
#include "bkmark_broken_subject.h"
#include "bkmark_thread.h"

#include "update.h"
#include "newthread.h"
#include "newthread_hour.h"
#include "broken_subject.h"
#include "check.h"
#include "down.h"
#include "write.h"
#include "post.h"
#include "post_refer.h"
#include "loading.h"
#include "loading_stop.h"

#include "dir.h"
#include "favorite.h"
#include "hist.h"
#include "hist_board.h"
#include "hist_close.h"
#include "hist_closeboard.h"
#include "hist_closeimg.h"

#include "board.h"
#include "board_update.h"
#include "board_updated.h"
#include "thread.h"
#include "thread_update.h"
#include "thread_updated.h"
#include "thread_old.h"
#include "image.h"
#include "link.h"
#include "info.h"

#if !GTKMM_CHECK_VERSION(2,5,0)
#include "play.h"
#endif

#include <cstring>

ICON::ICON_Manager* instance_icon_manager = NULL;


ICON::ICON_Manager* ICON::get_icon_manager()
{
    if( ! instance_icon_manager ) instance_icon_manager = new ICON::ICON_Manager();
    assert( instance_icon_manager );

    return instance_icon_manager;
}


void ICON::delete_icon_manager()
{
    if( instance_icon_manager ) delete instance_icon_manager;
    instance_icon_manager = NULL;
}


Glib::RefPtr< Gdk::Pixbuf > ICON::get_icon( int id )
{
    return get_icon_manager()->get_icon( id );
}


///////////////////////////////////////////////

using namespace ICON;


ICON_Manager::ICON_Manager()
{
#if GTKMM_CHECK_VERSION(3,0,0)
    struct Dummy : public Gtk::Image
    {
        Glib::RefPtr< Gdk::Pixbuf > render_icon( Gtk::BuiltinStockID stock, Gtk::BuiltinIconSize size )
        {
            return render_icon_pixbuf( stock, size );
        }
    } m_dummy;
#else
    Gtk::Image m_dummy;
#endif

    m_list_icons.resize( NUM_ICONS );

    m_list_icons[ ICON::JD16 ] =  Gdk::Pixbuf::create_from_inline( sizeof( icon_jd16 ), icon_jd16 );
    m_list_icons[ ICON::JD32 ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_jd32 ), icon_jd32 );
    m_list_icons[ ICON::JD48 ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_jd48 ), icon_jd48 );
    m_list_icons[ ICON::JD96 ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_jd96 ), icon_jd96 );

    // サイドバーで使用するアイコン
    m_list_icons[ ICON::DIR ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_dir ), icon_dir );
    m_list_icons[ ICON::IMAGE ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_image ), icon_image );
    m_list_icons[ ICON::LINK ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_link ), icon_link );

    // サイドバーやタブで使用するアイコン
    m_list_icons[ ICON::BOARD ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_board ), icon_board );
    m_list_icons[ ICON::BOARD_UPDATE ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_board_update ), icon_board_update );
    m_list_icons[ ICON::THREAD ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_thread ), icon_thread );
    m_list_icons[ ICON::THREAD_UPDATE ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_thread_update ), icon_thread_update );
    m_list_icons[ ICON::THREAD_OLD ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_thread_old ), icon_thread_old );

    // タブで使用するアイコン
    m_list_icons[ ICON::BOARD_UPDATED ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_board_updated ), icon_board_updated );
    m_list_icons[ ICON::THREAD_UPDATED ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_thread_updated ), icon_thread_updated );
    m_list_icons[ ICON::LOADING ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_loading ), icon_loading );
    m_list_icons[ ICON::LOADING_STOP ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_loading_stop ), icon_loading_stop );

    // スレ一覧で使用するアイコン
    m_list_icons[ ICON::BKMARK_UPDATE ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_bkmark_update ), icon_bkmark_update );
    m_list_icons[ ICON::BKMARK_BROKEN_SUBJECT ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_bkmark_broken_subject ), icon_bkmark_broken_subject );
    m_list_icons[ ICON::BKMARK ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_bkmark ), icon_bkmark );
    m_list_icons[ ICON::UPDATE ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_update ), icon_update );
    m_list_icons[ ICON::NEWTHREAD ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_newthread ), icon_newthread );
    m_list_icons[ ICON::NEWTHREAD_HOUR ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_newthread_hour ), icon_newthread_hour );
    m_list_icons[ ICON::BROKEN_SUBJECT ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_broken_subject ), icon_broken_subject );
    m_list_icons[ ICON::CHECK ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_check ), icon_check );
    m_list_icons[ ICON::OLD ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_down ), icon_down );
    m_list_icons[ ICON::INFO ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_info ), icon_info );

    // スレビューで使用するアイコン
    m_list_icons[ ICON::BKMARK_THREAD ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_bkmark_thread ), icon_bkmark_thread );
    m_list_icons[ ICON::POST ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_post ), icon_post );
    m_list_icons[ ICON::POST_REFER ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_post_refer ), icon_post_refer );

    // その他
    m_list_icons[ ICON::DOWN ] = m_list_icons[ ICON::OLD ];

    m_list_icons[ ICON::TRANSPARENT ] = Gdk::Pixbuf::create( Gdk::COLORSPACE_RGB, true, 8, 1, 1 );
    m_list_icons[ ICON::TRANSPARENT ]->fill( 0 );


    //////////////////////////////
    // ツールバーのアイコン

    // 共通
    m_list_icons[ ICON::SEARCH_PREV ]  = m_dummy.render_icon( Gtk::Stock::GO_UP, Gtk::ICON_SIZE_MENU );
    m_list_icons[ ICON::SEARCH_NEXT ]  = m_dummy.render_icon( Gtk::Stock::GO_DOWN, Gtk::ICON_SIZE_MENU );
    m_list_icons[ ICON::STOPLOADING ]  = m_dummy.render_icon( Gtk::Stock::STOP, Gtk::ICON_SIZE_MENU );
    m_list_icons[ ICON::WRITE ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_write ), icon_write );
    m_list_icons[ ICON::RELOAD ]  = m_dummy.render_icon( Gtk::Stock::REFRESH, Gtk::ICON_SIZE_MENU );
    m_list_icons[ ICON::APPENDFAVORITE ]  = m_dummy.render_icon( Gtk::Stock::COPY, Gtk::ICON_SIZE_MENU );
    m_list_icons[ ICON::DELETE ]  = m_dummy.render_icon( Gtk::Stock::DELETE, Gtk::ICON_SIZE_MENU );
    m_list_icons[ ICON::QUIT ]  = m_dummy.render_icon( Gtk::Stock::CLOSE, Gtk::ICON_SIZE_MENU );
    m_list_icons[ ICON::BACK ]  = m_dummy.render_icon( Gtk::Stock::GO_BACK, Gtk::ICON_SIZE_MENU );
    m_list_icons[ ICON::FORWARD ]  = m_dummy.render_icon( Gtk::Stock::GO_FORWARD, Gtk::ICON_SIZE_MENU );
    m_list_icons[ ICON::LOCK ]  = m_dummy.render_icon( Gtk::Stock::NO, Gtk::ICON_SIZE_MENU );

    // メイン
    m_list_icons[ ICON::BBSLISTVIEW ] = m_list_icons[ ICON::DIR ];
    m_list_icons[ ICON::FAVORITEVIEW ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_favorite ), icon_favorite );
    m_list_icons[ ICON::HISTVIEW ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_hist ), icon_hist );
    m_list_icons[ ICON::HIST_BOARDVIEW ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_hist_board ), icon_hist_board );
    m_list_icons[ ICON::HIST_CLOSEVIEW ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_hist_close ), icon_hist_close );
    m_list_icons[ ICON::HIST_CLOSEBOARDVIEW ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_hist_closeboard ), icon_hist_closeboard );
    m_list_icons[ ICON::HIST_CLOSEIMGVIEW ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_hist_closeimg ), icon_hist_closeimg );
    m_list_icons[ ICON::BOARDVIEW ] = m_list_icons[ ICON::BOARD ];
    m_list_icons[ ICON::ARTICLEVIEW ] = m_list_icons[ ICON::THREAD ];
    m_list_icons[ ICON::IMAGEVIEW ] = m_list_icons[ ICON::IMAGE ];
    m_list_icons[ ICON::GO ]  = m_dummy.render_icon( Gtk::Stock::JUMP_TO, Gtk::ICON_SIZE_MENU );
    m_list_icons[ ICON::UNDO ]  = m_dummy.render_icon( Gtk::Stock::UNDO, Gtk::ICON_SIZE_MENU );
    m_list_icons[ ICON::REDO ]  = m_dummy.render_icon( Gtk::Stock::REDO, Gtk::ICON_SIZE_MENU );

    // サイドバー
    m_list_icons[ ICON::CHECK_UPDATE_ROOT ]  = m_dummy.render_icon( Gtk::Stock::REFRESH, Gtk::ICON_SIZE_MENU );
    m_list_icons[ ICON::CHECK_UPDATE_OPEN_ROOT ]  = Gdk::Pixbuf::create_from_inline( sizeof( icon_thread ), icon_thread );

    // スレビュー
    m_list_icons[ ICON::SEARCH ]  = m_dummy.render_icon( Gtk::Stock::FIND, Gtk::ICON_SIZE_MENU );
#if GTKMM_CHECK_VERSION(2,5,0)
    m_list_icons[ ICON::LIVE ]  = m_dummy.render_icon( Gtk::Stock::MEDIA_PLAY, Gtk::ICON_SIZE_MENU );
#else
    m_list_icons[ ICON::LIVE ] = Gdk::Pixbuf::create_from_inline( sizeof( icon_play ), icon_play );
#endif

    // 検索バー
    m_list_icons[ ICON::CLOSE_SEARCH ]  = m_dummy.render_icon( Gtk::Stock::UNDO, Gtk::ICON_SIZE_MENU );
    m_list_icons[ ICON::CLEAR_SEARCH ]  = m_dummy.render_icon( Gtk::Stock::CLEAR, Gtk::ICON_SIZE_MENU );
    m_list_icons[ ICON::SEARCH_AND ]  = m_dummy.render_icon( Gtk::Stock::CUT, Gtk::ICON_SIZE_MENU );
    m_list_icons[ ICON::SEARCH_OR ]  = m_dummy.render_icon( Gtk::Stock::ADD, Gtk::ICON_SIZE_MENU );

    // 書き込みビュー
    m_list_icons[ ICON::PREVIEW ] = m_list_icons[ ICON::THREAD ];
    m_list_icons[ ICON::INSERTTEXT ]  = m_dummy.render_icon( Gtk::Stock::OPEN, Gtk::ICON_SIZE_MENU );

    load_theme();
}


ICON_Manager::~ICON_Manager()
{
#ifdef _DEBUG
    std::cout << "ICON::~ICON_Manager\n";
#endif
}


Glib::RefPtr< Gdk::Pixbuf > ICON_Manager::get_icon( const int id )
{
    return m_list_icons[ id ];
}


//
// アイコンテーマ読み込み
//
void ICON_Manager::load_theme()
{
    if( CACHE::file_exists( CACHE::path_theme_icon_root() ) != CACHE::EXIST_DIR ) return;

    const std::list< std::string > files = CACHE::get_filelist( CACHE::path_theme_icon_root() );
    if( ! files.size() ) return;

#ifdef _DEBUG
    std::cout << "ICON::load_theme\n";
#endif

    std::list< std::string >::const_iterator it = files.begin();
    for(; it != files.end(); ++it ){

#ifdef _DEBUG
        std::cout << *it << std::endl;
#endif

        int id = 0;

        // 拡張子を削除
        std::string filename = (*it);
        size_t i = (*it).rfind( '.' );
        if( i != std::string::npos ) filename = filename.substr( 0, i );

        while( iconfiles[ id ][ 0 ] != '\0' ){

            if( iconfiles[ id ][ 0 ] == filename[ 0 ]
                && filename.compare( iconfiles[ id ] ) == 0 ){
#ifdef _DEBUG
                std::cout << "hit : " << iconfiles[ id ] << " id = " << id << std::endl;
#endif
                m_list_icons[ id ]  = Gdk::Pixbuf::create_from_file( CACHE::path_theme_icon_root() + (*it) );
                break;
            }

            ++id;
        }
    }
}
