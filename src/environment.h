// License GPL2

#ifndef _ENVIRONMENT_H
#define _ENVIRONMENT_H

#include <string>

namespace ENVIRONMENT
{
    // WM
    enum class DesktopType
    {
        gnome = 0,
        xfce,
        kde,
        lxde,
        unity,
        cinnamon,
        mate,
        budgie,
        pantheon,
        enlightenment,
        lxqt,
        unknown
    };

    // configure_argsのモード
    enum
	{
        CONFIGURE_OMITTED = 0,
        CONFIGURE_OMITTED_MULTILINE,
		CONFIGURE_FULL
    };

    std::string get_progname();
    std::string get_jdcomments();
    std::string get_jdcopyright();
	std::string get_jdbbs();
	std::string get_jd2chlog();
	std::string get_jdhelp();
	std::string get_jdhelpcmd();
	std::string get_jdhelpreplstr();
    std::string get_jdlicense();
    std::string get_configure_args( const int mode = CONFIGURE_OMITTED );

    std::string get_jdversion();
    std::string get_distname();
    DesktopType get_wm();
    std::string get_wm_str();
	std::string get_gtkmm_version();
	std::string get_glibmm_version();
    std::string get_tlslib_version();
    std::string get_jdinfo();

    // ダイアログでClient-Side Decorationを使うか
    // JDimの設定とGtkSettingsから使うか判断する
    bool get_dialog_use_header_bar();
}


#endif
