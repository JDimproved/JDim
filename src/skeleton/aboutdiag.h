// ライセンス: GPL2

// Aboutダイアログの基底クラス

#ifndef _ABOUTDIAG_H
#define _ABOUTDIAG_H

#include <gtkmm.h>

namespace SKELETON
{
    class AboutDiag : public Gtk::AboutDialog
    {
      public:

        AboutDiag( const Glib::ustring product_name,
                  const Glib::ustring version,
                  const Glib::ustring icon_name,
                  const Glib::ustring comments,
                  const Glib::ustring website,
                  const Glib::ustring copyright,
                  const Glib::ustring license);

        virtual ~AboutDiag(){}

        virtual int run();
    };
}

#endif
