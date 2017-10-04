// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "editcolumns.h"

#include "xml/tools.h"

#include "type.h"

using namespace SKELETON;


EditColumns::EditColumns()
{

    add( m_name );
    add( m_image );

    add( m_type );
    add( m_url );
    add( m_data );
    add( m_underline );
    add( m_expand );
    add( m_fgcolor );
    add( m_dirid );
}


EditColumns::~EditColumns() noexcept = default;


void EditColumns::setup_row( Gtk::TreeModel::Row& row,
                             const Glib::ustring url, const Glib::ustring name, const Glib::ustring data, const int type, const size_t dirid )
{
    row[ m_name ] = name;
    row[ m_image ] = XML::get_icon( type );

    row[ m_type ] = type;
    row[ m_url ] = url;
    row[ m_data ] = data;
    row[ m_expand ] = false;
    row[ m_underline ] = false;
    row[ m_dirid ] = dirid;
}


void EditColumns::copy_row( const Gtk::TreeModel::Row& row_src, Gtk::TreeModel::Row& row_dest )
{
    const Glib::ustring url = row_src[ m_url ];
    const Glib::ustring name = row_src[ m_name ];
    const Glib::ustring data = row_src[ m_data ];
    const int type = row_src[ m_type ];
    const size_t dirid = row_src[ m_dirid ];

    setup_row( row_dest, url, name, data, type, dirid );
}
