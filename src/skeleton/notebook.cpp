// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "notebook.h"

using namespace SKELETON;

JDNotebook::JDNotebook()
    : Gtk::Notebook()
{}


JDNotebook::~JDNotebook()
{}


// unpack = true の時取り除く
int JDNotebook::append_remove_page( bool unpack, Widget& child, const Glib::ustring& tab_label, bool use_mnemonic )
{
    if( unpack ){
        remove( child );
        return 0;
    }

    return append_page( child, tab_label, use_mnemonic );
}
