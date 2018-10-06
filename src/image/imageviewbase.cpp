// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "imageadmin.h"
#include "imageviewbase.h"
#include "imageareabase.h"

#include "skeleton/msgdiag.h"

#include "dbtree/interface.h"

#include "dbimg/imginterface.h"
#include "dbimg/img.h"

#include "jdlib/miscutil.h"
#include "jdlib/miscgtk.h"

#include "control/controlutil.h"
#include "control/controlid.h"

#include "config/globalconf.h"

#include "cache.h"
#include "command.h"
#include "sharedbuffer.h"
#include "global.h"
#include "type.h"
#include "prefdiagfactory.h"
#include "usrcmdmanager.h"
#include "httpcode.h"
#include "session.h"

#include <sstream>


#define SIZE_MENU { 25, 50, 75, 100, 150, 200, 400 }



using namespace IMAGE;


ImageViewBase::ImageViewBase( const std::string& url, const std::string& arg1, const std::string& arg2 )
    : SKELETON::View( url ),
      m_wait( false ),
      m_loading( false ),
      m_under_mouse( false ),
      m_enable_menuslot( true )
{
    // 高速化のためデータベースに直接アクセス
    m_img =  DBIMG::get_img( get_url() );
    assert( m_img );

    // マウスジェスチャ可能
    set_enable_mg( true );
}



ImageViewBase::~ImageViewBase()
{
#ifdef _DEBUG    
    std::cout << "ImageViewBase::~ImageViewBase : " << get_url() << std::endl;
#endif
}


SKELETON::Admin* ImageViewBase::get_admin()
{
    return IMAGE::get_admin();
}


//
// 親ウィンドウを取得
//
Gtk::Window* ImageViewBase::get_parent_win()
{
    return IMAGE::get_admin()->get_win();
}


