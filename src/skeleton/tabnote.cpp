// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "tabnote.h"

#include <gtk/gtk.h>

using namespace SKELETON;


TabNotebook::TabNotebook()
  : Gtk::Notebook()
{
    set_border_width( 0 );
    set_show_border( false );
    property_can_focus() = false;

    // 下側の境界線を消す
    Glib::RefPtr< Gtk::RcStyle > rcst = get_modifier_style();
    int ythick = rcst->get_ythickness();
    if( ythick <= 0 ){
        Glib::RefPtr< Gtk::Style > st = get_style();
        ythick = st->get_ythickness();
    }

#ifdef _DEBUG
    std::cout << "TabNotebook::TabNotebook ythick = " << ythick << std::endl;
#endif

    if( ythick > 1 ){
        --ythick;
        rcst->set_ythickness( 1 );
        modify_style( rcst );
        property_tab_vborder() = property_tab_vborder() + ythick;
    }
}



int TabNotebook::append_tab( Widget& tab )
{
#ifdef _DEBUG
    std::cout << "TabNotebook::append_tab\n";
#endif

    // ダミーWidgetを作成してtabにappend (表示はされない )
    // remve_tab()でdeleteする
    Gtk::EventBox* dummypage = new Gtk::EventBox();

    return append_page( *dummypage , tab );
}


int TabNotebook::insert_tab( Widget& tab, int page )
{
#ifdef _DEBUG
    std::cout << "TabNotebook::insert_tab position = "  << page << std::endl;
#endif

    // ダミーWidgetを作成してtabにappend (表示はされない )
    // remve_page()でdeleteする
    Gtk::EventBox* dummypage = new Gtk::EventBox();

    return insert_page( *dummypage, tab, page );
}


void TabNotebook::remove_tab( int page )
{
#ifdef _DEBUG
    std::cout << "TabNotebook::remove_tab page = " << page << std::endl;
#endif

    // ダミーWidgetをdelete
    Gtk::Widget* dummypage = get_nth_page( page );
    remove_page( page );

    if( dummypage ) delete dummypage;
}


// signal_button_press_event と signal_button_release_event は emit されない
// ときがあるので自前でemitする
bool TabNotebook::on_button_press_event( GdkEventButton* event )
{
    if( m_sig_button_press.emit( event ) ) return true;

    return Gtk::Notebook::on_button_press_event( event );
}


bool TabNotebook::on_button_release_event( GdkEventButton* event )
{
    if( m_sig_button_release.emit( event ) ) return true;

    return Gtk::Notebook::on_button_release_event( event );
}
