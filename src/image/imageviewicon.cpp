// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "imageadmin.h"
#include "imageviewicon.h"
#include "imageareaicon.h"

#include "dbimg/img.h"

#include "jdlib/miscutil.h"

#include "type.h"
#include "dndmanager.h"
#include "sharedbuffer.h"
#include "cache.h"
#include "command.h"
#include "controlid.h"


// 枠を描く
#define DRAW_FRAME( color ) m_event_frame->modify_bg( Gtk::STATE_NORMAL, color ); 

using namespace IMAGE;


ImageViewIcon::ImageViewIcon( const std::string& url )
    : ImageViewBase( url )
{
#ifdef _DEBUG    
    std::cout << "ImageViewIcon::ImageViewIcon : " << get_url() << std::endl;
#endif

    // コントロールモード設定
    get_control().add_mode( CONTROL::MODE_IMAGEICON );

    //枠を描くためにm_eventの外にもう一つEventBoxを作る ( Gtk::HBox は modify_fg() 無効なので )
    m_event_frame = Gtk::manage( new Gtk::EventBox() );
    pack_start( *m_event_frame );
    m_event_frame->add( get_event() );
    get_event().set_border_width( 1 );
    DRAW_FRAME( Gdk::Color( "white" ) );

    setup_common();

    // D&D可能にする
    std::list< Gtk::TargetEntry > targets;
    targets.push_back( Gtk::TargetEntry( "text/plain", Gtk::TARGET_SAME_APP, 0 ) );
    get_event().drag_source_set( targets, Gdk::BUTTON1_MASK );
    get_event().drag_dest_set( targets );

    get_event().signal_drag_begin().connect( sigc::mem_fun( *this, &ImageViewIcon::slot_drag_begin ) );
    get_event().signal_drag_motion().connect( sigc::mem_fun( *this, &ImageViewIcon::slot_drag_motion ) );
    get_event().signal_drag_drop().connect( sigc::mem_fun( *this, &ImageViewIcon::slot_drag_drop ) );
    get_event().signal_drag_end().connect( sigc::mem_fun( *this, &ImageViewIcon::slot_drag_end ) );
}


ImageViewIcon::~ImageViewIcon()
{
#ifdef _DEBUG    
    std::cout << "ImageViewIcon::~ImageViewIcon : " << get_url() << std::endl;
#endif

    // スレッドを止めるために明示的にクリアする
    get_imagearea().clear();
}



//
// クロック入力
//
void ImageViewIcon::clock_in()
{
    View::clock_in();

    // ロード終了
    if( get_imagearea() && is_loading() && ! get_img()->is_loading() ){

        set_loading( false );

        // 画像表示
        show_view();
    }
}



//
// フォーカスイン
//
void ImageViewIcon::focus_view()
{
    DRAW_FRAME( Gdk::Color( "red" ) );
    get_event().grab_focus();
}


//
// フォーカスアウト
//
void ImageViewIcon::focus_out()
{
    SKELETON::View::focus_out();
    DRAW_FRAME( Gdk::Color( "white" ) );
}



//
// 表示
//
void ImageViewIcon::show_view()
{
    if( is_loading() ) return;

#ifdef _DEBUG
    std::cout << "ImageViewIcon::show_view url = " << get_url() << std::endl;
#endif    

    // 読み込み中        
    if( get_img()->is_loading() ) set_loading( true );

    // 画像が既に表示しているなら再描画
    if( get_imagearea() ) get_imagearea()->show_image();

    // 画像貼り付け
    // ロード中の時はclock_in()経由でもう一度 show_view()が呼び出される
    else{

        set_imagearea( Gtk::manage( new ImageAreaIcon( get_url() ) ) );
        get_imagearea()->show_image();
        show_all_children();
    }
}



//
// アイコンが切り替わった
//
void ImageViewIcon::switch_icon()
{
    DRAW_FRAME( Gdk::Color( "red" ) );
}




//
// ポップアップメニュー取得
//
// SKELETON::View::show_popupmenu() を参照すること
//
Gtk::Menu* ImageViewIcon::get_popupmenu( const std::string& url )
{
    Gtk::Menu* menu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_icon" ) );

    // タブ情報セット
    if( menu ){

        // 一番上のitemのラベルを書き換える
        Gtk::Menu_Helpers::MenuList::iterator it_item =  menu->items().begin();

        Gtk::Label* label = dynamic_cast< Gtk::Label* >( (*it_item).get_child() );
        if( label ) label->set_text_with_mnemonic( "移動 [ タブ数 " + MISC::itostr( IMAGE::get_admin()->get_tab_nums() ) + " ](_M)" );
    }

    return menu;
}



//
// マウスホイールイベント
//
bool ImageViewIcon::slot_scroll_event( GdkEventScroll* event )
{
    // 回転したらタブ切り替え
    int control = CONTROL::None;
    if( event->direction == GDK_SCROLL_UP ) control = CONTROL::TabLeft;
    if( event->direction == GDK_SCROLL_DOWN ) control = CONTROL::TabRight;

    ImageViewBase::operate_view( control );
    IMAGE::get_admin()->set_command( "switch_admin" );

    return true;
}



//
// D&D開始
//
void ImageViewIcon::slot_drag_begin( const Glib::RefPtr<Gdk::DragContext>& context )
{
#ifdef _DEBUG    
    std::cout << "ImageViewIcon::slot_drag_begin url = " << get_url() << std::endl;
#endif

    CORE::DND_Begin( get_url() );
    set_image_to_buffer();
}



//
// D&D中
//
bool ImageViewIcon::slot_drag_motion( const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time ) 
{
#ifdef _DEBUG    
    std::cout << "ImageViewIcon::slot_drag_motion url = " << get_url() << std::endl;
#endif

    return true;
}



//
// ドロップされた
bool ImageViewIcon::slot_drag_drop( const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time )
{
#ifdef _DEBUG    
    std::cout << "ImageViewIcon::slot_on_drag_drop url = " << get_url() << std::endl;
#endif

    // 他のアイコンからドロップされた場合は入れ換え
    std::list< CORE::DATA_INFO > infolist = CORE::SBUF_infolist();
    std::list< CORE::DATA_INFO >::iterator it = infolist.begin();
    CORE::DATA_INFO& info = ( *it );
    if( info.type == TYPE_IMAGE ){

#ifdef _DEBUG    
        std::cout << "url_from = " << info.url << std::endl;
#endif
        IMAGE::get_admin()->set_command( "reorder", get_url(), info.url, get_url() );
    }

    return true;
}



//
// D&D終了
//
void ImageViewIcon::slot_drag_end( const Glib::RefPtr< Gdk::DragContext >& context )
{
#ifdef _DEBUG    
    std::cout << "ImageViewIcon::slot_drag_end url = " << get_url() << std::endl;
#endif

    CORE::DND_End();
}