//
// 共通セットアップ
//
void ImageViewBase::setup_common()
{
    const int default_width = 200;
    const int default_height = 50;

    set_width_client( default_width );
    set_height_client( default_height );

    // focus 可、モーションキャプチャ可
#if GTKMM_CHECK_VERSION(2,18,0)
    m_event.set_can_focus( true );
#else
    m_event.set_flags( m_event.get_flags() | Gtk::CAN_FOCUS );
#endif
    m_event.add_events( Gdk::POINTER_MOTION_MASK );

    m_event.signal_button_press_event().connect( sigc::mem_fun( *this, &ImageViewBase::slot_button_press ) );
    m_event.signal_button_release_event().connect( sigc::mem_fun( *this, &ImageViewBase::slot_button_release ) );
    m_event.signal_motion_notify_event().connect(  sigc::mem_fun( *this, &ImageViewBase::slot_motion_notify ) );
    m_event.signal_key_press_event().connect( sigc::mem_fun(*this, &ImageViewBase::slot_key_press ) );
    m_event.signal_scroll_event().connect( sigc::mem_fun(*this, &ImageViewBase::slot_scroll_event ) );
    m_event.signal_enter_notify_event().connect(  sigc::mem_fun( *this, &ImageViewBase::slot_enter_notify_event ) );
    m_event.signal_leave_notify_event().connect(  sigc::mem_fun( *this, &ImageViewBase::slot_leave_notify_event ) );

    m_event.grab_focus();

    // ポップアップメニューの設定
    // アクショングループを作ってUIマネージャに登録
    action_group() = Gtk::ActionGroup::create();
    action_group()->add( Gtk::Action::create( "CancelMosaic", "CancelMosaic"),
                         sigc::mem_fun( *this, &ImageViewBase::slot_cancel_mosaic ) );
    action_group()->add( Gtk::Action::create( "ShowLargeImg", "サイズが大きい画像を表示(_G)"),
                         sigc::mem_fun( *this, &ImageViewBase::slot_show_large_img ) );
    action_group()->add( Gtk::Action::create( "LoadStop", "StopLoading" ), sigc::mem_fun( *this, &ImageViewBase::stop ) );
    action_group()->add( Gtk::Action::create( "Reload", "強制再読み込み(_E)"), sigc::mem_fun( *this, &ImageViewBase::slot_reload_force ) );
    action_group()->add( Gtk::Action::create( "AppendFavorite", "AppendFavorite"), sigc::mem_fun( *this, &ImageViewBase::slot_favorite ) );

    action_group()->add( Gtk::Action::create( "ZoomFitImage", "ZoomFitImage" ),
                         sigc::mem_fun( *this, &ImageViewBase::slot_fit_win ) );
    action_group()->add( Gtk::Action::create( "ZoomInImage", "ZoomInImage" ),
                         sigc::mem_fun( *this, &ImageViewBase::slot_zoom_in ) );
    action_group()->add( Gtk::Action::create( "ZoomOutImage", "ZoomOutImage" ),
                         sigc::mem_fun( *this, &ImageViewBase::slot_zoom_out ) );
    action_group()->add( Gtk::Action::create( "OrgSizeImage", "OrgSizeImage" ),
                         sigc::bind< int >( sigc::mem_fun( *this, &ImageViewBase::slot_resize_image ), 100 ) );

    action_group()->add( Gtk::Action::create( "Size_Menu", "サイズ変更(_R)" ) );

    // サイズ
    unsigned int size[] = SIZE_MENU;
    for( unsigned int i = 0; i < sizeof( size )/sizeof( unsigned int ) ; ++i ){
        int tmp_size = size[ i ];
        std::string str_size = MISC::itostr( tmp_size );
        //ショートカットは、1から始まる
        std::string str_shortcut = "(_" + MISC::itostr( i+1 ) + ")";
        Glib::RefPtr< Gtk::Action > action = Gtk::Action::create( "Size" + str_size, str_size + "%" + str_shortcut );
        action_group()->add( action, sigc::bind< int >( sigc::mem_fun( *this, &ImageViewBase::slot_resize_image ), tmp_size ) );
    }

    action_group()->add( Gtk::Action::create( "Move_Menu", ITEM_NAME_GO "(_M)" ) );
    action_group()->add( Gtk::Action::create( "MoveHead", "先頭に移動(_H)" ), sigc::mem_fun( *this, &ImageViewBase::slot_move_head ) );
    action_group()->add( Gtk::Action::create( "MoveTail", "最後に移動(_T)" ), sigc::mem_fun( *this, &ImageViewBase::slot_move_tail ) );

    action_group()->add( Gtk::Action::create( "Quit", "Quit" ), sigc::mem_fun( *this, &ImageViewBase::close_view ) );

    action_group()->add( Gtk::Action::create( "Close_Menu", "複数の画像を閉じる(_L)" ) );
    action_group()->add( Gtk::Action::create( "CloseOther", "他の画像(_O)" ), sigc::mem_fun( *this, &ImageViewBase::slot_close_other_views ) );
    action_group()->add( Gtk::Action::create( "CloseLeft", "左←の画像(_L)" ), sigc::mem_fun( *this, &ImageViewBase::slot_close_left_views ) );
    action_group()->add( Gtk::Action::create( "CloseRight", "右→の画像(_R)" ), sigc::mem_fun( *this, &ImageViewBase::slot_close_right_views ) );
    action_group()->add( Gtk::Action::create( "CloseError404", "エラー画像(404,403のみ)(_E)" ), sigc::mem_fun( *this, &ImageViewBase::slot_close_error_views ) );
    action_group()->add( Gtk::Action::create( "CloseError503", "エラー画像(timeout,503以外)(_T)" ), sigc::mem_fun( *this, &ImageViewBase::slot_close_notimeout_error_views ) );
    action_group()->add( Gtk::Action::create( "CloseErrorAll", "エラー画像(読込み中含め全て)(_W)" ), sigc::mem_fun( *this, &ImageViewBase::slot_close_all_error_views ) );
    action_group()->add( Gtk::Action::create( "CloseNoError", "エラー以外の画像(_N)" ), sigc::mem_fun( *this, &ImageViewBase::slot_close_noerror_views ) );
    action_group()->add( Gtk::Action::create( "CloseAll", "全ての画像(_A)" ), sigc::mem_fun( *this, &ImageViewBase::slot_close_all_views ) );

    action_group()->add( Gtk::ToggleAction::create( "LockTab", "タブをロックする(_K)", std::string(), false ),
                         sigc::mem_fun( *this, &ImageViewBase::slot_lock ) );

    action_group()->add( Gtk::Action::create( "OpenBrowser", ITEM_NAME_OPEN_BROWSER "(_W)" ),
                         sigc::mem_fun( *this, &ImageViewBase::slot_open_browser ) );
    action_group()->add( Gtk::Action::create( "OpenCacheBrowser", ITEM_NAME_OPEN_CACHE_BROWSER "(_X)" ),
                         sigc::mem_fun( *this, &ImageViewBase::slot_open_cache_browser ) );
    action_group()->add( Gtk::Action::create( "OpenRef", "参照元のレスを開く(_O)"), sigc::mem_fun( *this, &ImageViewBase::slot_open_ref ) );
    action_group()->add( Gtk::Action::create( "CopyURL", ITEM_NAME_COPY_URL "(_U)" ), sigc::mem_fun( *this, &ImageViewBase::slot_copy_url ) );
    action_group()->add( Gtk::Action::create( "Save", "Save"), sigc::mem_fun( *this, &ImageViewBase::slot_save ) );
    action_group()->add( Gtk::Action::create( "SaveAll", "全ての画像を保存(_A)..."), sigc::mem_fun( *this, &ImageViewBase::slot_save_all ) );

    action_group()->add( Gtk::Action::create( "DeleteMenu", "Delete" ) );    
    action_group()->add( Gtk::Action::create( "DeleteImage", "削除する(_D)"), sigc::mem_fun( *this, &ImageViewBase::delete_view ) );
    action_group()->add( Gtk::ToggleAction::create( "ProtectImage", "キャッシュを保護する(_H)", std::string(), false ),
                         sigc::mem_fun( *this, &ImageViewBase::slot_toggle_protectimage ) );

    action_group()->add( Gtk::Action::create( "AboneImage", "画像をあぼ〜んする(_B)"), sigc::mem_fun( *this, &ImageViewBase::slot_abone_img ) );

    action_group()->add( Gtk::Action::create( "PreferenceImage", "PreferenceImage"), sigc::mem_fun( *this, &ImageViewBase::show_preference ) );
    action_group()->add( Gtk::Action::create( "Preference", "プロパティ(_P)..."), sigc::mem_fun( *this, &ImageViewBase::show_preference ) );

    const std::string usrcmd = CORE::get_usrcmd_manager()->create_usrcmd_menu( action_group() );
    const int usrcmd_size = CORE::get_usrcmd_manager()->get_size();
    for( int i = 0; i < usrcmd_size; ++i ){
        Glib::RefPtr< Gtk::Action > act = CORE::get_usrcmd_manager()->get_action( action_group(), i );
        if( act ) act->signal_activate().connect(
            sigc::bind< int >( sigc::mem_fun( *this, &ImageViewBase::slot_usrcmd ), i ) );
    }

    ui_manager() = Gtk::UIManager::create();    
    ui_manager()->insert_action_group( action_group() );

    // 画像ビューのメニュー
    const std::string menu = 

    "<popup name='popup_menu'>"

    "<menuitem action='CancelMosaic'/>"
    "<menuitem action='ShowLargeImg'/>"
    "<separator/>"

    "<menuitem action='Quit'/>"
    "<separator/>"

    "<menu action='Size_Menu'>"
    "<menuitem action='Size25'/>"
    "<menuitem action='Size50'/>"
    "<menuitem action='Size75'/>"
    "<menuitem action='Size100'/>"
    "<menuitem action='Size150'/>"
    "<menuitem action='Size200'/>"
    "<menuitem action='Size400'/>"
    "</menu>"
    "<menuitem action='OrgSizeImage'/>"
    "<menuitem action='ZoomFitImage'/>"
    "<menuitem action='ZoomInImage'/>"
    "<menuitem action='ZoomOutImage'/>"
    "<separator/>"

    "<menuitem action='OpenBrowser'/>"
    "<menuitem action='OpenCacheBrowser'/>"
    "<menuitem action='OpenRef'/>"
    + usrcmd
    + std::string( 
    "<separator/>"

    "<menuitem action='CopyURL'/>"
    "<separator/>"

    "<menuitem action='AppendFavorite'/>"
    "<menuitem action='Save'/>"
    "<separator/>"

    "<menuitem action='LoadStop'/>"
    "<menuitem action='Reload'/>"
    "<separator/>"

    "<menuitem action='ProtectImage'/>"
    "<menu action='DeleteMenu'>"
    "<menuitem action='DeleteImage'/>"
    "</menu>"

    "<separator/>"
    "<menuitem action='AboneImage'/>"

    "<separator/>"

    "<menuitem action='PreferenceImage'/>"

    "</popup>"
    );

    // アイコンのメニュー
    const std::string menu_icon = 
    
    "<popup name='popup_menu_icon'>"

    "<menu action='Move_Menu'>"
    "<menuitem action='MoveHead'/>"
    "<menuitem action='MoveTail'/>"
    "</menu>"
    "<separator/>"

    "<menuitem action='LockTab'/>"
    "<separator/>"

    "<menuitem action='Quit'/>"
    "<separator/>"

    "<menu action='Close_Menu'>"
    "<menuitem action='CloseAll'/>"
    "<menuitem action='CloseOther'/>"
    "<menuitem action='CloseLeft'/>"
    "<menuitem action='CloseRight'/>"
    "<separator/>"
    "<menuitem action='CloseError404'/>"
    "<menuitem action='CloseError503'/>"
    "<menuitem action='CloseErrorAll'/>"
    "<separator/>"
    "<menuitem action='CloseNoError'/>"
    "</menu>"
    "<separator/>"

    "<menuitem action='SaveAll'/>"
    "<separator/>"

    "<menuitem action='OpenBrowser'/>"
    "<menuitem action='OpenCacheBrowser'/>"
    "<menuitem action='OpenRef'/>"
    "<separator/>"

    "<menuitem action='CopyURL'/>"
    "<separator/>"

    "<menuitem action='AppendFavorite'/>"
    "<menuitem action='Save'/>"
    "<separator/>"

    "<menuitem action='LoadStop'/>"
    "<menuitem action='Reload'/>"
    "<separator/>"

    "<menuitem action='ProtectImage'/>"
    "<menu action='DeleteMenu'>"
    "<menuitem action='DeleteImage'/>"
    "</menu>"
    "<separator/>"

    "<separator/>"
    "<menuitem action='AboneImage'/>"

    "<separator/>"
    "<menuitem action='Preference'/>"

    "</popup>";

    // 画像ポップアップのメニュー
    const std::string menu_popup = 

    "<popup name='popup_menu_popup'>"

    "<menuitem action='CancelMosaic'/>"
    "<separator/>"

    "<menuitem action='Quit'/>"
    "<separator/>"

    "<menuitem action='CopyURL'/>"
    "<separator/>"

    "<menuitem action='Save'/>"
    "<separator/>"

    "<menuitem action='ProtectImage'/>"
    "<menu action='DeleteMenu'>"
    "<menuitem action='DeleteImage'/>"
    "</menu>"

    "<separator/>"
    "<menuitem action='AboneImage'/>"

    "<separator/>"

    "<menuitem action='Preference'/>"

    "</popup>"
    ;


    ui_manager()->add_ui_from_string(
        "<ui>"
        + menu
        + menu_icon
        + menu_popup
        + "</ui>"
        );

    // ポップアップメニューにキーアクセレータやマウスジェスチャを表示
    Gtk::Menu* popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu" ) );
    CONTROL::set_menu_motion( popupmenu );

    popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_icon" ) );
    CONTROL::set_menu_motion( popupmenu );

    popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_popup" ) );
    CONTROL::set_menu_motion( popupmenu );
}


