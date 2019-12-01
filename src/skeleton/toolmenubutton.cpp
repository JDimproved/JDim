// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "toolmenubutton.h"
#include "menubutton.h"
#include "backforwardbutton.h"

using namespace SKELETON;

ToolMenuButton::ToolMenuButton() = default;


ToolMenuButton::ToolMenuButton( const std::string& label, const bool expand,
                                const bool show_arrow, Gtk::Widget& widget )
{
    SKELETON::MenuButton *button = Gtk::manage( new SKELETON::MenuButton( show_arrow, widget ) );
    setup( button, label, expand );
}


ToolMenuButton::ToolMenuButton( const std::string& label, const bool expand,
                                const bool show_arrow ,
                                const int id )
{
    SKELETON::MenuButton *button = Gtk::manage( new SKELETON::MenuButton( show_arrow, id ) );
    setup( button, label, expand );
}


ToolMenuButton::~ToolMenuButton() noexcept = default;


void ToolMenuButton::setup( SKELETON::MenuButton* button, const std::string& label, const bool expand )
{
    m_button = button;
    assert( m_button != nullptr );
    assert( m_button->get_label_widget() != nullptr );

    Gtk::MenuItem* item = nullptr;

    // アイコンの場合はアイコン表示
    Gtk::Image* image = dynamic_cast< Gtk::Image* >( m_button->get_label_widget() );
    if( image ){
        const Gtk::ImageType type = image->get_storage_type();
        if( type == Gtk::IMAGE_STOCK ) {
            Gtk::StockID id;
            Gtk::IconSize size;
            image->get_stock( id, size );
            item = Gtk::manage( new Gtk::ImageMenuItem( *Gtk::manage( new Gtk::Image( id, size ) ), label ) );
        }
        else if( type == Gtk::IMAGE_PIXBUF ) {
            auto pixbuf = image->get_pixbuf();
            item = Gtk::manage( new Gtk::ImageMenuItem( *Gtk::manage( new Gtk::Image( pixbuf ) ), label ) );
        }
    }
    if( !item ) {
        item = Gtk::manage( new Gtk::MenuItem( label ) );
    }

    if( item ){
        item->signal_activate().connect( sigc::mem_fun( *m_button, &MenuButton::on_clicked ) );
        set_proxy_menu_item( label, *item );
    }

    set_expand( expand );
    add( *m_button );
}


///////////////////////////



ToolBackForwardButton::ToolBackForwardButton( const std::string& label, const bool expand,
                                              const std::string& url, const bool back )
{
    SKELETON::MenuButton *button = Gtk::manage( new SKELETON::BackForwardButton( url, back ) );
    setup( button, label, expand );
}


SKELETON::BackForwardButton* ToolBackForwardButton::get_backforward_button()
{
    return dynamic_cast< SKELETON::BackForwardButton* >( get_button() );
}
