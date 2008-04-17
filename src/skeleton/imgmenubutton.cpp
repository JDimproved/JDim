// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "imgmenubutton.h"

#include "jdlib/miscutil.h"

#include "command.h"

using namespace SKELETON;

enum
{
    MENU_MAX_LNG = 50 // メニューに表示する文字数(半角)
};


ImgMenuButton::ImgMenuButton( const Gtk::StockID& stock_id,
                              const Gtk::BuiltinIconSize icon_size )
{
    m_img = Gtk::manage( new Gtk::Image( stock_id, icon_size ) );
    setup();
}


ImgMenuButton::ImgMenuButton()
    : m_img( NULL )
{
    setup();
}


void ImgMenuButton::setup()
{
    m_popupmenu =  NULL;
    m_on_arrow = false;

    Gtk::HBox *hbox = Gtk::manage( new Gtk::HBox() );
    m_arrow = Gtk::manage( new Gtk::Arrow( Gtk::ARROW_DOWN, Gtk::SHADOW_NONE ) );

    hbox->set_spacing( 2 );
    if( m_img ) hbox->pack_start( *m_img, Gtk::PACK_SHRINK );
    hbox->pack_start( *m_arrow, Gtk::PACK_SHRINK );

    add_events( Gdk::ENTER_NOTIFY_MASK );
    add_events( Gdk::POINTER_MOTION_MASK );
    add_events( Gdk::LEAVE_NOTIFY_MASK );
    signal_enter_notify_event().connect( sigc::mem_fun( *this, &ImgMenuButton::slot_enter ) );
    signal_leave_notify_event().connect( sigc::mem_fun( *this, &ImgMenuButton::slot_leave ) );
    signal_motion_notify_event().connect(  sigc::mem_fun( *this, &ImgMenuButton::slot_motion ) );

    add( *hbox );
    show_all_children();

    // メニュー項目作成
    Glib::RefPtr< Gtk::ActionGroup > actiongroup = Gtk::ActionGroup::create();
    Glib::RefPtr< Gtk::AccelGroup > agroup  = CORE::get_mainwindow()->get_accel_group();
    for( int i = 0 ; i < MAX_MENU_SIZE; ++i ){
        Glib::RefPtr< Gtk::Action > action = Gtk::Action::create( "menu" + MISC::itostr( i ), "dummy" );
        action->set_accel_group( agroup );
        Gtk::MenuItem* item = Gtk::manage( action->create_menu_item() );
        actiongroup->add( action, sigc::bind< int >( sigc::mem_fun( *this, &ImgMenuButton::slot_menu_selected ), i ) );
        m_menuitems.push_back( item );
    }

    set_focus_on_click( false );
}


// virtual
ImgMenuButton::~ImgMenuButton()
{
    if( m_popupmenu ) delete m_popupmenu;
}


//
// メニュー項目追加
//
void ImgMenuButton::append_menu( std::vector< std::string >& items )
{
    // 古いメニューからメニュー項目を取り除いてdelete
    if( m_popupmenu ){
        const int menusize = m_popupmenu->get_children().size();
        for( int i = 0; i < menusize; ++i ) m_popupmenu->remove( *m_menuitems[ i ] );
        delete m_popupmenu;
    }
    m_popupmenu = NULL;

    if( ! items.size() ) return;

    // 新しくメニューを作成して項目追加
    m_popupmenu = Gtk::manage( new Gtk::Menu() );
    const int size = MIN( items.size(), MAX_MENU_SIZE );
    for( int i = 0 ; i < size; ++i ){
        Gtk::MenuItem* item = m_menuitems[ i ];
        dynamic_cast< Gtk::Label* >( item->get_child() )->set_text( MISC::cut_str( items[ i ], MENU_MAX_LNG ) );
        m_popupmenu->append( *item );
    }
}


//
// ポップアップメニュー表示
//
// virtual
void ImgMenuButton::show_popupmenu()
{
    if( ! m_popupmenu ) return;

    m_popupmenu->popup( Gtk::Menu::SlotPositionCalc( sigc::mem_fun( *this, &ImgMenuButton::slot_popup_pos ) ),
                        0, gtk_get_current_event_time() );
}


//
// メニューが選ばれた
//
void ImgMenuButton::slot_menu_selected( int i )
{
#ifdef _DEBUG
    std::cout << "ImgMenuButton::slot_menu_selected i = " << i << std::endl;
#endif

    m_sig_selected.emit( i );
}


//
// ボタンがクリックされたらクリックした位置で処理を変える
//
void ImgMenuButton::on_clicked()
{
#ifdef _DEBUG
    std::cout << "ImgMenuButton::on_clicked\n";
#endif

    if( m_on_arrow ) show_popupmenu();
    else m_sig_clicked.emit();
}


//
// ポップアップメニューの位置決め
//
void ImgMenuButton::slot_popup_pos( int& x, int& y, bool& push_in )
{
    int ox, oy;
    get_window()->get_origin( ox, oy );
    Gdk::Rectangle rect = get_allocation();
    x = ox + rect.get_x();
    y = oy + rect.get_y() + rect.get_height();
    push_in = false;
}


bool ImgMenuButton::slot_enter( GdkEventCrossing* event )
{
#ifdef _DEBUG
    std::cout << "ImgMenuButton::slot_enter\n";
#endif

    check_on_arrow( (int)event->x );

    return true;
}


bool ImgMenuButton::slot_leave( GdkEventCrossing* event )
{
#ifdef _DEBUG
    std::cout << "ImgMenuButton::slot_leave\n";
#endif

    m_on_arrow = false;

    return true;
}


bool ImgMenuButton::slot_motion( GdkEventMotion* event )
{
    check_on_arrow( (int)event->x );

    return true;
}


//
// ポインタが矢印の下にあるかチェック
//
void ImgMenuButton::check_on_arrow( int ex )
{
    if( ! m_img ){
        m_on_arrow = true;
        return;
    }

    Gdk::Rectangle rect = m_arrow->get_allocation();
    int x = rect.get_x();
    rect = get_allocation();
    x -= rect.get_x();

    if( ex >= x ) m_on_arrow = true;
    else m_on_arrow = false;

#ifdef _DEBUG
    std::cout << "ImgMenuButton::check_on_arrow x = " << ex << " / " << x 
              << " on = " << m_on_arrow << std::endl;
#endif
}
