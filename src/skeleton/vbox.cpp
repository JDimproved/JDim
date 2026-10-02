// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "vbox.h"

#include "jdlib/gtk_compat.h"

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
    else if( expand && fill ) {
        JDLIB::compat::box_append_expand( *this, child, padding );
    }
    else if( expand ) {
        JDLIB::compat::box_append_expand_nofill( *this, child, padding );
    }
    else {
        assert( ! fill ); // 呼び出し元に expand=false, fill=true のパターンはない
        JDLIB::compat::box_append_shrink( *this, child, padding );
    }
}