//
// ImageAreaBaseのセット
// 
void ImageViewBase::set_imagearea( ImageAreaBase* imagearea )
{
    assert( imagearea );

    m_imagearea.clear();
    m_imagearea = imagearea;

    set_width_client( imagearea->get_width() );
    set_height_client( imagearea->get_height() );
    m_event.add( *m_imagearea );
}



//
// ImageAreaBaseのクリア
// 
void ImageViewBase::remove_imagearea()
{
    if( m_imagearea ){
        m_event.remove();
        m_imagearea.clear();
    }
}



//
// コマンド
//
const bool ImageViewBase::set_command( const std::string& command, const std::string& arg1, const std::string& arg2 )
{
    if( command == "switch_icon" ) switch_icon();
    else if( command == "update_status" ) update_status();

    return true;
}



//
// 再読込
//
void ImageViewBase::reload()
{
#ifdef _DEBUG
    std::cout << "ImageViewBase::reload url = " << get_url() << std::endl;
#endif

    std::string refurl = m_img->get_refurl();
    const bool mosaic = m_img->get_mosaic();
    m_img->download_img( refurl, mosaic, 0 );

    if( m_img->is_loading() ){

        CORE::core_set_command( "redraw", get_url() );
        CORE::core_set_command( "redraw_article" );
    }
}




