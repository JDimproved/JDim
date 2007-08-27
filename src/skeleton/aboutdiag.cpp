// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#if GTKMMVER >= 260
#include "aboutdiag.h"

#include "command.h"
#include "session.h"

using namespace SKELETON;


AboutDiag::AboutDiag( const Glib::ustring product_name,
                  const Glib::ustring version,
                  const Glib::ustring icon_name,
                  const Glib::ustring comments,
                  const Glib::ustring website,
                  const Glib::ustring copyright,
                  const Glib::ustring license)
    : Gtk::AboutDialog()
{
    Gtk::AboutDialog::set_name( product_name );
    Gtk::AboutDialog::set_version( version );
    Gtk::AboutDialog::set_logo_icon_name( icon_name );
    Gtk::AboutDialog::set_comments( comments );
    Gtk::AboutDialog::set_website( website );
    Gtk::AboutDialog::set_copyright( copyright );
    Gtk::AboutDialog::set_license( license );
}


int AboutDiag::run()
{
#ifdef _DEBUG
    std::cout << "AboutDiag::run start\n";
#endif

    SESSION::set_dialog_shown( true );
    CORE::core_set_command( "dialog_shown" );

    int ret = Gtk::AboutDialog::run();

    SESSION::set_dialog_shown( false );
    CORE::core_set_command( "dialog_hidden" );

#ifdef _DEBUG
    std::cout << "AboutDiag::run fin\n";
#endif

    return ret;
}
#endif  // GTKMMVER
