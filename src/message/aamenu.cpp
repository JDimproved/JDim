// AA 管理クラス

#define _DEBUG
#include "jddebug.h"

#include "aamenu.h"

#include "jdlib/miscutil.h"

#include "aamanager.h"
#include "cache.h"
#include "command.h"

using namespace MESSAGE;


AAMenu::AAMenu( Gtk::Window& parent, const std::string& url )
    : Gtk::Menu() , m_parent( parent ), m_popup( Gtk::WINDOW_POPUP ), m_url( url )
{
#ifdef _DEBUG
    std::cout << "AAMenu::AAMenu\n";
#endif

    m_popup.add( m_textview );
    m_popup.show_all_children();

    create_popupmenu();
}


int AAMenu::get_size()
{
    return items().size();
}


void AAMenu::set_text( const std::string& text )
{
    m_popup.show();
    m_popup.resize( 1, 1 );
    m_textview.get_buffer()->set_text( text );
}



// メニュー作成
void AAMenu::create_popupmenu()
{
    const int maxchar = 20;

    std::string aa_lines;
    if( ! CACHE::load_rawdata( CACHE::path_aalist(), aa_lines ) ) return;

    Glib::RefPtr< Gtk::Action > action;
    Glib::RefPtr< Gtk::ActionGroup > actiongroup = Gtk::ActionGroup::create();
    int menu_id = 0;

    std::list< std::string >& list_aa_labels = CORE::get_aamanager()->get_labels();

    std::list< std::string >::iterator it = list_aa_labels.begin();
    for( ; it != list_aa_labels.end() ; ++it, ++menu_id )
    {
        std::string aa_label = *it;

#ifdef _DEBUG
        std::cout << "label = " << aa_label << std::endl;
#endif
        action = Gtk::Action::create( "aa" + MISC::itostr( menu_id ), aa_label.substr( 0, maxchar ) );
        action->set_accel_group( m_parent.get_accel_group() );
        actiongroup->add( action, sigc::bind< int >( sigc::mem_fun( *this, &AAMenu::slot_aainput_menu_clicked ), menu_id ) );
        append( *Gtk::manage( action->create_menu_item() ) );
    }
}


void AAMenu::on_map()
{
#ifdef _DEBUG
    std::cout << "AAMenu::on_realize\n";
#endif

    Gtk::Menu::on_map();

    m_activeitem = 0;
    select_item( items()[ m_activeitem ] );

    // ポップアップウィンドウ表示
    int x, y;
    get_window()->get_root_origin( x, y );
    x += get_width();
    m_popup.move( x, y );

    set_text( CORE::get_aamanager()->get_aa( m_activeitem ) );
}


void AAMenu::on_hide()
{
#ifdef _DEBUG
    std::cout << "AAMenu::on_hide\n";
#endif

    m_popup.hide();

    Gtk::Menu::on_hide();
}


// キー入力のフック
bool AAMenu::on_key_press_event( GdkEventKey* event )
{
    // 下移動
    if( m_activeitem < get_size()-1
        && ( event->keyval == GDK_j
             || ( ( event->state & GDK_CONTROL_MASK ) && event->keyval == GDK_n )
             || event->keyval == GDK_space
            ) ){

#ifdef _DEBUG
        std::cout << "AAMenu::on_key_press_event : down\n";
#endif
        ++m_activeitem;
        select_item( items()[ m_activeitem ] );
        set_text( CORE::get_aamanager()->get_aa( m_activeitem ) );
    }

    // 上移動
    else if( m_activeitem > 0
             && ( event->keyval == GDK_k
                  || ( ( event->state & GDK_CONTROL_MASK ) && event->keyval == GDK_p )
                 ) ){

#ifdef _DEBUG
        std::cout << "AAMenu::on_key_press_event : up\n";
#endif
        --m_activeitem;
        select_item( items()[ m_activeitem ] );
        set_text( CORE::get_aamanager()->get_aa( m_activeitem ) );
    }

    // スペースをキャンセル
    if( event->keyval == GDK_space ){
#ifdef _DEBUG
        std::cout << "AAMenu::on_key_press_event : space\n";
#endif
        return true;
    }

    return Gtk::Menu::on_key_press_event( event );
}


//
// アスキーアート入力
//
void AAMenu::slot_aainput_menu_clicked( int num )
{
#ifdef _DEBUG
    std::cout << "AAMenu::on_key_press_event\n";
    std::cout << CORE::get_aamanager()->get_aa( m_activeitem ) << std::endl;
#endif

    CORE::core_set_command( "open_message", m_url, CORE::get_aamanager()->get_aa( m_activeitem ) );

    CORE::get_aamanager()->move_to_top( num );
}