//
// ロード停止
//
void ImageViewBase::stop()
{
#ifdef _DEBUG
    std::cout << "ImageViewBase::stop url = " << get_url() << std::endl;
#endif
    m_img->stop_load();
}




//
// 再描画
//
void ImageViewBase::redraw_view()
{
#ifdef _DEBUG
    std::cout << "ImageViewBase::redraw_view url = " << get_url() << std::endl;
#endif    

    show_view();
}



//
// 先頭に移動
//
void ImageViewBase::slot_move_head()
{
    IMAGE::get_admin()->set_command( "tab_head", "" );
}



//
// 最後に移動
//
void ImageViewBase::slot_move_tail()
{
    IMAGE::get_admin()->set_command( "tab_tail", "" );
}


//
// 閉じる
//
void ImageViewBase::close_view()
{
    IMAGE::get_admin()->set_command( "close_view", get_url() );
}



//
// 他の画像を閉じる
//
void ImageViewBase::slot_close_other_views()
{
    IMAGE::get_admin()->set_command( "close_other_views", get_url() );
}


//
// 左の画像を閉じる
//
void ImageViewBase::slot_close_left_views()
{
    IMAGE::get_admin()->set_command( "close_left_views", get_url() );
}


//
// 右の画像を閉じる
//
void ImageViewBase::slot_close_right_views()
{
    IMAGE::get_admin()->set_command( "close_right_views", get_url() );
}


//
// エラーの画像を閉じる( HTTP404 のみ )
//
void ImageViewBase::slot_close_error_views()
{
    IMAGE::get_admin()->set_command( "close_error_views", "", "" );
}


//
// エラーの画像を閉じる( timeout, 503 以外 )
//
void ImageViewBase::slot_close_notimeout_error_views()
{
    IMAGE::get_admin()->set_command( "close_error_views", "", "notimeout" );
}


