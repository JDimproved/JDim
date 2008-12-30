// License GPL2

#ifndef _ENVIRINMENT_H
#define _ENVIRONMENT_H

#include <string>

namespace ENVIRONMENT
{
    // WM
    enum
    {
        WM_GNOME = 0,
        WM_XFCE,
        WM_KDE,
        WM_UNKNOWN
    };

    std::string get_jdcomments();
    std::string get_jdcopyright();
	std::string get_jdbbs();
	std::string get_jd2chlog();
	std::string get_jdhelp();
	std::string get_jdhelpcmd();
    std::string get_jdlicense();
    std::string get_configure_args( const unsigned int unit = 0 );

    std::string get_jdversion();
    std::string get_distname();
    int get_wm();
    std::string get_wm_str();
	std::string get_gtkmm_version();
	std::string get_glibmm_version();
    std::string get_jdinfo();
}


#endif
