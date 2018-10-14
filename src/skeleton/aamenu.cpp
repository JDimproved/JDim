// AA 選択ポップアップメニュークラス

//#define _DEBUG
#include "jddebug.h"

#include "aamenu.h"

#include "jdlib/miscutil.h"

#include "config/globalconf.h"

#include "fontid.h"
#include "colorid.h"
#include "aamanager.h"
#include "cache.h"

using namespace SKELETON;

AAMenu::AAMenu( Gtk::Window& parent )
    : Gtk::Menu() , m_parent( parent ), m_popup( SKELETON::POPUPWIN_NOFRAME )
{
#ifdef _DEBUG
    std::cout << "AAMenu::AAMenu\n";
#endif

    Pango::FontDescription pfd( CONFIG::get_fontname( FONT_MESSAGE ) );
    pfd.set_weight( Pango::WEIGHT_NORMAL );
    m_textview.modify_font( pfd );
    m_textview.modify_text( Gtk::STATE_NORMAL, Gdk::Color( CONFIG::get_color( COLOR_CHAR_SELECTION ) ) );
    m_textview.modify_base( Gtk::STATE_NORMAL, Gdk::Color( CONFIG::get_color( COLOR_BACK_SELECTION ) ) );

    m_popup.sig_configured().connect( sigc::mem_fun( *this, &AAMenu::slot_configured_popup ) );
    m_popup.add( m_textview );
    m_popup.show_all_children();

    create_popupmenu();
}


AAMenu::~AAMenu()
{
#ifdef _DEBUG
    std::cout << "AAMenu::~AAMenu\n";
#endif
}


const int AAMenu::get_size()
{
    return items().size();
}


void AAMenu::set_text( const std::string& text )
{
    m_popup.show();
    m_popup.resize( 1, 1 );
    m_textview.get_buffer()->set_text( text );
}


// メニュー項目作成
void AAMenu::create_menuitem( Glib::RefPtr< Gtk::ActionGroup > actiongroup, Gtk::Menu* menu, const int id )
{
    const int maxchar = 20;

    Glib::ustring aa_label = CORE::get_aamanager()->get_label( id );
    std::string shortcut = CORE::get_aamanager()->id2shortcut( id );
    if( ! shortcut.empty() ) aa_label = "[" + shortcut + "] " + aa_label;

    std::string actname = "aa" + MISC::itostr( id );
    if( actiongroup->get_action( actname ) ) return; // 登録済み

#ifdef _DEBUG
    std::cout << actname << " label = " << aa_label << std::endl;
#endif

    Glib::RefPtr< Gtk::Action > action = Gtk::Action::create( actname, aa_label.substr( 0, maxchar ) );
    action->set_accel_group( m_parent.get_accel_group() );

    Gtk::MenuItem* item = Gtk::manage( action->create_menu_item() );

    actiongroup->add( action, sigc::bind< Gtk::MenuItem* >( sigc::mem_fun( *this, &AAMenu::slot_aainput_menu_clicked ), item ) );
    item->signal_select().connect( sigc::bind< Gtk::MenuItem* >( sigc::mem_fun( *this, &AAMenu::slot_select_item ), item ) );

    menu->append( *item );
    m_map_items.insert( std::make_pair( item, id ) );
}



// メニュー作成
void AAMenu::create_popupmenu()
{
    std::string aa_lines;
    if( ! CACHE::load_rawdata( CACHE::path_aalist(), aa_lines ) ) return;

    Glib::RefPtr< Gtk::ActionGroup > actiongroup = Gtk::ActionGroup::create();

    // 履歴
    for( int i = 0 ; i < CORE::get_aamanager()->get_historysize() ; ++i ){
        int org_id = CORE::get_aamanager()->history2id( i );
        create_menuitem( actiongroup, this, org_id );
    }

    if( CORE::get_aamanager()->get_historysize() ){
        Gtk::MenuItem* item = Gtk::manage( new Gtk::SeparatorMenuItem() );
        append( *item );
        m_map_items.insert( std::make_pair( item, -1 ) );
    }

    for( int i = 0 ; i < CORE::get_aamanager()->get_size() ; ++i ) create_menuitem( actiongroup, this, i );

    show_all_children();
}