//
// エラーの画像を閉じる( 全て )
//
void ImageViewBase::slot_close_all_error_views()
{
    IMAGE::get_admin()->set_command( "close_error_views", "", "all" );
}


//
// エラー以外の画像を閉じる
//
void ImageViewBase::slot_close_noerror_views()
{
    IMAGE::get_admin()->set_command( "close_noerror_views" );
}


//
// 全ての画像を閉じる
//
void ImageViewBase::slot_close_all_views()
{
    IMAGE::get_admin()->set_command( "close_all_views" );
}


//
// プロパティ
//
void ImageViewBase::show_preference()
{
    CORE::core_set_command( "hide_popup" );

    SKELETON::PrefDiag* pref= CORE::PrefDiagFactory( get_parent_win(), CORE::PREFDIAG_IMAGE, get_url() );

    IMAGE::get_admin()->set_command_immediately( "disable_fold_win" ); // run 直前に呼ぶこと
    pref->run();
    IMAGE::get_admin()->set_command_immediately( "enable_fold_win" );  // run 直後に呼ぶこと

    delete pref;
}


//
// 削除
//
void ImageViewBase::delete_view()
{
    delete_view_impl( false );
}


void ImageViewBase::delete_view_impl( const bool show_diag )
{
    CORE::core_set_command( "hide_popup" );

    if( show_diag ){

        if( m_img->is_protected() ){

            SKELETON::MsgDiag mdiag( get_parent_win(), "キャッシュ保護されています" );
            mdiag.run();
            return;
        }
        else if( CONFIG::get_show_delimgdiag() ){

            SKELETON::MsgCheckDiag mdiag( get_parent_win(),
                                          "画像を削除しますか？",
                                          "今後表示しない(常に削除) (_D)",
                                          Gtk::MESSAGE_QUESTION,
                                          Gtk::BUTTONS_YES_NO,
                                          Gtk::RESPONSE_NO
                );

            if( mdiag.run() != Gtk::RESPONSE_YES ) return;
            if( mdiag.get_chkbutton().get_active() ) CONFIG::set_show_delimgdiag( false );
        }
    }

    CORE::core_set_command( "delete_image", get_url() );
}


//
// クリックした時の処理
//
void ImageViewBase::clicked()
{
    IMAGE::get_admin()->set_command( "switch_image", get_url() );
    IMAGE::get_admin()->set_command( "switch_admin" );
}



//
// viewの操作
//
const bool ImageViewBase::operate_view( const int control )
{
    if( CONTROL::operate_common( control, get_url(), IMAGE::get_admin() ) ) return true;

#ifdef _DEBUG
    std::cout << "ImageViewBase::operate_view control = " << control << std::endl;
#endif

    switch( control ){

        case CONTROL::CancelMosaic:
        case CONTROL::CancelMosaicButton:
            slot_cancel_mosaic();
            break;

        case CONTROL::ZoomInImage:
            slot_zoom_in();
            break;

        case CONTROL::ZoomOutImage:
            slot_zoom_out();
            break;

        case CONTROL::ZoomFitImage:
            slot_fit_win();
            break;

        case CONTROL::OrgSizeImage:
            slot_resize_image( 100 );
            break;

        case CONTROL::ReloadTabButton:
        case CONTROL::Reload:
            reload();
            break;

        case CONTROL::StopLoading:
            stop();
            break;

        case CONTROL::CloseImageButton:
        case CONTROL::CloseImageTabButton:
        case CONTROL::Quit:
            close_view();
            break;

        // スクロール
        case CONTROL::ScrollUpImage:
            IMAGE::get_admin()->set_command( "scroll_up" );
            break;

        case CONTROL::ScrollDownImage:
            IMAGE::get_admin()->set_command( "scroll_down" );
            break;

        case CONTROL::ScrollLeftImage:
            IMAGE::get_admin()->set_command( "scroll_left" );
            break;

        case CONTROL::ScrollRightImage:
            IMAGE::get_admin()->set_command( "scroll_right" );
            break;

            // article に切り替え
        case CONTROL::Left:
            CORE::core_set_command( "switch_leftview" );
            break;

        case CONTROL::ToggleArticle:
            CORE::core_set_command( "toggle_article" );
            break;

        case CONTROL::Save:
        case CONTROL::SaveImageButton:
            slot_save();
            break;

        case CONTROL::Delete:
            delete_view_impl( true );
            break;

            // お気に入りに追加
        case CONTROL::AppendFavorite:
            slot_favorite();
            break;

            // 画像のプロパティ
        case CONTROL::PreferenceView:
            show_preference();
            break;

        default:
            return false;
    }

    return true;
}
                                                    




//
// キープレスイベント
//
const bool ImageViewBase::slot_key_press( GdkEventKey* event )
{
#ifdef _DEBUG
    std::cout << "ImageViewBase::slot_key_press url = " << get_url() << std::endl;
#endif

    return operate_view( get_control().key_press( event ) );
}



