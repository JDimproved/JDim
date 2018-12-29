// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "columns.h"

#include "config/globalconf.h"

#include "xml/tools.h"

#include "colorid.h"
#include "type.h"

using namespace BBSLIST;


TreeColumns::TreeColumns()
    : SKELETON::EditColumns()
{}

TreeColumns::~TreeColumns() noexcept
{}

void TreeColumns::setup_row( Gtk::TreeModel::Row& row,
                             const Glib::ustring url, const Glib::ustring name, const Glib::ustring data,
                             const int type, const size_t dirid )
{
    SKELETON::EditColumns::setup_row( row, url, name, data, type, dirid );

    if( type == TYPE_COMMENT ) row[ m_fgcolor ] = Gdk::Color( CONFIG::get_color( COLOR_CHAR_BBS_COMMENT ) );
}
