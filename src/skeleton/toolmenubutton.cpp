// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "toolmenubutton.h"

using namespace SKELETON;


ToolMenuButton::ToolMenuButton( MenuButton& button, const std::string& label, const bool expand )
{
    assert( button.get_label_widget() != NULL );
    Gtk::MenuItem* item = NULL;

    // アイコンの場合はアイコン表示
    Gtk::Image* image = dynamic_cast< Gtk::Image* >( button.get_label_widget() );
    if( image ){
        Gtk::StockID id;
        Gtk::IconSize size;
        image->get_stock( id, size );
        item = Gtk::manage( new Gtk::ImageMenuItem( *Gtk::manage( new Gtk::Image( id, size ) ), "" ) );
    }
    else item = Gtk::manage( new Gtk::MenuItem( label ) );

    if( item ){
        item->signal_activate().connect( sigc::mem_fun( button, &MenuButton::on_clicked ) );
        set_proxy_menu_item( label, *item );
    }

    set_expand( expand );
    add( button );
}
