// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "vpaned.h"

using namespace SKELETON;

JDVPaned::JDVPaned()
    : Gtk::VPaned()
{}


JDVPaned::~JDVPaned()
{}


// unpack = true の時取り除く
void JDVPaned::add_remove1( bool unpack, Gtk::Widget& child )
{
    if( unpack ) remove( child );
    else add1( child );
}

// unpack = true の時取り除く
void JDVPaned::add_remove2( bool unpack, Gtk::Widget& child )
{
    if( unpack ) remove( child );
    else add2( child );
}
