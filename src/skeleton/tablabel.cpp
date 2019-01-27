// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "tablabel.h"

#include "icons/iconmanager.h"

#include "config/globalconf.h"

#include "dndmanager.h"

using namespace SKELETON;

enum
{
    SPACING_LABEL = 3 // アイコンとラベルの間のスペース
};


TabLabel::TabLabel( const std::string& url )
    : m_url( url ), m_id_icon( ICON::NUM_ICONS ), m_image( NULL )
{
#ifdef _DEBUG
    std::cout << "TabLabel::TabLabel " << m_url << std::endl;
#endif

    // 背景透過
    set_visible_window( false );

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


// アイコンセット
void TabLabel::set_id_icon( const int id )
{
    if( ! CONFIG::get_show_tab_icon() ) return;
    if( m_id_icon == id ) return;
    if( id == ICON::NONE ) return;

    if( !m_image ){
        m_image = new Gtk::Image();
        m_hbox.remove( m_label );
        m_hbox.set_spacing( SPACING_LABEL );
        m_hbox.pack_start( *m_image, Gtk::PACK_SHRINK );
        m_hbox.pack_start( m_label, Gtk::PACK_SHRINK );
        show_all_children();
    }

#ifdef _DEBUG
    std::cout << "TabLabel::set_icon " <<  m_fulltext << " id = " << id << std::endl;
#endif

    m_id_icon = id;

    m_image->set( ICON::get_icon( id ) );
}


// タブの文字列の文字数がlngになるようにリサイズする
void TabLabel::resize_tab( const unsigned int lng )
{
    Glib::ustring ulabel( m_fulltext );
    const unsigned int lng_org = ulabel.length();
    ulabel.resize( lng );
    if( lng < lng_org ) ulabel += "...";
    m_label.set_text( ulabel );
}


//
// D&D設定
//
void TabLabel::set_dragable( bool dragable, int button )
{
    if( dragable ){

        std::vector< Gtk::TargetEntry > targets;
        targets.push_back( Gtk::TargetEntry( DNDTARGET_FAVORITE, Gtk::TARGET_SAME_APP, 0 ) );
        targets.push_back( Gtk::TargetEntry( DNDTARGET_TAB, Gtk::TARGET_SAME_APP, 0 ) );

        // ドラッグ開始側にする
        switch( button ){

            case 1: drag_source_set( targets, Gdk::BUTTON1_MASK ); break;
            case 2: drag_source_set( targets, Gdk::BUTTON2_MASK ); break;
            case 3: drag_source_set( targets, Gdk::BUTTON3_MASK ); break;

            default: return;
        }
    }
    else{
        drag_source_unset();
        drag_dest_unset();
    }
}


//
// 本体の横幅 - ラベルの横幅
//
int TabLabel::get_label_margin()
{
    int label_margin;

    int x_pad, y_pad;
    m_label.get_padding( x_pad, y_pad );
    label_margin = x_pad*2
        + m_hbox.get_spacing() + m_hbox.get_border_width()*2
        + get_border_width()*2;

    if( CONFIG::get_show_tab_icon() && m_id_icon < ICON::NUM_ICONS ) label_margin += ICON::get_icon( m_id_icon )->get_width();

#ifdef _DEBUG
    std::cout << "image_w = " << m_image->get_allocation().get_width()
              << " label_w = "  << m_label.get_allocation().get_width()
              << " alloc_w =  " << get_allocation().get_width()
              << " label_margine = " << label_margin
              << std::endl;
#endif

    return label_margin;
}


//
// マウスが動いた
//
bool TabLabel::on_motion_notify_event( GdkEventMotion* event )
{
    const bool ret = Gtk::EventBox::on_motion_notify_event( event );

    m_sig_tab_motion_event.emit();

    return ret;
}


//
// マウスが出た
//
bool TabLabel::on_leave_notify_event( GdkEventCrossing* event )
{
#ifdef _DEBUG
    std::cout << "TabLabel::leave\n";
#endif

    const bool ret = Gtk::EventBox::on_leave_notify_event( event );

    m_sig_tab_leave_event.emit();

    return ret;
}


//
// ドラッグ開始
//
void TabLabel::on_drag_begin( const Glib::RefPtr< Gdk::DragContext >& context )
{
#ifdef _DEBUG
    std::cout << "TabLabel::on_drag_begin " << m_fulltext << std::endl;
#endif

    m_sig_tab_drag_begin.emit();

    Gtk::EventBox::on_drag_begin( context );
}


//
// D&Dで受信側がデータ送信を要求してきた
//
void TabLabel::on_drag_data_get( const Glib::RefPtr<Gdk::DragContext>& context,
                                 Gtk::SelectionData& selection_data, guint info, guint time )
{
#ifdef _DEBUG
    std::cout << "TabLabel::on_drag_data_get target = " << selection_data.get_target()
              << " " << m_fulltext << std::endl;;
#endif

    Gtk::EventBox::on_drag_data_get( context, selection_data, info, time );

    m_sig_tab_drag_data_get( selection_data );
}


//
// ドラッグ終了
//
void TabLabel::on_drag_end( const Glib::RefPtr< Gdk::DragContext >& context )
{
#ifdef _DEBUG
    std::cout << "TabLabel::on_drag_end " << m_fulltext << std::endl;;
#endif

    Gtk::EventBox::on_drag_end( context );

    m_sig_tab_drag_end.emit();
}