void AAMenu::on_map()
{
#ifdef _DEBUG
    std::cout << "AAMenu::on_realize\n";
#endif

    Gtk::Menu::on_map();
    select_item( items()[ 0 ] );
}


void AAMenu::on_hide()
{
#ifdef _DEBUG
    std::cout << "AAMenu::on_hide\n";
#endif

    m_popup.hide();

    Gtk::Menu::on_hide();
}


// 下移動
bool AAMenu::move_down()
{
#ifdef _DEBUG
    std::cout << "AAMenu::move_down\n";
#endif

    Gtk::Menu_Helpers::MenuList::iterator it = items().begin();
    for( ; it != items().end() && &(*it) != m_activeitem; ++it );

    ++it;
    if( m_map_items[ &(*it) ] == -1 ) ++it; // セパレータ
    if( it == items().end() ) it = items().begin(); // 一番下まで行ったら上に戻る
    select_item( *it );

    return true;
}


// 上移動
bool AAMenu::move_up()
{
#ifdef _DEBUG
    std::cout << "AAMenu::move_up\n";
#endif

    Gtk::Menu_Helpers::MenuList::iterator it = items().begin();
    for( ; it != items().end() && &(*it) != m_activeitem; ++it );

    if( it != items().begin() ){ 
        --it;
        if( m_map_items[ &(*it) ] == -1 && it != items().begin() ) --it;  // セパレータ
        select_item( *it );
    }
    else select_item( items().back() ); // 一番上に行ったら下に戻る

    return true;
}


// キー入力のフック
bool AAMenu::on_key_press_event( GdkEventKey* event )
{
#ifdef _DEBUG
    std::cout << "AAMenu::on_key_press_event key = " << event->keyval << std::endl;
#endif

    // 下移動
    if( event->keyval == GDK_j
             || ( ( event->state & GDK_CONTROL_MASK ) && event->keyval == GDK_n )
             || event->keyval == GDK_space
        ){

        move_down();
        return true;
    }

    // 上移動
    else if( event->keyval == GDK_k
                  || ( ( event->state & GDK_CONTROL_MASK ) && event->keyval == GDK_p )
        ){
        move_up();
        return true;
    }

    // ショートカット
    else{ 

        int id = CORE::get_aamanager()->shortcut2id( event->string[ 0 ] );

#ifdef _DEBUG
        std::cout << "id = " << id << " key = " << event->string[ 0 ] << std::endl;
#endif

        if( id >= 0 ){

            std::string aa = CORE::get_aamanager()->get_aa( id );
            if( ! aa.empty() ){
                m_sig_selected.emit( aa );
                CORE::get_aamanager()->append_history( id );
                hide();
            }
        }
    }

    return Gtk::Menu::on_key_press_event( event );
}


//
// ポップアップウィンドウのサイズが変わった
//
void AAMenu::slot_configured_popup( int width, int height )
{
    int sw = get_screen()->get_width();
    int x, y;
#if GTKMM_CHECK_VERSION(2,18,0)
    get_window()->get_root_coords( 0, 0, x, y );
#else
    get_window()->get_root_origin( x, y );
#endif

#ifdef _DEBUG
    std::cout << " AAMenu::slot_configured_popup w = " << width
              << " h = " << height
              << " x = " << x
              << " screen width = " << sw << std::endl;
#endif

    y -= height;
    if( x + width > sw  ) x = sw -  width;

    m_popup.move( x, y );
}


//
// メニューの行を選択
//
void AAMenu::slot_select_item( Gtk::MenuItem* item )
{
    if( ! get_active() ) return;

    int id = m_map_items[ item ];
    m_activeitem = item;

#ifdef _DEBUG
    std::cout << "AAMenu::slot_select_item id = " << id << std::endl;
#endif

    set_text( CORE::get_aamanager()->get_aa( id ) );
}


//
// アスキーアート入力
//
void AAMenu::slot_aainput_menu_clicked( Gtk::MenuItem* item )
{
    if( ! get_active() ) return;

    int id = m_map_items[ item ];
    m_activeitem = item;

#ifdef _DEBUG
    std::cout << "AAMenu::slot_aainput_menu_clicked id = " << id << std::endl;
    std::cout << CORE::get_aamanager()->get_aa( id ) << std::endl;
#endif

    m_sig_selected.emit( CORE::get_aamanager()->get_aa( id ) );
    CORE::get_aamanager()->append_history( id );
}

