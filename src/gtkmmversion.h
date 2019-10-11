// ライセンス: GPL2

#ifndef GTKMM_VERSION_H
#define GTKMM_VERSION_H

// GTK_CHECK_VERSION in gtk/gtkversion.h (GPL2)
#ifndef GTKMM_CHECK_VERSION
#define GTKMM_CHECK_VERSION(major,minor,micro) \
    (GTKMM_MAJOR_VERSION > (major) || \
     (GTKMM_MAJOR_VERSION == (major) && GTKMM_MINOR_VERSION > (minor)) || \
     (GTKMM_MAJOR_VERSION == (major) && GTKMM_MINOR_VERSION == (minor) && \
      GTKMM_MICRO_VERSION >= (micro)))
#endif

#endif // GTKMM_VERSION_H