//
// ボタンクリック
//
bool ImageViewBase::slot_button_press( GdkEventButton* event )
{
#ifdef _DEBUG
    std::cout << "ImageViewBase::slot_button_press url = " << get_url() << std::endl;
#endif

    // マウスジェスチャ
    get_control().MG_start( event );

    // ホイールマウスジェスチャ
    get_control().MG_wheel_start( event );

    // ダブルクリック
    // button_release_eventでは event->type に必ず GDK_BUTTON_RELEASE が入る
    m_dblclick = false;
    if( event->type == GDK_2BUTTON_PRESS ) m_dblclick = true; 

    // クリック
    // 反応を良くするため slot_button_release() ではなくてボタンを押した時点で処理する
    if( get_control().button_alloted( event, CONTROL::ClickButton ) ) clicked();

    return true;
}



//
// マウスボタンのリリースイベント
//
bool ImageViewBase::slot_button_release( GdkEventButton* event )
{
    /// マウスジェスチャ
    int mg = get_control().MG_end( event );

    // ホイールマウスジェスチャ
    // 実行された場合は何もしない 
    if( get_control().MG_wheel_end( event ) ) return true;

    if( mg != CONTROL::None && enable_mg() ){
        operate_view( mg );
        return true;
    }

    // ダブルクリックの処理のため一時的にtypeを切替える
    GdkEventType type_copy = event->type;
    if( m_dblclick ) event->type = GDK_2BUTTON_PRESS;

    // ポップアップメニュー
    if( get_control().button_alloted( event, CONTROL::PopupmenuButton ) ){

        show_popupmenu( "", false );
    }

    else if( is_under_mouse() ) operate_view( get_control().button_press( event ) );

    event->type = type_copy;

    return true;
}



//
// マウスモーション
//
bool ImageViewBase::slot_motion_notify( GdkEventMotion* event )
{
    /// マウスジェスチャ
    get_control().MG_motion( event );

    return true;
}



//
// マウスホイールイベント
//
bool ImageViewBase::slot_scroll_event( GdkEventScroll* event )
{
    // ホイールマウスジェスチャ
    int control = get_control().MG_wheel_scroll( event );
    if( enable_mg() && control != CONTROL::None ){
        operate_view( control );
        return true;
    }

    return false;
}


//
// マウスが入った
//
bool ImageViewBase::slot_enter_notify_event( GdkEventCrossing* event )
{
#ifdef _DEBUG
    std::cout << "ImageViewBase::slot_enter_notify_event\n";
#endif
    m_under_mouse = true;

    return true;
}

//
// マウスが出た
//
bool ImageViewBase::slot_leave_notify_event( GdkEventCrossing* event )
{
#ifdef _DEBUG
    std::cout << "ImageViewBase::slot_leave_notify_event\n";
#endif
    m_under_mouse = false;

    return true;
}


//
// 強制再読み込み
//
void ImageViewBase::slot_reload_force()
{
    if( ! m_enable_menuslot ) return;

    if( ! SESSION::is_online() ){

        CORE::core_set_command( "hide_popup" );

        SKELETON::MsgDiag mdiag( get_parent_win(), "オフラインです" );
        mdiag.run();
        return;
    }

    // コードをリセットしてから再読み込みをかける
    m_img->reset();
    m_img->set_code( HTTP_INIT );
    m_img->set_type( DBIMG::T_UNKNOWN );
    reload();
}


//
// モザイク解除
//
void ImageViewBase::slot_cancel_mosaic()
{
    if( ! m_enable_menuslot ) return;

    if( ! m_img->is_cached() ) return;
    if( ! m_img->get_mosaic() ) return;

    if( m_img->is_fake() ){

        CORE::core_set_command( "hide_popup" );

        std::string type = "本当の画像タイプは";
        switch( DBIMG::get_type_real( get_url() ) ){
            case DBIMG::T_JPG: type += "JPEG"; break;
            case DBIMG::T_PNG: type += "PNG"; break;
            case DBIMG::T_GIF: type += "GIF"; break;
            case DBIMG::T_BMP: type += "BMP"; break;
        }
        type += "です。";

        SKELETON::MsgDiag mdiag( get_parent_win(),
                                 "拡張子が偽装されています。" + type + "\n\nモザイクを解除しますか？",
                                 false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );

        mdiag.set_default_response( Gtk::RESPONSE_NO );
        if( mdiag.run() != Gtk::RESPONSE_YES ) return;
    }

    m_img->set_mosaic( false );
    CORE::core_set_command( "redraw", get_url() );
}


//
// 大きい画像を表示
//
void ImageViewBase::slot_show_large_img()
{
    m_img->show_large_img();
}


