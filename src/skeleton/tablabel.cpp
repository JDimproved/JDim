// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "tablabel.h"

#include "icons/iconmanager.h"

#include "config/globalconf.h"

using namespace SKELETON;

TabLabel::TabLabel( const std::string& url )
    : m_url( url ), m_id_icon( ICON::NUM_ICONS ), m_image( NULL ), m_image_width( 0 ), m_under_mouse( false )
{
#ifdef _DEBUG
    std::cout << "TabLabel::TabLabel " << m_url << std::endl;
#endif

    add_events( Gdk::ENTER_NOTIFY_MASK );
    add_events( Gdk::POINTER_MOTION_MASK );
    add_events( Gdk::LEAVE_NOTIFY_MASK );

    add( m_hbox );
    m_hbox.pack_start( m_label, Gtk::PACK_SHRINK );

    show_all_children();
}


TabLabel::~TabLabel()
{
#ifdef _DEBUG
    std::cout << "TabLabel::~TabLabel " << m_fulltext << std::endl;
#endif

    if( m_image ) delete m_image;
}


// タブとラベルの間のすき間のサイズ
// 取得方法が分からないのでとりあえずマジックナンバーを返しておく
const int TabLabel::get_margin()
{
    const int mrg_tab = 12;
    return mrg_tab;
}


// アイコンセット
void TabLabel::set_id_icon( int id )
{
    if( ! CONFIG::get_show_tab_icon() ) return;
    if( m_id_icon == id ) return;

    if( !m_image ){
        m_image = new Gtk::Image();
        m_hbox.remove( m_label );
        m_hbox.pack_start( *m_image, Gtk::PACK_SHRINK );
        m_hbox.pack_start( m_label, Gtk::PACK_SHRINK );
        show_all_children();
    }

#ifdef _DEBUG
    std::cout << "TabLabel::set_icon " <<  m_fulltext << " id = " << id << std::endl;
#endif

    m_id_icon = id;

    m_image->set( ICON::get_icon( id ) );
    m_image_width = ICON::get_icon( id )->get_width();
}


// タブの文字列の文字数がlngになるようにリサイズする
void TabLabel::resize_tab( int lng )
{
    Glib::ustring ulabel( m_fulltext );
    ulabel.resize( lng );
    m_label.set_text( ulabel );
}



//
// D&D設定
//
void TabLabel::set_dragable( bool dragable, int button )
{
    if( dragable ){

        std::list< Gtk::TargetEntry > targets;
        targets.push_back( Gtk::TargetEntry( "text/plain", Gtk::TARGET_SAME_APP, 0 ) );

        // ドラッグ側
        switch( button ){

            case 1: drag_source_set( targets, Gdk::BUTTON1_MASK ); break;
            case 2: drag_source_set( targets, Gdk::BUTTON2_MASK ); break;
            case 3: drag_source_set( targets, Gdk::BUTTON3_MASK ); break;

            default: return;
        }

        // ドロップ側
        drag_dest_set( targets );
    }
    else{
        drag_source_unset();
        drag_dest_unset();
    }
}



//
// マウスが入った
//
bool TabLabel::on_enter_notify_event( GdkEventCrossing* event )
{
#ifdef _DEBUG
    std::cout << "TabLabel::enter " << m_fulltext << std::endl;
#endif

    m_under_mouse = true;

    return Gtk::EventBox::on_enter_notify_event( event );
}


//
// マウスが動いた
//
bool TabLabel::on_motion_notify_event( GdkEventMotion* event )
{
    m_sig_tab_motion_event.emit();

    return Gtk::EventBox::on_motion_notify_event( event );
}


//
// マウスが出た
//
bool TabLabel::on_leave_notify_event( GdkEventCrossing* event )
{
#ifdef _DEBUG
    std::cout << "TabLabel::leave " << m_fulltext << std::endl;
#endif

    m_under_mouse = false;

    m_sig_tab_leave_event.emit();

    return Gtk::EventBox::on_leave_notify_event( event );
}


//
// ドラッグ開始
//
void TabLabel::on_drag_begin( const Glib::RefPtr< Gdk::DragContext >& context )
{
#ifdef _DEBUG
    std::cout << "TabLabel::on_drag_begin " << m_fulltext << std::endl;
#endif

    m_under_mouse = false;

    m_sig_tab_drag_begin.emit();

    return Gtk::EventBox::on_drag_begin( context );
}


//
// ドロップされた
//
bool TabLabel::on_drag_drop( const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time )
{
#ifdef _DEBUG
    std::cout << "TabLabel::on_drag_drop " << m_fulltext << std::endl;;
#endif

    m_under_mouse = true;

    m_sig_tab_drag_drop.emit();

    return Gtk::EventBox::on_drag_drop( context, x, y, time );
}


//
// ドラッグ終了
//
void TabLabel::on_drag_end( const Glib::RefPtr< Gdk::DragContext >& context )
{
#ifdef _DEBUG
    std::cout << "TabLabel::on_drag_end " << m_fulltext << std::endl;;
#endif

    m_sig_tab_drag_end.emit();

    Gtk::EventBox::on_drag_end( context );
}
