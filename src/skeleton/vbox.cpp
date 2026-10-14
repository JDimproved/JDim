// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "vbox.h"

using namespace SKELETON;


JDVBox::JDVBox()
    : Gtk::Box{ Gtk::ORIENTATION_VERTICAL, 0 }
{
}


JDVBox::~JDVBox() noexcept = default;


// unpack = true の時取り除く
void JDVBox::pack_remove_start( bool unpack, Widget& child, bool expand, bool fill, guint padding )
{
    if( unpack ) remove( child );
    else pack_start( child, expand, fill, padding );
}

// unpack = true の時取り除く
void JDVBox::pack_remove_end( bool unpack, Widget& child, bool expand, bool fill, guint padding )
{
    if( unpack ) remove( child );
    else pack_end( child, expand, fill, padding );
}