//
// ウィンドウにサイズを合わせる
//
void ImageViewBase::slot_fit_win()
{
    if( m_img->is_zoom_to_fit() ) SESSION::toggle_img_fit_mode();
    m_img->set_zoom_to_fit( true );
    CORE::core_set_command( "redraw", get_url() );
}



//
// ズームイン
//
void ImageViewBase::slot_zoom_in()
{
    zoom_in_out( true );
}


//
// ズームアウト
//
void ImageViewBase::slot_zoom_out()
{
    zoom_in_out( false );
}


//
// ズームイン、アウトの実行部
void ImageViewBase::zoom_in_out( bool zoomin )
{
    unsigned int size[] = SIZE_MENU;
    unsigned int size_current = m_img->get_size();
    int size_zoomin = 0, size_zoomout = 0;

    // 現在のサイズから次のサイズを決定
    if( size_current ){

        size_zoomin = size_current + 100;
        size_zoomout = size_current - 100;

        unsigned int maxsize = sizeof( size )/sizeof( unsigned int );
        for( unsigned int i = 1; i < maxsize ; ++i ){

            if( zoomin && size[ i ] > size_current + 5 ){
                size_zoomin = size[ i ];
                break;
            }

            if( !zoomin && size[ i ] > size_current -5 ){
                size_zoomout = size[ i -1 ];
                break;
            }
        }
    }

#ifdef _DEBUG
    std::cout << "ImageViewBase::zoom_in_out\n"
              << "size_current = " << size_current << std::endl
              << "zoomin = "  << size_zoomin << std::endl
              << "zoomout = "  << size_zoomout << std::endl;
#endif

    if( zoomin ) slot_resize_image( size_zoomin );
    else slot_resize_image( size_zoomout );
}



//
// 画像サイズ
//
void ImageViewBase::slot_resize_image( int size )
{
    unsigned int sizemenu[] = SIZE_MENU;
    int maxsize = sizemenu[ sizeof( sizemenu )/sizeof( unsigned int ) -1 ];

    if( size <= 0 ) return;
    if( size > maxsize ) return;
    if( !m_img->is_zoom_to_fit() && m_img->get_size() == size ) return;

    m_img->set_zoom_to_fit( false );
    m_img->set_size( size );
    CORE::core_set_command( "redraw", get_url() );
}


//
// ロックする
//
void ImageViewBase::slot_lock()
{
    if( ! m_enable_menuslot ) return;

    if( is_locked() ) unlock();
    else lock();
}


//
// ブラウザで開く
//
void ImageViewBase::slot_open_browser()
{
    if( ! m_enable_menuslot ) return;

    CORE::core_set_command( "open_url_browser", get_url() );
}


//
// キャッシュをブラウザで開く
//
void ImageViewBase::slot_open_cache_browser()
{
    if( ! m_enable_menuslot ) return;
    if( ! m_img->is_cached() ) return;

    const std::string url = "file://" + m_img->get_cache_path();
    CORE::core_set_command( "open_url_browser", url );
}


//
// 参照元を開く
//
void ImageViewBase::slot_open_ref()
{
    if( ! m_enable_menuslot ) return;

    const std::string refurl = m_img->get_refurl();

    int center, from, to;
    std::string num_str;
    const std::string url = DBTREE::url_dat( refurl, center, to, num_str );
    if( url.empty() ) return;

    const int range = 10;
    from = MAX( 0, center - range );
    to = center + range;
    std::stringstream ss;
    ss << from << "-" << to;

    CORE::core_set_command( "open_article_res" ,url, ss.str(), MISC::itostr( center ) );
}



//
// URLをクリップボードにコピー
//
void ImageViewBase::slot_copy_url()
{
    if( ! m_enable_menuslot ) return;

    MISC::CopyClipboard( get_url() );
}


//
// 保存
//
void ImageViewBase::slot_save()
{
    if( ! m_enable_menuslot ) return;

    CORE::core_set_command( "hide_popup" );
    m_img->save( IMAGE::get_admin()->get_win(), std::string() );
}



//
// すべて保存
//
void ImageViewBase::slot_save_all()
{
    if( ! m_enable_menuslot ) return;

    IMAGE::get_admin()->set_command( "save_all" );
}



//
// お気に入り
//
void ImageViewBase::slot_favorite()
{
    if( ! m_enable_menuslot ) return;

    set_image_to_buffer();
    CORE::core_set_command( "append_favorite", URL_FAVORITEVIEW );
}


//
// 画像キャッシュ保護
//
void ImageViewBase::slot_toggle_protectimage()
{
    if( ! m_enable_menuslot ) return;

    m_img->set_protect( ! m_img->is_protected( ) );
    CORE::core_set_command( "redraw", get_url() ); // ステータス再描画
}


//
// 画像あぼーん
//
void ImageViewBase::slot_abone_img()
{
    m_img->set_abone( true );
    delete_view();
}


