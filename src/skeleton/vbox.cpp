// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "vbox.h"

using namespace SKELETON;

JDVBox::JDVBox()
    : Gtk::VBox()
{}


JDVBox::~JDVBox() noexcept = default;


// unpack = true の時取り除く
void JDVBox::pack_remove_start( bool unpack, Widget& child, Gtk::PackOptions options, guint padding )
{
    if( unpack ) remove( child );
    else pack_start( child, options, padding );
}

// unpack = true の時取り除く
void JDVBox::pack_remove_end( bool unpack, Widget& child, Gtk::PackOptions options, guint padding )
{
    if( unpack ) remove( child );
    else pack_end( child, options, padding );
}
