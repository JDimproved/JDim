// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "columns.h"

#include "xml/tools.h"

using namespace BBSLIST;


TreeColumns::TreeColumns()
{
    add( m_col_name );
    add( m_underline );
    add( m_col_url );
    add( m_col_image );
    add( m_type );
    add( m_expand );
}

TreeColumns::~TreeColumns()
{}


void TreeColumns::setup_row( Gtk::TreeModel::Row& row, const Glib::ustring url, const Glib::ustring name, const int type )
{
    row[ m_col_url ] = url;
    row[ m_col_name ] = name;
    row[ m_type ] = type;
    row[ m_underline ] = false;
    row[ m_col_image ] = XML::get_icon( type );
}