//
// 共有バッファセット
//
void ImageViewBase::set_image_to_buffer()
{
    CORE::DATA_INFO info;
    info.type = TYPE_IMAGE;
    info.parent = IMAGE::get_admin()->get_win();
    info.url = get_url();
    info.name = get_url();
    info.path = Gtk::TreePath( "0" ).to_string();

    CORE::DATA_INFO_LIST list_info;
    list_info.push_back( info );
    CORE::SBUF_set_list( list_info );
}



//
// ポップアップメニューを表示する前にメニューのアクティブ状態を切り替える
//
// SKELETON::View::show_popupmenu() を参照すること
//
void ImageViewBase::activate_act_before_popupmenu( const std::string& url )
{
    if( !m_img ) return;

    // toggle　アクションを activeにするとスロット関数が呼ばれるので処理しないようにする
    m_enable_menuslot = false;

    Glib::RefPtr< Gtk::Action > act;
    Glib::RefPtr< Gtk::ToggleAction > tact;

    bool current_protect = m_img->is_protected();

    // ロック
    act = action_group()->get_action( "LockTab" );
    if( act ){

        Glib::RefPtr< Gtk::ToggleAction > tact = Glib::RefPtr< Gtk::ToggleAction >::cast_dynamic( act ); 
        if( is_locked() ) tact->set_active( true );
        else tact->set_active( false );
    }

    // 閉じる
    act = action_group()->get_action( "Quit" );
    if( act ){
        if( is_locked() ) act->set_sensitive( false );
        else act->set_sensitive( true );
    }

    // モザイク
    act = action_group()->get_action( "CancelMosaic" );
    if( act ){
        if( m_img->is_cached() && m_img->get_mosaic() ) act->set_sensitive( true );
        else act->set_sensitive( false );
    }

    // サイズの大きい画像を表示
    act = action_group()->get_action( "ShowLargeImg" );
    if( act ){
        if( m_img->get_type() == DBIMG::T_LARGE ) act->set_sensitive( true );
        else act->set_sensitive( false );
    }

    // サイズ系メニュー、お気に入り、保存
    std::string sizemenus[] =
    {
        "Size_Menu",
        "OrgSizeImage",
        "ZoomFitImage",
        "ZoomInImage",
        "ZoomOutImage",
        "AppendFavorite",
        "Save"
    };
    for( int i = 0; i < 7; ++i ){
	    act = action_group()->get_action( sizemenus[ i ] );
        if( act ){
            if( m_img->is_cached() ) act->set_sensitive( true );
            else act->set_sensitive( false );
        }
    }

    // キャッシュをブラウザで開く
    act = action_group()->get_action( "OpenCacheBrowser" );
    if( act ){
        if( m_img->is_cached() ) act->set_sensitive( true );
        else act->set_sensitive( false );
    }

    // 参照元スレ
    act = action_group()->get_action( "OpenRef" );
    if( act ){
        if( ! m_img->get_refurl().empty() ) act->set_sensitive( true );
        else act->set_sensitive( false );
    }
    
    // 保護
    act = action_group()->get_action( "ProtectImage" );
    if( act ){

        if( m_img->is_cached() ){

            act->set_sensitive( true );

            tact = Glib::RefPtr< Gtk::ToggleAction >::cast_dynamic( act ); 
            if( tact ){
                if( current_protect ) tact->set_active( true );
                else tact->set_active( false );
            }
        }
        else act->set_sensitive( false );
    }

    // 削除
    act = action_group()->get_action( "DeleteMenu" );
    if( act ){
        if(  m_img->get_code() != HTTP_INIT && ! m_img->is_protected() ) act->set_sensitive( true );
        else act->set_sensitive( false );
    }

    // ロード停止
    act = action_group()->get_action( "LoadStop" );
    if( act ){
        if( m_img->is_loading() ) act->set_sensitive( true );
        else act->set_sensitive( false );
    }

    // あぼーん
    act = action_group()->get_action( "AboneImage" );
    if( act ){
        if( ! m_img->is_protected() ) act->set_sensitive( true );
        else act->set_sensitive( false );
    }

    // ユーザコマンド
    // 選択不可かどうか判断して visible か sensitive にする
    const std::string url_article = DBTREE::url_dat( m_img->get_refurl() );
    CORE::get_usrcmd_manager()->toggle_sensitive( action_group(), url_article, get_url(), "" );

    m_enable_menuslot = true;
}


//
// ユーザコマンド実行
//
void ImageViewBase::slot_usrcmd( int num )
{
    int from, to;
    std::string num_str;
    const std::string url_article = DBTREE::url_dat( m_img->get_refurl(), from, to, num_str );

    CORE::core_set_command( "exec_usr_cmd" ,
                            url_article,
                            MISC::itostr( num ),
                            get_url(),
                            "",
                            MISC::itostr( from )
        );
}
