// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "menubutton.h"

#include "jdlib/miscutil.h"

#include "icons/iconmanager.h"

#include "command.h"

using namespace SKELETON;

enum
{
    MENU_MAX_LNG = 50 // メニューに表示する文字数(半角)
};


MenuButton::MenuButton( const bool show_arrow, Gtk::Widget* label, Gtk::PackOptions options )
    : m_label{ label }
    , m_enable_sig_clicked{ true }
{
    Gtk::Box* hbox = Gtk::manage( new Gtk::Box( Gtk::ORIENTATION_HORIZONTAL ) );

    hbox->set_spacing( 4 );
    if( m_label ) hbox->pack_start( *m_label, options );

    if( show_arrow ){
        m_arrow = Gtk::manage( new Gtk::Image() );
        m_arrow->set_from_icon_name( "pan-down-symbolic", Gtk::ICON_SIZE_SMALL_TOOLBAR );
        hbox->pack_start( *m_arrow, Gtk::PACK_SHRINK );
    }
    else m_enable_sig_clicked = false;

    add_events( Gdk::ENTER_NOTIFY_MASK );
    add_events( Gdk::POINTER_MOTION_MASK );
    add_events( Gdk::LEAVE_NOTIFY_MASK );
    add_events( Gdk::SCROLL_MASK );
    signal_enter_notify_event().connect( sigc::mem_fun( *this, &MenuButton::slot_enter ) );
    signal_leave_notify_event().connect( sigc::mem_fun( *this, &MenuButton::slot_leave ) );
    signal_motion_notify_event().connect(  sigc::mem_fun( *this, &MenuButton::slot_motion ) );

    add( *hbox );
    show_all_children();

    // メニュー項目作成
    Glib::RefPtr< Gtk::ActionGroup > actiongroup = Gtk::ActionGroup::create();
    Glib::RefPtr< Gtk::AccelGroup > agroup  = CORE::get_mainwindow()->get_accel_group();
    for( size_t i = 0 ; i < MAX_MENU_SIZE; ++i ){
        Glib::RefPtr< Gtk::Action > action = Gtk::Action::create( "menu" + std::to_string( i ), "dummy" );
        action->set_accel_group( agroup );
        Gtk::MenuItem* item = Gtk::manage( action->create_menu_item() );
        actiongroup->add( action, sigc::bind( sigc::mem_fun( *this, &MenuButton::slot_menu_selected ), i ) );
        m_menuitems.push_back( item );
    }

    set_focus_on_click( false );
}


MenuButton::MenuButton( const bool show_arrow, Gtk::Widget& label )
    : MenuButton( show_arrow, &label )
{
}


MenuButton::MenuButton( const bool show_arrow, const int id )
    : MenuButton( show_arrow, Gtk::manage( new Gtk::Image( ICON::get_icon( id ), Gtk::ICON_SIZE_SMALL_TOOLBAR ) ), Gtk::PACK_SHRINK )
{
}


void MenuButton::set_tooltip_arrow( const std::string& tooltip )
{
    if( m_arrow ) {
        m_arrow->set_tooltip_text( tooltip );
    }
}


//
// メニュー項目追加
//
void MenuButton::append_menu( std::vector< std::string >& items )
{
    // 古いメニューからメニュー項目を取り除いてdelete
    if( m_popupmenu ){
        const int menusize = m_popupmenu->get_children().size();
        for( int i = 0; i < menusize; ++i ) m_popupmenu->remove( *m_menuitems[ i ] );
        m_popupmenu.reset();
    }

    if( ! items.size() ) return;

    // 新しくメニューを作成して項目追加
    m_popupmenu = std::make_unique<Gtk::Menu>();
    const size_t size = MIN( items.size(), MAX_MENU_SIZE );
    for( size_t i = 0 ; i < size; ++i ){

        Gtk::MenuItem* item = nullptr;

        if( items[ i ] == "separator" ){
            item = Gtk::manage( new Gtk::SeparatorMenuItem() );
        }
        else{
            item = m_menuitems[ i ];
            if( auto label = dynamic_cast<Gtk::Label*>( item->get_child() ) ) {
                label->set_text( MISC::cut_str( items[i], MENU_MAX_LNG ) );
            }
        }

        if( item ) m_popupmenu->append( *item );
    }
    m_popupmenu->show_all();
}


//
// ポップアップメニュー表示
//
// virtual
void MenuButton::show_popupmenu()
{
    if( ! m_popupmenu ) return;

    // Specify the current event by nullptr.
    m_popupmenu->popup_at_widget( this, Gdk::GRAVITY_SOUTH_WEST, Gdk::GRAVITY_NORTH_WEST, nullptr );
}


//
// メニューが選ばれた
//
void MenuButton::slot_menu_selected( int i )
{
#ifdef _DEBUG
    std::cout << "MenuButton::slot_menu_selected i = " << i << std::endl;
#endif

    m_sig_selected.emit( i );
}


//
// ボタンがクリックされたらクリックした位置で処理を変える
//
void MenuButton::on_clicked()
{
#ifdef _DEBUG
    std::cout << "MenuButton::on_clicked\n";
#endif

    if( ! m_enable_sig_clicked || m_on_arrow ) show_popupmenu();
    else m_sig_clicked.emit();
}


bool MenuButton::slot_enter( GdkEventCrossing* event )
{
#ifdef _DEBUG
    std::cout << "MenuButton::slot_enter\n";
#endif

    check_on_arrow( static_cast<int>(event->x) );

    return true;
}


bool MenuButton::slot_leave( GdkEventCrossing* event )
{
#ifdef _DEBUG
    std::cout << "MenuButton::slot_leave\n";
#endif

    m_on_arrow = false;

    return true;
}


bool MenuButton::slot_motion( GdkEventMotion* event )
{
    check_on_arrow( static_cast<int>(event->x) );

    return true;
}


//
// ポインタが矢印の下にあるかチェック
//
void MenuButton::check_on_arrow( int ex )
{
    if( ! m_arrow ){
        m_on_arrow = false;
        return;
    }

    if( ! m_label ){
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
    std::cout << "MenuButton::check_on_arrow x = " << ex << " / " << x
              << " on = " << m_on_arrow << std::endl;
#endif
}
