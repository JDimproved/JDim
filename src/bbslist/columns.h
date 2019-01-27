// ライセンス: GPL2

//
// コラム
//

#ifndef _BBSLISTCOLUMNS_H
#define _BBSLISTCOLUMNS_H

#include "skeleton/editcolumns.h"

namespace BBSLIST
{
    class TreeColumns : public SKELETON::EditColumns
    {

    public:
        
        TreeColumns();
        ~TreeColumns() noexcept;

        void setup_row( Gtk::TreeModel::Row& row,
                        const Glib::ustring url, const Glib::ustring name, const Glib::ustring data,
                        const int type, const size_t dirid ) override;
    };
}

#endif
